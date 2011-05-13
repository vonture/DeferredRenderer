#pragma once

#include "Defines.h"
#include "IUpdateable.h"
#include "IHasContent.h"
#include "Renderer.h"
#include "Camera.h"
#include "KeyboardState.h"
#include "MouseState.h"

class Game : public IUpdateable, public IHasContent
{
private:
	Renderer _renderer;
	FirstPersonCamera _camera;

	ModelInstance _powerPlant;
	ModelInstance _tankScene;
	DirectionalLight _dLight1;
	DirectionalLight _dLight2;

public:
	Game();
	~Game();

	void OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown);
	void OnFrameMove(double totalTime, float dt);
	void OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};