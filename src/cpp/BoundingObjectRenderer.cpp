#include "BoundingObjectRenderer.h"
#include "ShaderLoader.h"

BoundingObjectRenderer::BoundingObjectRenderer()
	: _boxVB(NULL), _boxIB(NULL), _sphereVB(NULL), _inputLayout(NULL), _vertexShader(NULL),
	  _pixelShader(NULL), _constantBuffer(NULL)
{
}

BoundingObjectRenderer::~BoundingObjectRenderer()
{
}

void BoundingObjectRenderer::Add(const AxisAlignedBox& aabb)
{
	if (_nextAABB < MAX_BOUNDING_OBJECTS)
	{
		_aabbs[_nextAABB] = aabb;
		_nextAABB++;
	}
}

void BoundingObjectRenderer::Add(const OrientedBox& obb)
{
	if (_nextOBB < MAX_BOUNDING_OBJECTS)
	{
		_obbs[_nextOBB] = obb;
		_nextOBB++;
	}
}

void BoundingObjectRenderer::Add(const Sphere& sphere)
{
	if (_nextSphere < MAX_BOUNDING_OBJECTS)
	{
		_spheres[_nextSphere] = sphere;
		_nextSphere++;
	}
}

void BoundingObjectRenderer::Add(const Frustum& frust)
{
	if (_nextFrust < MAX_BOUNDING_OBJECTS)
	{
		_frustums[_nextFrust] = frust;
		_nextFrust++;
	}
}

void BoundingObjectRenderer::Clear()
{
	_nextAABB = 0;
	_nextOBB = 0;
	_nextSphere = 0;
	_nextFrust = 0;
}

