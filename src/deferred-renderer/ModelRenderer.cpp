#include "PCH.h"
#include "ModelRenderer.h"
#include "ModelInstanceSet.h"

ModelRenderer::ModelRenderer()
    : _meshVertexShader(NULL), _alphaThresholdBuffer(NULL), _modelPropertiesBuffer(NULL),
    _instanceWorldVB(NULL)
{
    for (UINT i = 0; i < 2; i++)
    {
        for (UINT j = 0; j < 2; j++)
        {
            for (UINT k = 0; k < 2; k++)
            {
                _meshPixelShader[i][j][k][0] = NULL;
                _meshPixelShader[i][j][k][1] = NULL;
            }
        }
    }

    SetAlphaCutoutEnabled(true);
    SetAlphaThreshold(0.05f);
}

HRESULT ModelRenderer::RenderModels(ID3D11DeviceContext* pd3dDeviceContext,
                                    vector<ModelInstance*>* instances, Camera* camera)
{
    HRESULT hr;
    D3D11_MAPPED_SUBRESOURCE mappedResource;

    XMFLOAT4X4 fViewProj = camera->GetViewProjection();
    XMMATRIX viewProj = XMLoadFloat4x4(&fViewProj);

    XMFLOAT4X4 fPrevViewProj = camera->GetPreviousViewProjection();
    XMMATRIX prevViewProj = XMLoadFloat4x4(&fPrevViewProj);

    pd3dDeviceContext->VSSetShader(_meshVertexShader->VertexShader, NULL, 0);

    pd3dDeviceContext->IASetInputLayout(_meshVertexShader->InputLayout);
    pd3dDeviceContext->OMSetDepthStencilState(_dsStates.GetDepthWriteStencilSetDesc(), 1);

    pd3dDeviceContext->RSSetState(_rasterStates.GetBackFaceCull());

    float blendFactor[4] = {1, 1, 1, 1};
    pd3dDeviceContext->OMSetBlendState(_blendStates.GetBlendDisabled(), blendFactor, 0xFFFFFFFF);

    ID3D11SamplerState* sampler = _samplerStates.GetAnisotropic16Wrap();
    pd3dDeviceContext->PSSetSamplers(0, 1, &sampler);

    // if Alpha cutout is enabled, map the alpha cutout info buffer
    if (_alphaCutoutEnabled)
    {
        V_RETURN(pd3dDeviceContext->Map(_alphaThresholdBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
        CB_MODEL_ALPHA_THRESHOLD* cutoutProperties = (CB_MODEL_ALPHA_THRESHOLD*)mappedResource.pData;
        cutoutProperties->AlphaThreshold = _alphaThreshold;
        pd3dDeviceContext->Unmap(_alphaThresholdBuffer, 0);

        pd3dDeviceContext->PSSetConstantBuffers(1, 1, &_alphaThresholdBuffer);
    }

    V(pd3dDeviceContext->Map(_modelPropertiesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
    CB_MODEL_PROPERTIES* modelProperties = (CB_MODEL_PROPERTIES*)mappedResource.pData;
    XMStoreFloat4x4(&modelProperties->ViewProjection, XMMatrixTranspose(viewProj));
    XMStoreFloat4x4(&modelProperties->PreviousViewProjection, XMMatrixTranspose(prevViewProj));
    pd3dDeviceContext->Unmap(_modelPropertiesBuffer, 0);

    pd3dDeviceContext->VSSetConstantBuffers(0, 1, &_modelPropertiesBuffer);

    Frustum cameraFrust = camera->CreateFrustum();
    ModelInstanceSet modelSet = ModelInstanceSet(instances, &cameraFrust);

    // Copy the instance world matrices into the vertex buffer
    V(pd3dDeviceContext->Map(_instanceWorldVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
    XMFLOAT4X4* instanceVB = (XMFLOAT4X4*)mappedResource.pData;

    for (UINT i = 0; i < modelSet.GetModelCount(); i++)
    {
        for (UINT j = 0; j < modelSet.GetInstanceCount(i); j++)
        {
            ModelInstance* instance = modelSet.GetInstance(i, j);

            XMFLOAT4X4 fWorld = instance->GetWorld();
            XMMATRIX world = XMLoadFloat4x4(&fWorld);

            XMFLOAT4X4 fPrevWorld = instance->GetPreviousWorld();
            XMMATRIX prevWorld = XMLoadFloat4x4(&fPrevWorld);

            XMStoreFloat4x4(&instanceVB[modelSet.GetGlobalIndex(i, j)], XMMatrixTranspose(world));
        }
    }

    pd3dDeviceContext->Unmap(_instanceWorldVB, 0);

    UINT instanceVBStride = sizeof(XMFLOAT4X4);
    UINT instanceVBOffset = 0;
    pd3dDeviceContext->IASetVertexBuffers(1, 1, &_instanceWorldVB, &instanceVBStride, &instanceVBOffset);

    ID3D11PixelShader* prevPS = NULL;
    for (UINT i = 0; i < modelSet.GetModelCount(); i++)
    {
        Model* model = modelSet.GetModel(i);

        // Render each mesh
        for (UINT j = 0; j < model->GetMeshCount(); j++)
        {
            const Mesh* mesh = model->GetMesh(j);
            UINT partCount = mesh->GetMeshPartCount();

            ID3D11Buffer* meshVB = mesh->GetVertexBuffer();
            UINT meshStride = mesh->GetVertexStride();
            UINT meshOffset = 0;

            pd3dDeviceContext->IASetVertexBuffers(0, 1, &meshVB, &meshStride, &meshOffset);
            pd3dDeviceContext->IASetIndexBuffer(mesh->GetIndexBuffer(), mesh->GetIndexBufferFormat(), 0);
            pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            for (UINT k = 0; k < partCount; k++)
            {
                const MeshPart* part = mesh->GetMeshPart(k);
                const Material* mat = model->GetMaterial(part->MaterialIndex);

                ID3D11Buffer* buf = mat->GetPropertiesBuffer();
                pd3dDeviceContext->PSSetConstantBuffers(0, 1, &buf);

                ID3D11ShaderResourceView* diffSRV = mat->GetDiffuseSRV();
                ID3D11ShaderResourceView* normSRV = mat->GetNormalSRV();
                ID3D11ShaderResourceView* specSRV = mat->GetSpecularSRV();

                ID3D11ShaderResourceView* srvs[3] = { diffSRV, normSRV, specSRV };
                pd3dDeviceContext->PSSetShaderResources(0, 3, srvs);

                // Set the shader if it wasn't the same for the last mesh
                ID3D11PixelShader* ps = _meshPixelShader[diffSRV != NULL][normSRV != NULL][specSRV != NULL]
                [_alphaCutoutEnabled && mesh->GetAlphaCutoutEnabled()]->PixelShader;
                if (ps != prevPS)
                {
                    pd3dDeviceContext->PSSetShader(ps, NULL, 0);
                    prevPS = ps;
                }

                pd3dDeviceContext->DrawIndexedInstanced(part->IndexCount, modelSet.GetInstanceCount(i),
                    part->IndexStart, part->VertexStart, modelSet.GetGlobalIndex(i, 0));
            }
        }
    }

    // Null the second vertex buffer
    ID3D11Buffer* nullVB[1] = { NULL };
    UINT nullStride[1] = { 0 };
    UINT nullOffset[1] = { 0 };

    pd3dDeviceContext->IASetVertexBuffers(1, 1, nullVB, nullStride, nullOffset);

    // Unset the alpha cutout buffer
    if (_alphaCutoutEnabled)
    {
        ID3D11Buffer* nullBuffer[1] = { NULL };
        pd3dDeviceContext->PSSetConstantBuffers(2, 1, nullBuffer);
    }

    return S_OK;
}

HRESULT ModelRenderer::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
    HRESULT hr;

    D3D_SHADER_MACRO meshMacros[] =
    {
        { "DIFFUSE_MAPPED", "" },
        { "NORMAL_MAPPED", "" },
        { "SPECULAR_MAPPED", "" },
        { "ALPHA_CUTOUT_ENABLED", "" },
        NULL,
    };

    char debugName[256];

    PixelShaderOptions psOpts =
    {
        "PS_Mesh",    // const char* EntryPoint;
        meshMacros,    // D3D_SHADER_MACRO* Defines;
        debugName,    // const char* DebugName;
    };

    for (UINT i = 0; i < 2; i++)
    {
        meshMacros[0].Definition = i ? "1" : "0";

        for (UINT j = 0; j < 2; j++)
        {
            meshMacros[1].Definition = j ? "1" : "0";

            for (UINT k = 0; k < 2; k++)
            {
                meshMacros[2].Definition = k ? "1" : "0";

                // Load no alpha cutout
                meshMacros[3].Definition = "0";
                sprintf_s(debugName, "G-Buffer Mesh (diffuse = %u, normal = %u, specular = %u, alpha cutout = %u)",
                    i, j, k, 0);
                V_RETURN(pContentManager->LoadContent(pd3dDevice, L"Mesh.hlsl", &psOpts, &_meshPixelShader[i][j][k][0]));

                // Load alpha cutout
                meshMacros[3].Definition = "1";
                sprintf_s(debugName, "G-Buffer Mesh (diffuse = %u, normal = %u, specular = %u, alpha cutout = %u)",
                    i, j, k, 1);
                V_RETURN(pContentManager->LoadContent(pd3dDevice, L"Mesh.hlsl", &psOpts, &_meshPixelShader[i][j][k][1]));
            }
        }
    }

    // Load the vertex shader
    D3D11_INPUT_ELEMENT_DESC layout_mesh[] =
    {
        { "POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT,        0, 0,  D3D11_INPUT_PER_VERTEX_DATA,        0 },
        { "NORMAL",        0, DXGI_FORMAT_R32G32B32_FLOAT,        0, 12, D3D11_INPUT_PER_VERTEX_DATA,        0 },
        { "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,        0, 24, D3D11_INPUT_PER_VERTEX_DATA,        0 },
        { "TANGENT",    0, DXGI_FORMAT_R32G32B32_FLOAT,        0, 32, D3D11_INPUT_PER_VERTEX_DATA,        0 },
        { "BINORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT,        0, 44, D3D11_INPUT_PER_VERTEX_DATA,        0 },
        { "WORLD",        0, DXGI_FORMAT_R32G32B32A32_FLOAT,    1, 0,  D3D11_INPUT_PER_INSTANCE_DATA,    1 },
        { "WORLD",        1, DXGI_FORMAT_R32G32B32A32_FLOAT,    1, 16, D3D11_INPUT_PER_INSTANCE_DATA,    1 },
        { "WORLD",        2, DXGI_FORMAT_R32G32B32A32_FLOAT,    1, 32, D3D11_INPUT_PER_INSTANCE_DATA,    1 },
        { "WORLD",        3, DXGI_FORMAT_R32G32B32A32_FLOAT,    1, 48, D3D11_INPUT_PER_INSTANCE_DATA,    1 },
    };

    VertexShaderOptions vsOpts =
    {
        "VS_Mesh",                // const char* EntryPoint;
        NULL,                    // D3D_SHADER_MACRO* Defines;
        layout_mesh,            // D3D11_INPUT_ELEMENT_DESC* InputElements;
        ARRAYSIZE(layout_mesh),    // UINT InputElementCount;
        "G-Buffer Mesh"            // const char* DebugName;
    };

    V_RETURN(pContentManager->LoadContent(pd3dDevice, L"Mesh.hlsl", &vsOpts, &_meshVertexShader));

    D3D11_BUFFER_DESC bufferDesc =
    {
        0, //UINT ByteWidth;
        D3D11_USAGE_DYNAMIC, //D3D11_USAGE Usage;
        D3D11_BIND_CONSTANT_BUFFER, //UINT BindFlags;
        D3D11_CPU_ACCESS_WRITE, //UINT CPUAccessFlags;
        0, //UINT MiscFlags;
        0, //UINT StructureByteStride;
    };

    bufferDesc.ByteWidth = sizeof(CB_MODEL_PROPERTIES);
    V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_modelPropertiesBuffer));

    bufferDesc.ByteWidth = sizeof(CB_MODEL_ALPHA_THRESHOLD);
    V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_alphaThresholdBuffer));

    D3D11_BUFFER_DESC vbDesc =
    {
        sizeof(XMFLOAT4X4) * MAX_INSTANCES, // INT ByteWidth;
        D3D11_USAGE_DYNAMIC, // D3D11_USAGE Usage;
        D3D11_BIND_VERTEX_BUFFER, // UINT BindFlags;
        D3D11_CPU_ACCESS_WRITE, // UINT CPUAccessFlags;
        0, // UINT MiscFlags;
        0, // UINT StructureByteStride;
    };
    V_RETURN(pd3dDevice->CreateBuffer(&vbDesc, NULL, &_instanceWorldVB));

    V_RETURN(_dsStates.OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));
    V_RETURN(_samplerStates.OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));
    V_RETURN(_blendStates.OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));
    V_RETURN(_rasterStates.OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));

    return S_OK;
}

