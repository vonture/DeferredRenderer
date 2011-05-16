#include "CombinePostProcess.h"

CombinePostProcess::CombinePostProcess()
	: _pixelShader(NULL)
{
}

CombinePostProcess::~CombinePostProcess()
{
}

HRESULT CombinePostProcess::Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
		ID3D11RenderTargetView* dstRTV, ID3D11DepthStencilView* dstDSV, GBuffer* gBuffer,
		LightBuffer* lightBuffer)
{
	HRESULT hr;

	//const DXGI_SURFACE_DESC* pBackBufferDesc = DXUTGetDXGIBackBufferSurfaceDesc();
	
	pd3dImmediateContext->OMSetRenderTargets(1, &dstRTV, dstDSV);

	//const float rtClear[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	//pd3dImmediateContext->ClearRenderTargetView(dstRTV, rtClear);
	//pd3dImmediateContext->ClearDepthStencilView(dstDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

	//pd3dImmediateContext->PSSetShader(_pixelShader, NULL, 0);
	V_RETURN(gBuffer->PSSetShaderResources(pd3dImmediateContext, 0));
	V_RETURN(lightBuffer->PSSetShaderResources(pd3dImmediateContext, 4));

	ID3D11SamplerState* sampler = GetSamplerStates()->GetPoint();
	pd3dImmediateContext->PSSetSamplers(0, 1, &sampler);

	pd3dImmediateContext->OMSetDepthStencilState(GetDepthStencilStates()->GetDepthDisabled(), 0);

	float blendFactor[4] = {1, 1, 1, 1};
	pd3dImmediateContext->OMSetBlendState(GetBlendStates()->GetBlendDisabled(), blendFactor, 0xFFFFFFFF);
	
	V_RETURN(_fsQuad.Render(pd3dImmediateContext, _pixelShader));

	V_RETURN(gBuffer->PSUnsetShaderResources(pd3dImmediateContext, 0));
	V_RETURN(lightBuffer->PSUnsetShaderResources(pd3dImmediateContext, 4));

	return S_OK;
}

HRESULT CombinePostProcess::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(PostProcess::OnD3D11CreateDevice(pd3dDevice,pBackBufferSurfaceDesc));

	ID3DBlob* pBlob = NULL;

	V_RETURN(CompileShaderFromFile( L"GBufferCombine.hlsl", "PS_Combine", "ps_4_0", NULL, &pBlob ) );   
    V_RETURN(pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_pixelShader));
	SAFE_RELEASE(pBlob);

	V_RETURN(_fsQuad.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));

	return S_OK;
}

void CombinePostProcess::OnD3D11DestroyDevice()
{
	PostProcess::OnD3D11DestroyDevice();

	SAFE_RELEASE(_pixelShader);

	_fsQuad.OnD3D11DestroyDevice();
}

HRESULT CombinePostProcess::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(PostProcess::OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_fsQuad.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	
	return S_OK;
}

void CombinePostProcess::OnD3D11ReleasingSwapChain()
{
	PostProcess::OnD3D11ReleasingSwapChain();

	_fsQuad.OnD3D11ReleasingSwapChain();
}