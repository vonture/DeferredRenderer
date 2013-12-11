#include "PCH.h"
#include "UIPostProcess.h"
#include "Logger.h"
#include "PixelShaderLoader.h"

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
                              ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer, ParticleBuffer* pBuffer,
                              LightBuffer* lightBuffer)
{
    BEGIN_EVENT_D3D(L"UI");

    // Set the parameters of the ui renderer
    _uiRenderer.SetImmediateContext(pd3dImmediateContext);

    // Set the render targets
    pd3dImmediateContext->OMSetRenderTargets(1, &dstRTV, NULL);

    // Render UI
    BEGIN_EVENT_D3D(L"Canvas");
    _canvas->RenderCanvas();
    END_EVENT_D3D(L"");

    END_EVENT_D3D(L"");

    return S_OK;
}

LRESULT UIPostProcess::OnMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    MSG message;
    message.hwnd = hWnd;
    message.message = msg;
    message.wParam = wParam;
    message.lParam = lParam;

    return _input->ProcessMessage(message) ? 0 : 1;
}

void UIPostProcess::OnFrameMove(double totalTime, float dt)
{
    _canvas->DoThink();
}

HRESULT UIPostProcess::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager,
                                           const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
    HRESULT hr;

    V_RETURN(PostProcess::OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));
    _uiRenderer.OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc);

    _canvas->SetPos(0, 0);
    _canvas->SetSize(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);

    return S_OK;
}

void UIPostProcess::OnD3D11DestroyDevice(ContentManager* pContentManager)
{
    PostProcess::OnD3D11DestroyDevice(pContentManager);
    _uiRenderer.OnD3D11DestroyDevice(pContentManager);
}

HRESULT UIPostProcess::OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager,
                                               IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
    HRESULT hr;

    V_RETURN(PostProcess::OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));
    _uiRenderer.OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc);

    _canvas->SetPos(0, 0);
    _canvas->SetSize(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);

    return S_OK;
}

void UIPostProcess::OnD3D11ReleasingSwapChain(ContentManager* pContentManager)
{
    PostProcess::OnD3D11ReleasingSwapChain(pContentManager);
    _uiRenderer.OnD3D11ReleasingSwapChain(pContentManager);
}