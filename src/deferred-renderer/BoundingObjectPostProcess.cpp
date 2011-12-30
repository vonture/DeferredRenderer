#include "PCH.h"
#include "BoundingObjectPostProcess.h"
#include "PixelShaderLoader.h"
#include "VertexShaderLoader.h"

BoundingObjectPostProcess::BoundingObjectPostProcess()
	: _boxVB(NULL), _boxIB(NULL), _sphereVB(NULL), _inputLayout(NULL), _vertexShader(NULL),
	  _pixelShader(NULL), _wvpConstantBuffer(NULL), _colorConstantBuffer(NULL)
{
	SetIsAdditive(true);
	SetColor(XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f));
}

BoundingObjectPostProcess::~BoundingObjectPostProcess()
{
}

void BoundingObjectPostProcess::Add(const AxisAlignedBox& aabb)
{
	_objects.AddAxisAlignedBox(aabb);
}

void BoundingObjectPostProcess::Add(const OrientedBox& obb)
{
	_objects.AddOrientedBox(obb);
}

void BoundingObjectPostProcess::Add(const Sphere& sphere)
{
	_objects.AddSphere(sphere);
}

void BoundingObjectPostProcess::Add(const Frustum& frust)
{
	_objects.AddFrustum(frust);
}

void BoundingObjectPostProcess::Add(BoundingObjectSet* objectSet)
{
	_objects.Merge(objectSet);
}

void BoundingObjectPostProcess::Clear()
{
	_objects.Clear();
}

