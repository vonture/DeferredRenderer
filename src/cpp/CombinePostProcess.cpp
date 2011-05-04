#include "DXUT.h"
#include "CombinePostProcess.h"

CombinePostProcess::CombinePostProcess()
	: _pixelShader(NULL), _sampler(NULL)
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

	const float rtClear[] = { 0.2f, 0.2f, 0.2f, 0.0f };
	pd3dImmediateContext->ClearRenderTargetView(dstRTV, rtClear);
	pd3dImmediateContext->ClearDepthStencilView(dstDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

	//pd3dImmediateContext->PSSetShader(_pixelShader, NULL, 0);
	V_RETURN(gBuffer->PSSetShaderResources(pd3dImmediateContext, 0));
	V_RETURN(lightBuffer->PSSetShaderResources(pd3dImmediateContext, 4));

	pd3dImmediateContext->PSSetSamplers(0, 1, &_sampler);
	
	V_RETURN(_fsQuad.Render(pd3dImmediateContext, _pixelShader));

	V_RETURN(gBuffer->PSUnsetShaderResources(pd3dImmediateContext, 0));
	V_RETURN(lightBuffer->PSUnsetShaderResources(pd3dImmediateContext, 4));

	return S_OK;
}

HRESULT CombinePostProcess::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;
	ID3DBlob* pBlob = NULL;

	V_RETURN(CompileShaderFromFile( L"GBufferCombine.hlsl", "PS_Combine", "ps_4_0", &pBlob ) );   
    V_RETURN(pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_pixelShader));
	SAFE_RELEASE(pBlob);

	/*D3D11_SAMPLER_DESC samplerDesc = 
    {
        D3D11_FILTER_MIN_MAG_MIP_LINEAR,// D3D11_FILTER Filter;
        D3D11_TEXTURE_ADDRESS_BORDER, //D3D11_TEXTURE_ADDRESS_MODE AddressU;
        D3D11_TEXTURE_ADDRESS_BORDER, //D3D11_TEXTURE_ADDRESS_MODE AddressV;
        D3D11_TEXTURE_ADDRESS_BORDER, //D3D11_TEXTURE_ADDRESS_MODE AddressW;
        0,//FLOAT MipLODBias;
        1,//UINT MaxAnisotropy;
        D3D11_COMPARISON_LESS , //D3D11_COMPARISON_FUNC ComparisonFunc;
        0.0,0.0,0.0,0.0,//FLOAT BorderColor[ 4 ];
        0,//FLOAT MinLOD;
        0//FLOAT MaxLOD;   
    };*/

	D3D11_SAMPLER_DESC samplerDesc;
    ZeroMemory(&samplerDesc, sizeof(samplerDesc));
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

	V_RETURN(pd3dDevice->CreateSamplerState(&samplerDesc, &_sampler));

	V_RETURN(_fsQuad.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));

	return S_OK;
}

void CombinePostProcess::OnD3D11DestroyDevice()
{
	SAFE_RELEASE(_pixelShader);
	SAFE_RELEASE(_sampler);

	_fsQuad.OnD3D11DestroyDevice();
}

HRESULT CombinePostProcess::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(_fsQuad.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	
	return S_OK;
}

void CombinePostProcess::OnD3D11ReleasingSwapChain()
{
	_fsQuad.OnD3D11ReleasingSwapChain();
}