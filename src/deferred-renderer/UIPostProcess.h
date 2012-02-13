#pragma once

#include "PCH.h"
#include "PostProcess.h"
#include "UIRenderer.h"
#include "IUpdateable.h"

#include "Gwen/Skins/Simple.h"
#include "Gwen/Skins/TexturedBase.h"
#include "Gwen/Controls/Canvas.h"
#include "Gwen/Input/Windows.h"

class UIPostProcess : public PostProcess, public IUpdateable
{
private:
	Gwen::Skin::Simple* _skin;
	Gwen::Controls::Canvas* _canvas;
	Gwen::Input::Windows* _input;

	UIRenderer _uiRenderer;

public:
	UIPostProcess();
	~UIPostProcess();

	Gwen::Controls::Canvas* GetCanvas() { return _canvas; }

	HRESULT Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
		ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer, ParticleBuffer* pBuffer,LightBuffer* lightBuffer);

	LRESULT OnMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void OnFrameMove(double totalTime, float dt);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11DestroyDevice(ContentManager* pContentManager);

	HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain(ContentManager* pContentManager);
};