HRESULT BoundingObjectPostProcess::Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
	ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer, LightBuffer* lightBuffer)
{
	BEGIN_EVENT_D3D(L"Bounding objects");

	HRESULT hr;
	
	pd3dImmediateContext->OMSetRenderTargets(1, &dstRTV, gBuffer->GetReadOnlyDepthDSV());

	D3D11_MAPPED_SUBRESOURCE mappedResource;

	XMFLOAT4X4 fViewProj = camera->GetViewProjection();
	XMMATRIX viewProj = XMLoadFloat4x4(&fViewProj);

	UINT strides = sizeof(BOUNDING_OBJECT_VERTEX);
    UINT offsets = 0;

	ID3D11Buffer* boxVBs[1] = { _boxVB };
	ID3D11Buffer* sphereVBs[1] = { _sphereVB };

	pd3dImmediateContext->VSSetShader(_vertexShader, NULL, 0);
    pd3dImmediateContext->PSSetShader(_pixelShader, NULL, 0);
	
	pd3dImmediateContext->IASetInputLayout(_inputLayout);	

	pd3dImmediateContext->OMSetDepthStencilState(GetDepthStencilStates()->GetDepthEnabled(), 0);

	float blendFactor[4] = {1, 1, 1, 1};
	pd3dImmediateContext->OMSetBlendState(GetBlendStates()->GetBlendDisabled(), blendFactor, 0xFFFFFFFF);

	V_RETURN(pd3dImmediateContext->Map(_colorConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	CB_BOUNDING_OBJECT_COLOR* colorProperties = (CB_BOUNDING_OBJECT_COLOR*)mappedResource.pData;
	colorProperties->Color = _boColor;
	pd3dImmediateContext->Unmap(_colorConstantBuffer, 0);

	pd3dImmediateContext->PSSetConstantBuffers(0, 1, &_colorConstantBuffer);

	BEGIN_EVENT_D3D(L"AABBs");
	pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	pd3dImmediateContext->IASetIndexBuffer(_boxIB, DXGI_FORMAT_R16_UINT, 0);
    pd3dImmediateContext->IASetVertexBuffers(0, 1, boxVBs, &strides, &offsets);
	for (UINT i = 0; i < _objects.GetAxisAlignedBoxeCount(); i++)
	{
		AxisAlignedBox* aabb = _objects.GetAxisAlignedBox(i);

		// Calculate the world matrix
		XMMATRIX scale = XMMatrixScalingFromVector(XMLoadFloat3(&aabb->Extents));
		XMMATRIX trans = XMMatrixTranslationFromVector(XMLoadFloat3(&aabb->Center));

		XMMATRIX world = XMMatrixMultiply(scale, trans);
		XMMATRIX wvp = XMMatrixMultiply(world, viewProj);

		V_RETURN(pd3dImmediateContext->Map(_wvpConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		CB_BOUNDING_OBJECT_PROPERTIES* properties = (CB_BOUNDING_OBJECT_PROPERTIES*)mappedResource.pData;
		XMStoreFloat4x4(&properties->WorldViewProjection, XMMatrixTranspose(wvp));
		pd3dImmediateContext->Unmap(_wvpConstantBuffer, 0);

		pd3dImmediateContext->VSSetConstantBuffers(0, 1, &_wvpConstantBuffer);

		pd3dImmediateContext->DrawIndexed(24, 0, 0);
	}
	END_EVENT_D3D(L"");

	BEGIN_EVENT_D3D(L"OBBs");
	for (UINT i = 0; i < _objects.GetOrientedBoxCount(); i++)
	{
		OrientedBox* obb = _objects.GetOrientedBox(i);

		// Calculate the world matrix
		XMMATRIX rot = XMMatrixRotationQuaternion(XMLoadFloat4(&obb->Orientation));
		XMMATRIX scale = XMMatrixScalingFromVector(XMLoadFloat3(&obb->Extents));
		
		XMVECTOR position = XMLoadFloat3(&obb->Center);
		XMMATRIX translate = XMMatrixTranslationFromVector(position);

		XMMATRIX world = XMMatrixMultiply(scale,  XMMatrixMultiply(rot, translate));

		XMMATRIX wvp = XMMatrixMultiply(world, viewProj);

		V_RETURN(pd3dImmediateContext->Map(_wvpConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		CB_BOUNDING_OBJECT_PROPERTIES* properties = (CB_BOUNDING_OBJECT_PROPERTIES*)mappedResource.pData;
		XMStoreFloat4x4(&properties->WorldViewProjection, XMMatrixTranspose(wvp));
		pd3dImmediateContext->Unmap(_wvpConstantBuffer, 0);

		pd3dImmediateContext->VSSetConstantBuffers(0, 1, &_wvpConstantBuffer);

		pd3dImmediateContext->DrawIndexed(24, 0, 0);
	}
	END_EVENT_D3D(L"");

	BEGIN_EVENT_D3D(L"Frustums");
	for (UINT i = 0; i < _objects.GetFrustumCount(); i++)
	{
		//Frustum* frust = _objects.GetFrustum(i);
	}
	END_EVENT_D3D(L"");

	BEGIN_EVENT_D3D(L"Spheres");
	pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
    pd3dImmediateContext->IASetVertexBuffers(0, 1, sphereVBs, &strides, &offsets);	
	for (UINT i = 0; i < _objects.GetSphereCount(); i++)
	{
		Sphere* sphere = _objects.GetSphere(i);

		// Calculate the world matrix
		float rad = sphere->Radius;

		XMMATRIX scale = XMMatrixScaling(rad, rad, rad);
		XMMATRIX trans = XMMatrixTranslationFromVector(XMLoadFloat3(&sphere->Center));

		XMMATRIX world = XMMatrixMultiply(scale, trans);
		XMMATRIX wvp = XMMatrixMultiply(world, viewProj);

		V_RETURN(pd3dImmediateContext->Map(_wvpConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		CB_BOUNDING_OBJECT_PROPERTIES* properties = (CB_BOUNDING_OBJECT_PROPERTIES*)mappedResource.pData;
		XMStoreFloat4x4(&properties->WorldViewProjection, XMMatrixTranspose(wvp));
		pd3dImmediateContext->Unmap(_wvpConstantBuffer, 0);

		pd3dImmediateContext->VSSetConstantBuffers(0, 1, &_wvpConstantBuffer);

		pd3dImmediateContext->Draw(SPHERE_POINT_COUNT, 0);
	}
	END_EVENT_D3D(L"");

	ID3D11ShaderResourceView* nullSRV[1] = { NULL };
	pd3dImmediateContext->PSSetShaderResources(0, 1, nullSRV);

	END_EVENT_D3D(L"");
	return S_OK;
}

void BoundingObjectPostProcess::fillRingVB(BOUNDING_OBJECT_VERTEX* buffer, UINT startIdx, UINT numSegments,
	const XMFLOAT3& Origin, const XMFLOAT3& MajorAxis, const XMFLOAT3& MinorAxis)
{
    XMVECTOR vOrigin = XMLoadFloat3( &Origin);
    XMVECTOR vMajor = XMLoadFloat3(&MajorAxis);
    XMVECTOR vMinor = XMLoadFloat3(&MinorAxis);

    FLOAT fAngleDelta = XM_2PI / (float)(numSegments - 1);
    // Instead of calling cos/sin for each segment we calculate
    // the sign of the angle delta and then incrementally calculate sin
    // and cosine from then on.
    XMVECTOR cosDelta = XMVectorReplicate(cosf(fAngleDelta));
    XMVECTOR sinDelta = XMVectorReplicate(sinf(fAngleDelta));
    XMVECTOR incrementalSin = XMVectorZero();
    static const XMVECTOR initialCos =
    {
        1.0f, 1.0f, 1.0f, 1.0f
    };
    XMVECTOR incrementalCos = initialCos;
    for(UINT i = startIdx; i < startIdx + numSegments - 1; i++ )
    {
        XMVECTOR Pos;
        Pos = XMVectorMultiplyAdd(vMajor, incrementalCos, vOrigin);
        Pos = XMVectorMultiplyAdd(vMinor, incrementalSin, Pos);
        XMStoreFloat3(&buffer[i].Position, Pos);

        // Standard formula to rotate a vector.
        XMVECTOR newCos = incrementalCos * cosDelta - incrementalSin * sinDelta;
        XMVECTOR newSin = incrementalCos * sinDelta + incrementalSin * cosDelta;
        incrementalCos = newCos;
        incrementalSin = newSin;
    }
    buffer[startIdx + numSegments - 1] = buffer[startIdx];
}


HRESULT BoundingObjectPostProcess::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(PostProcess::OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));

	// Load the pixel shader
	PixelShaderContent* psContent = NULL;
	PixelShaderOptions psOpts =
	{
		"PS_BoundingObject",// const char* EntryPoint;
		NULL,				// D3D_SHADER_MACRO* Defines;
		"Bounding Object",	// const char* DebugName;
	};
	
	V_RETURN(pContentManager->LoadContent(pd3dDevice, L"BoundingObject.hlsl", &psOpts, &psContent));

	_pixelShader = psContent->PixelShader;
	_pixelShader->AddRef();

	SAFE_RELEASE(psContent);

	// Load the sprite vertex shader and input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

	VertexShaderContent* vsContent = NULL;
	VertexShaderOptions vsOpts = 
	{
		"VS_BoundingObject",// const char* EntryPoint;
		NULL,				// D3D_SHADER_MACRO* Defines;
		layout,				// D3D11_INPUT_ELEMENT_DESC* InputElements;
		ARRAYSIZE(layout),	// UINT InputElementCount;
		"Bounding Object",	// const char* DebugName;
	};

	V_RETURN(pContentManager->LoadContent(pd3dDevice, L"BoundingObject.hlsl", &vsOpts, &vsContent));

	_vertexShader = vsContent->VertexShader;
	_vertexShader->AddRef();

	_inputLayout = vsContent->InputLayout;
	_inputLayout->AddRef();

	SAFE_RELEASE(vsContent);
	
	// Create the constant buffer
	D3D11_BUFFER_DESC cbDesc =
	{
		0, //UINT ByteWidth;
		D3D11_USAGE_DYNAMIC, //D3D11_USAGE Usage;
		D3D11_BIND_CONSTANT_BUFFER, //UINT BindFlags;
		D3D11_CPU_ACCESS_WRITE, //UINT CPUAccessFlags;
		0, //UINT MiscFlags;
		0, //UINT StructureByteStride;
	};

	cbDesc.ByteWidth = sizeof(CB_BOUNDING_OBJECT_PROPERTIES);
	V_RETURN(pd3dDevice->CreateBuffer(&cbDesc, NULL, &_wvpConstantBuffer));

	cbDesc.ByteWidth = sizeof(CB_BOUNDING_OBJECT_COLOR);
	V_RETURN(pd3dDevice->CreateBuffer(&cbDesc, NULL, &_colorConstantBuffer));

	// Create the vertex buffers
	D3D11_BUFFER_DESC vbDesc =
    {
        0,
        D3D11_USAGE_DEFAULT,
        D3D11_BIND_VERTEX_BUFFER,
        0,
        0
    };

	D3D11_BUFFER_DESC ibDesc =
    {
        0,
        D3D11_USAGE_DEFAULT,
        D3D11_BIND_INDEX_BUFFER,
        0,
        0
    };

	D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = NULL;
    initData.SysMemPitch = 0;
    initData.SysMemSlicePitch = 0;
	
	BOUNDING_OBJECT_VERTEX boxVerts[8];
	boxVerts[0].Position = XMFLOAT3(-1, -1, -1);	
	boxVerts[1].Position = XMFLOAT3(1, -1, -1);	
	boxVerts[2].Position = XMFLOAT3(1, -1, 1);	
	boxVerts[3].Position = XMFLOAT3(-1, -1, 1);	
	boxVerts[4].Position = XMFLOAT3(-1, 1, -1);
	boxVerts[5].Position = XMFLOAT3(1, 1, -1);	
	boxVerts[6].Position = XMFLOAT3(1, 1, 1);	
	boxVerts[7].Position = XMFLOAT3(-1, 1, 1);

	static const WORD boxIndices[] =
    {
        0, 1,
        1, 2,
        2, 3,
        3, 0,
        4, 5,
        5, 6,
        6, 7,
        7, 4,
        0, 4,
        1, 5,
        2, 6,
        3, 7
    };

    initData.pSysMem = &boxVerts;
	vbDesc.ByteWidth = sizeof(BOUNDING_OBJECT_VERTEX) * 8;
	V_RETURN(pd3dDevice->CreateBuffer(&vbDesc, &initData, &_boxVB));

	initData.pSysMem = &boxIndices;
	ibDesc.ByteWidth = sizeof(WORD) * 24;
	V_RETURN(pd3dDevice->CreateBuffer(&ibDesc, &initData, &_boxIB));

	// Create the sphere vb
	BOUNDING_OBJECT_VERTEX sphereVerts[SPHERE_POINT_COUNT];
	fillRingVB(sphereVerts, (SPHERE_POINT_COUNT / 3) * 0, (SPHERE_POINT_COUNT / 3), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f));
	fillRingVB(sphereVerts, (SPHERE_POINT_COUNT / 3) * 1, (SPHERE_POINT_COUNT / 3), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
	fillRingVB(sphereVerts, (SPHERE_POINT_COUNT / 3) * 2, (SPHERE_POINT_COUNT / 3), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f));

	initData.pSysMem = &sphereVerts;
	vbDesc.ByteWidth = sizeof(BOUNDING_OBJECT_VERTEX) * SPHERE_POINT_COUNT;
	V_RETURN(pd3dDevice->CreateBuffer(&vbDesc, &initData, &_sphereVB));
	
	return S_OK;
}


void BoundingObjectPostProcess::OnD3D11DestroyDevice()
{
	PostProcess::OnD3D11DestroyDevice();

	SAFE_RELEASE(_boxIB);
	SAFE_RELEASE(_boxVB);
	SAFE_RELEASE(_sphereVB);
	SAFE_RELEASE(_inputLayout);
	SAFE_RELEASE(_vertexShader);
	SAFE_RELEASE(_pixelShader);
	SAFE_RELEASE(_wvpConstantBuffer);
	SAFE_RELEASE(_colorConstantBuffer);
}

HRESULT BoundingObjectPostProcess::OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
                        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(PostProcess::OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));

	return S_OK;
}

void BoundingObjectPostProcess::OnD3D11ReleasingSwapChain()
{
	PostProcess::OnD3D11ReleasingSwapChain();
}