#pragma once

#include "PCH.h"
#include "IHasContent.h"

class BlendStates : public IHasContent
{
private:
	ID3D11BlendState* _blendDisabled;
	ID3D11BlendState* _additiveBlend;
	ID3D11BlendState* _alphaBlend;
	ID3D11BlendState* _pmAlphaBlend;
	ID3D11BlendState* _noColor;

	static D3D11_BLEND_DESC getBlendDisabledDesc();
	static D3D11_BLEND_DESC getAdditiveBlendDesc();
	static D3D11_BLEND_DESC getAlphaBlendDesc();
	static D3D11_BLEND_DESC getPreMultipliedAlphaBlendDesc();
	static D3D11_BLEND_DESC getColorWriteDisabledDesc();

public:
	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	void OnD3D11DestroyDevice();
	HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();

	ID3D11BlendState* GetBlendDisabled() { return _blendDisabled; };
	ID3D11BlendState* GetAdditiveBlend() { return _additiveBlend; };
	ID3D11BlendState* GetAlphaBlend() { return _alphaBlend; };
	ID3D11BlendState* GetPreMultipliedAlphaBlend() { return _pmAlphaBlend; };
	ID3D11BlendState* GetColorWriteDisabled() { return _noColor; };
};


class RasterizerStates : public IHasContent
{
private:
	ID3D11RasterizerState* _noCull;
	ID3D11RasterizerState* _cullBackFaces;
	ID3D11RasterizerState* _cullBackFacesScissor;
	ID3D11RasterizerState* _cullFrontFaces;
	ID3D11RasterizerState* _cullFrontFacesScissor;
	ID3D11RasterizerState* _noCullNoMS;
	ID3D11RasterizerState* _noCullScissor;
	ID3D11RasterizerState* _wireframe;

	static D3D11_RASTERIZER_DESC getNoCullDesc();
	static D3D11_RASTERIZER_DESC getFrontFaceCullDesc();
	static D3D11_RASTERIZER_DESC getFrontFaceCullScissorDesc();
	static D3D11_RASTERIZER_DESC getBackFaceCullDesc();
	static D3D11_RASTERIZER_DESC getBackFaceCullScissorDesc();
	static D3D11_RASTERIZER_DESC getNoCullNoMSDesc();
	static D3D11_RASTERIZER_DESC getNoCullScissorDesc();
	static D3D11_RASTERIZER_DESC getWireframeDesc();

public:
	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	void OnD3D11DestroyDevice();
	HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();

	ID3D11RasterizerState* GetNoCull() { return _noCull; };
	ID3D11RasterizerState* GetBackFaceCull() { return _cullBackFaces; };
	ID3D11RasterizerState* GetBackFaceCullScissor() { return _cullBackFacesScissor; };
	ID3D11RasterizerState* GetFrontFaceCull() { return _cullFrontFaces; };
	ID3D11RasterizerState* GetFrontFaceCullScissor() { return _cullFrontFacesScissor; };
	ID3D11RasterizerState* GetNoCullNoMS() { return _noCullNoMS; };
	ID3D11RasterizerState* GetNoCullScissor() { return _noCullScissor; };
	ID3D11RasterizerState* GetWireframe() { return _wireframe; };
};


class DepthStencilStates : public IHasContent
{
private:
	ID3D11DepthStencilState* _stencilReplace;
	ID3D11DepthStencilState* _stencilEqual;
	ID3D11DepthStencilState* _stencilNotEqual;
	ID3D11DepthStencilState* _depthDisabled;
	ID3D11DepthStencilState* _depthEnabled;
	ID3D11DepthStencilState* _revDepthEnabled;
	ID3D11DepthStencilState* _depthWriteEnabled;
	ID3D11DepthStencilState* _revDepthWriteEnabled;

	static D3D11_DEPTH_STENCIL_DESC getStencilReplaceDesc();
	static D3D11_DEPTH_STENCIL_DESC getStencilEqualDesc();
	static D3D11_DEPTH_STENCIL_DESC getStencilNotEqualDesc();

	static D3D11_DEPTH_STENCIL_DESC getDepthDisabledDesc();
	static D3D11_DEPTH_STENCIL_DESC getDepthEnabledDesc();
	static D3D11_DEPTH_STENCIL_DESC getReverseDepthEnabledDesc();
	static D3D11_DEPTH_STENCIL_DESC getDepthWriteEnabledDesc();
	static D3D11_DEPTH_STENCIL_DESC getReverseDepthWriteEnabledDesc();

public:
	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	void OnD3D11DestroyDevice();
	HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();

	ID3D11DepthStencilState* GetStencilReplace() { return _stencilReplace; };
	ID3D11DepthStencilState* GetStencilEqual() { return _stencilEqual; };
	ID3D11DepthStencilState* GetStencilNotEqual() { return _stencilNotEqual; };
	ID3D11DepthStencilState* GetDepthDisabled() { return _depthDisabled; };
	ID3D11DepthStencilState* GetDepthEnabled() { return _depthEnabled; };
	ID3D11DepthStencilState* GetReverseDepthEnabled() { return _revDepthEnabled; };
	ID3D11DepthStencilState* GetDepthWriteEnabled() { return _depthWriteEnabled; };
	ID3D11DepthStencilState* GetReverseDepthWriteEnabled() { return _revDepthWriteEnabled; };
};


class SamplerStates : public IHasContent
{
private:
	ID3D11SamplerState* _linearClamp;
	ID3D11SamplerState* _pointClamp;
	ID3D11SamplerState* _anisotropic16Clamp;
	ID3D11SamplerState* _linearWrap;
	ID3D11SamplerState* _pointWrap;
	ID3D11SamplerState* _anisotropic16Wrap;
	ID3D11SamplerState* _shadowMap;
	
	static D3D11_SAMPLER_DESC getLinearClampDesc();
	static D3D11_SAMPLER_DESC getPointClampDesc();
	static D3D11_SAMPLER_DESC getAnisotropic16ClampDesc();	
	static D3D11_SAMPLER_DESC getLinearWrapDesc();
	static D3D11_SAMPLER_DESC getPointWrapDesc();
	static D3D11_SAMPLER_DESC getAnisotropic16WrapDesc();
	static D3D11_SAMPLER_DESC getShadowMapDesc();

public:
	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	void OnD3D11DestroyDevice();
	HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();

	ID3D11SamplerState* GetLinearClamp() { return _linearClamp; };
	ID3D11SamplerState* GetPointClamp() { return _pointClamp; };
	ID3D11SamplerState* GetAnisotropic16Clamp() { return _anisotropic16Clamp; };
	ID3D11SamplerState* GetLinearWrap() { return _linearWrap; };
	ID3D11SamplerState* GetPointWrap() { return _pointWrap; };
	ID3D11SamplerState* GetAnisotropic16Wrap() { return _anisotropic16Wrap; };
	ID3D11SamplerState* GetShadowMap() { return _shadowMap; };
};