HRESULT BoundingObjectRenderer::Render(ID3D11DeviceContext* pd3dImmediateContext, Camera* camera)
{
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;	

	XMMATRIX viewProj = camera->GetViewProjection();

	UINT strides = sizeof(BOUNDING_OBJECT_VERTEX);
    UINT offsets = 0;

	ID3D11Buffer* boxVBs[1] = { _boxVB };
	ID3D11Buffer* sphereVBs[1] = { _sphereVB };

	pd3dImmediateContext->VSSetShader( _vertexShader, NULL, 0 );
    pd3dImmediateContext->PSSetShader( _pixelShader, NULL, 0 );
	
	pd3dImmediateContext->IASetInputLayout(_inputLayout);

	pd3dImmediateContext->OMSetDepthStencilState(GetDepthStencilStates()->GetDepthDisabled(), 0);

	float blendFactor[4] = {1, 1, 1, 1};
	pd3dImmediateContext->OMSetBlendState(GetBlendStates()->GetBlendDisabled(), blendFactor, 0xFFFFFFFF);

	pd3dImmediateContext->RSSetState(GetRasterizerStates()->GetWireframe());

	pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	pd3dImmediateContext->IASetIndexBuffer(_boxIB, DXGI_FORMAT_R16_UINT, 0);
    pd3dImmediateContext->IASetVertexBuffers(0, 1, boxVBs, &strides, &offsets);
	for (UINT i = 0; i < _nextAABB; i++)
	{
		// Calculate the world matrix
		XMMATRIX scale = XMMatrixScalingFromVector(XMLoadFloat3(&_aabbs[i].Extents));
		XMMATRIX trans = XMMatrixTranslationFromVector(XMLoadFloat3(&_aabbs[i].Center));

		XMMATRIX world = XMMatrixMultiply(scale, trans);
		XMMATRIX wvp = XMMatrixMultiply(world, viewProj);

		V_RETURN(pd3dImmediateContext->Map(_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		CB_BOUNDING_OBJECT_PROPERTIES* properties = (CB_BOUNDING_OBJECT_PROPERTIES*)mappedResource.pData;
		properties->WorldViewProjection = XMMatrixTranspose(wvp);
		pd3dImmediateContext->Unmap(_constantBuffer, 0);

		pd3dImmediateContext->DrawIndexed(24, 0, 0);
	}

	for (UINT i = 0; i < _nextOBB; i++)
	{
		// Calculate the world matrix
		XMMATRIX rot = XMMatrixRotationQuaternion(XMLoadFloat4( &_obbs[i].Orientation));
		XMMATRIX scale = XMMatrixScalingFromVector(XMLoadFloat3(&_obbs[i].Extents));

		XMMATRIX world = XMMatrixMultiply(rot, scale);

		XMVECTOR position = XMLoadFloat3(&_obbs[i].Center);
		world.r[3] = XMVectorSelect(world.r[3], position, XMVectorSelectControl(1, 1, 1, 0));

		XMMATRIX wvp = XMMatrixMultiply(world, viewProj);

		V_RETURN(pd3dImmediateContext->Map(_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		CB_BOUNDING_OBJECT_PROPERTIES* properties = (CB_BOUNDING_OBJECT_PROPERTIES*)mappedResource.pData;
		properties->WorldViewProjection = XMMatrixTranspose(wvp);
		pd3dImmediateContext->Unmap(_constantBuffer, 0);

		pd3dImmediateContext->VSSetConstantBuffers(0, 1, &_constantBuffer);

		pd3dImmediateContext->DrawIndexed(24, 0, 0);
	}

	for (UINT i = 0; i < _nextFrust; i++)
	{
	}

	pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
    pd3dImmediateContext->IASetVertexBuffers(0, 1, sphereVBs, &strides, &offsets);
	for (UINT i = 0; i < _nextSphere; i++)
	{
		// Calculate the world matrix
		float rad = _spheres[i].Radius;

		XMMATRIX scale = XMMatrixScaling(rad, rad, rad);
		XMMATRIX trans = XMMatrixTranslationFromVector(XMLoadFloat3(&_spheres[i].Center));

		XMMATRIX world = XMMatrixMultiply(scale, trans);
		XMMATRIX wvp = XMMatrixMultiply(world, viewProj);

		V_RETURN(pd3dImmediateContext->Map(_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		CB_BOUNDING_OBJECT_PROPERTIES* properties = (CB_BOUNDING_OBJECT_PROPERTIES*)mappedResource.pData;
		properties->WorldViewProjection = XMMatrixTranspose(wvp);
		pd3dImmediateContext->Unmap(_constantBuffer, 0);

		pd3dImmediateContext->VSSetConstantBuffers(0, 1, &_constantBuffer);

		pd3dImmediateContext->Draw(SPHERE_POINT_COUNT, 0);
	}

	return S_OK;
}

void BoundingObjectRenderer::fillRingVB(BOUNDING_OBJECT_VERTEX* buffer, UINT startIdx, UINT numSegments,
	const XMFLOAT3& Origin, const XMFLOAT3& MajorAxis, const XMFLOAT3& MinorAxis, const XMFLOAT3& Color )
{
    XMVECTOR vOrigin = XMLoadFloat3( &Origin );
    XMVECTOR vMajor = XMLoadFloat3( &MajorAxis );
    XMVECTOR vMinor = XMLoadFloat3( &MinorAxis );

    FLOAT fAngleDelta = XM_2PI / (float)(numSegments - 1);
    // Instead of calling cos/sin for each segment we calculate
    // the sign of the angle delta and then incrementally calculate sin
    // and cosine from then on.
    XMVECTOR cosDelta = XMVectorReplicate( cosf( fAngleDelta ) );
    XMVECTOR sinDelta = XMVectorReplicate( sinf( fAngleDelta ) );
    XMVECTOR incrementalSin = XMVectorZero();
    static const XMVECTOR initialCos =
    {
        1.0f, 1.0f, 1.0f, 1.0f
    };
    XMVECTOR incrementalCos = initialCos;
    for(UINT i = startIdx; i < startIdx + numSegments - 1; i++ )
    {
        XMVECTOR Pos;
        Pos = XMVectorMultiplyAdd( vMajor, incrementalCos, vOrigin );
        Pos = XMVectorMultiplyAdd( vMinor, incrementalSin, Pos );
        XMStoreFloat3(&buffer[i].Position, Pos );
		buffer[i].Color = Color;
        // Standard formula to rotate a vector.
        XMVECTOR newCos = incrementalCos * cosDelta - incrementalSin * sinDelta;
        XMVECTOR newSin = incrementalCos * sinDelta + incrementalSin * cosDelta;
        incrementalCos = newCos;
        incrementalSin = newSin;
    }
    buffer[startIdx + numSegments - 1] = buffer[startIdx];

}


HRESULT BoundingObjectRenderer::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;
	ID3DBlob* pBlob = NULL;

	// Compile the shaders
	V_RETURN( CompileShaderFromFile( L"BoundingObject.hlsl", "PS_BoundingObject", "ps_4_0", NULL, &pBlob ) );   
    V_RETURN( pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_pixelShader));
	SAFE_RELEASE(pBlob);	

	V_RETURN( CompileShaderFromFile( L"BoundingObject.hlsl", "VS_BoundingObject", "vs_4_0", NULL, &pBlob ) );   
    V_RETURN( pd3dDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_vertexShader));

	// Create the input layout
	const D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

	V_RETURN( pd3dDevice->CreateInputLayout(layout, ARRAYSIZE(layout), pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(), &_inputLayout));
	SAFE_RELEASE(pBlob);

	// Create the constant buffer
	D3D11_BUFFER_DESC cbDesc =
	{
		sizeof(CB_BOUNDING_OBJECT_PROPERTIES), //UINT ByteWidth;
		D3D11_USAGE_DYNAMIC, //D3D11_USAGE Usage;
		D3D11_BIND_CONSTANT_BUFFER, //UINT BindFlags;
		D3D11_CPU_ACCESS_WRITE, //UINT CPUAccessFlags;
		0, //UINT MiscFlags;
		0, //UINT StructureByteStride;
	};

	V_RETURN(pd3dDevice->CreateBuffer(&cbDesc, NULL, &_constantBuffer));

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

	XMFLOAT3 boColor = XMFLOAT3(1.0f, 1.0f, 1.0f);

	BOUNDING_OBJECT_VERTEX boxVerts[8];
	boxVerts[0].Position = XMFLOAT3(-1, -1, -1);
	boxVerts[0].Color = boColor;
	
	boxVerts[1].Position = XMFLOAT3(1, -1, -1);
	boxVerts[1].Color = boColor;
	
	boxVerts[2].Position = XMFLOAT3(1, -1, 1);
	boxVerts[2].Color = boColor;
	
	boxVerts[3].Position = XMFLOAT3(-1, -1, 1);
	boxVerts[3].Color = boColor;
	
	boxVerts[4].Position = XMFLOAT3(-1, 1, -1);
	boxVerts[4].Color = boColor;
	
	boxVerts[5].Position = XMFLOAT3(1, 1, -1);
	boxVerts[5].Color = boColor;
	
	boxVerts[6].Position = XMFLOAT3(1, 1, 1);
	boxVerts[6].Color = boColor;
	
	boxVerts[7].Position = XMFLOAT3(-1, 1, 1);
	boxVerts[7].Color = boColor;

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
	fillRingVB(sphereVerts, (SPHERE_POINT_COUNT / 3) * 0, (SPHERE_POINT_COUNT / 3), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), boColor);
	fillRingVB(sphereVerts, (SPHERE_POINT_COUNT / 3) * 1, (SPHERE_POINT_COUNT / 3), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), boColor);
	fillRingVB(sphereVerts, (SPHERE_POINT_COUNT / 3) * 2, (SPHERE_POINT_COUNT / 3), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), boColor);

	initData.pSysMem = &sphereVerts;
	vbDesc.ByteWidth = sizeof(BOUNDING_OBJECT_VERTEX) * SPHERE_POINT_COUNT;
	V_RETURN(pd3dDevice->CreateBuffer(&vbDesc, &initData, &_sphereVB));

	
	V_RETURN(_dsStates.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_samplerStates.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_blendStates.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_rasterStates.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));

	return S_OK;
}


void BoundingObjectRenderer::OnD3D11DestroyDevice()
{
	SAFE_RELEASE(_boxIB);
	SAFE_RELEASE(_boxVB);
	SAFE_RELEASE(_sphereVB);
	SAFE_RELEASE(_inputLayout);
	SAFE_RELEASE(_vertexShader);
	SAFE_RELEASE(_pixelShader);
	SAFE_RELEASE(_constantBuffer);

	_dsStates.OnD3D11DestroyDevice();
	_samplerStates.OnD3D11DestroyDevice();
	_blendStates.OnD3D11DestroyDevice();
	_rasterStates.OnD3D11DestroyDevice();
}

HRESULT BoundingObjectRenderer::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;
	
	V_RETURN(_dsStates.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_samplerStates.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_blendStates.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_rasterStates.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));

	return S_OK;
}

void BoundingObjectRenderer::OnD3D11ReleasingSwapChain()
{
	_dsStates.OnD3D11ReleasingSwapChain();
	_samplerStates.OnD3D11ReleasingSwapChain();
	_blendStates.OnD3D11ReleasingSwapChain();
	_rasterStates.OnD3D11ReleasingSwapChain();
}