void ModelRenderer::OnD3D11DestroyDevice(ContentManager* pContentManager)
{
    for (UINT i = 0; i < 2; i++)
    {
        for (UINT j = 0; j < 2; j++)
        {
            for (UINT k = 0; k < 2; k++)
            {
                SAFE_CM_RELEASE(pContentManager, _meshPixelShader[i][j][k][0]);
                SAFE_CM_RELEASE(pContentManager, _meshPixelShader[i][j][k][1]);
            }
        }
    }

    SAFE_CM_RELEASE(pContentManager, _meshVertexShader);

    SAFE_RELEASE(_modelPropertiesBuffer);
    SAFE_RELEASE(_alphaThresholdBuffer);
    SAFE_RELEASE(_instanceWorldVB);

    _dsStates.OnD3D11DestroyDevice(pContentManager);
    _samplerStates.OnD3D11DestroyDevice(pContentManager);
    _blendStates.OnD3D11DestroyDevice(pContentManager);
    _rasterStates.OnD3D11DestroyDevice(pContentManager);
}

HRESULT ModelRenderer::OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
                                               const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
    HRESULT hr;

    V_RETURN(_dsStates.OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));
    V_RETURN(_samplerStates.OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));
    V_RETURN(_blendStates.OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));
    V_RETURN(_rasterStates.OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));

    return S_OK;
}

void ModelRenderer::OnD3D11ReleasingSwapChain(ContentManager* pContentManager)
{
    _dsStates.OnD3D11ReleasingSwapChain(pContentManager);
    _samplerStates.OnD3D11ReleasingSwapChain(pContentManager);
    _blendStates.OnD3D11ReleasingSwapChain(pContentManager);
    _rasterStates.OnD3D11ReleasingSwapChain(pContentManager);
}