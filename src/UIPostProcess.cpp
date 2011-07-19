#include "UIPostProcess.h"
#include "ShaderLoader.h"

UIPostProcess::UIPostProcess()
	: _skin(), _canvas(&_skin)
{
	_skin.SetRender(&_uiRenderer);
	_input.Initialize(&_canvas);
}

UIPostProcess::~UIPostProcess()
{
}

HRESULT UIPostProcess::Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
	ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer, LightBuffer* lightBuffer)
{
	D3DPERF_BeginEvent(D3DCOLOR_COLORVALUE(0.0f, 0.0f, 1.0f, 1.0f), L"UI");

	HRESULT hr;
	
	// Set the parameters of the ui renderer
	_uiRenderer.SetImmediateContext(pd3dImmediateContext);

	// Set the render targets
	pd3dImmediateContext->OMSetRenderTargets(1, &dstRTV, NULL);

	// Prepare the blend, depth and sampler
	pd3dImmediateContext->OMSetDepthStencilState(GetDepthStencilStates()->GetDepthDisabled(), 0);

	float blendFactor[4] = {1, 1, 1, 1};
	pd3dImmediateContext->OMSetBlendState(GetBlendStates()->GetBlendDisabled(), blendFactor, 0xFFFFFFFF);

	Quad* fsQuad = GetFullScreenQuad();

	ID3D11SamplerState* sampler = GetSamplerStates()->GetPoint();
	pd3dImmediateContext->PSSetSamplers(0, 1, &sampler);

	// Prepare the SRVs
	ID3D11ShaderResourceView* ppSRVs[1] = { src };
	pd3dImmediateContext->PSSetShaderResources(0, 1, ppSRVs);

	// Copy over the prev image
	D3DPERF_BeginEvent(D3DCOLOR_COLORVALUE(1.0f, 0.0f, 0.0f, 1.0f), L"Copy");
	V_RETURN(fsQuad->Render(pd3dImmediateContext, _copyPS));
	D3DPERF_EndEvent();

	// Null the SRVs
	ID3D11ShaderResourceView* ppSRVNULL[1] = { NULL };
	pd3dImmediateContext->PSSetShaderResources(0, 1, ppSRVNULL);

	// Render UI
	D3DPERF_BeginEvent(D3DCOLOR_COLORVALUE(1.0f, 0.0f, 0.0f, 1.0f), L"Canvas");
	_canvas.RenderCanvas();
	D3DPERF_EndEvent();

	D3DPERF_EndEvent();

	return S_OK;
}

LRESULT UIPostProcess::OnMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MSG message;
	message.hwnd = hWnd;
	message.message = msg;
	message.wParam = wParam;
	message.lParam = lParam;

	return SUCCEEDED(_input.ProcessMessage(message)) ? 0 : -1;
}

void UIPostProcess::OnFrameMove(double totalTime, float dt)
{
	_canvas.DoThink();
}

HRESULT UIPostProcess::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(PostProcess::OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	_uiRenderer.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc);

	// Load the shaders
	ID3DBlob* pBlob = NULL;

	V_RETURN( CompileShaderFromFile( L"Copy.hlsl", "PS_Copy", "ps_4_0", NULL, &pBlob ) );   
    V_RETURN( pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_copyPS));
	SAFE_RELEASE(pBlob);
	SET_DEBUG_NAME(_copyPS, "UI post process copy pixel shader");

	_skin.Init(L"UI\\skin.png");

	_canvas.SetPos(0, 0);
	_canvas.SetSize(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);	

	return S_OK;
}

void UIPostProcess::OnD3D11DestroyDevice()
{
	PostProcess::OnD3D11DestroyDevice();
	_uiRenderer.OnD3D11DestroyDevice();

	SAFE_RELEASE(_copyPS);
}

HRESULT UIPostProcess::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(PostProcess::OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	_uiRenderer.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc);

	_canvas.SetPos(0, 0);
	_canvas.SetSize(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);

	return S_OK;
}

void UIPostProcess::OnD3D11ReleasingSwapChain()
{
	PostProcess::OnD3D11ReleasingSwapChain();
	_uiRenderer.OnD3D11ReleasingSwapChain();
}