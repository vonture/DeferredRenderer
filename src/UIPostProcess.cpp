#include "UIPostProcess.h"
#include "ShaderLoader.h"
#include "Logger.h"

UIPostProcess::UIPostProcess()
{
	SetIsAdditive(true);

	_skin = new Gwen::Skin::Simple();
	_skin->SetRender(&_uiRenderer);

	_canvas = new Gwen::Controls::Canvas(_skin);
	
	_input = new Gwen::Input::Windows();
	_input->Initialize(_canvas);
}

UIPostProcess::~UIPostProcess()
{
	SAFE_DELETE(_input);
	SAFE_DELETE(_canvas);
	SAFE_DELETE(_skin);
}

HRESULT UIPostProcess::Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
	ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer, LightBuffer* lightBuffer)
{
	BEGIN_EVENT(L"UI");
		
	// Set the parameters of the ui renderer
	_uiRenderer.SetImmediateContext(pd3dImmediateContext);

	// Set the render targets
	pd3dImmediateContext->OMSetRenderTargets(1, &dstRTV, NULL);

	// Render UI
	BEGIN_EVENT(L"Canvas");
	_canvas->RenderCanvas();
	END_EVENT();

	END_EVENT();

	return S_OK;
}

LRESULT UIPostProcess::OnMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MSG message;
	message.hwnd = hWnd;
	message.message = msg;
	message.wParam = wParam;
	message.lParam = lParam;

	return SUCCEEDED(_input->ProcessMessage(message)) ? 0 : -1;
}

void UIPostProcess::OnFrameMove(double totalTime, float dt)
{
	_canvas->DoThink();
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
	
	_canvas->SetPos(0, 0);
	_canvas->SetSize(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);	

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

	_canvas->SetPos(0, 0);
	_canvas->SetSize(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);

	return S_OK;
}

void UIPostProcess::OnD3D11ReleasingSwapChain()
{
	PostProcess::OnD3D11ReleasingSwapChain();
	_uiRenderer.OnD3D11ReleasingSwapChain();
}