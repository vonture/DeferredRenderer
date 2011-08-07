#pragma once

#include "PCH.h"
#include "IHasContent.h"
#include "ShaderLoader.h"

struct QUAD_VERTEX
{
    XMFLOAT4 Position;
    XMFLOAT2 TexCoord;
};

class Quad : public IHasContent
{
private:
	ID3D11VertexShader* _vertexShader;
	ID3D11InputLayout* _inputLayout;
	ID3D11Buffer* _vertexBuffer;

protected:
	virtual void fillVerticies(QUAD_VERTEX* verts) = 0;

public:
	Quad();
	~Quad();

	HRESULT Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11PixelShader* pixelShader);

	virtual HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	virtual void OnD3D11DestroyDevice();

	virtual HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	virtual void OnD3D11ReleasingSwapChain();
};