#include "DXUT.h"
#include "Game.h"

Game::Game()
	: _renderer(), _camera(0.1f, 100.0f, 1.0f, 1.0f), _powerPlant(L"\\models\\powerplant\\powerplant.sdkmesh"),
	 _tankScene(L"\\models\\tankscene\\tankscene.sdkmesh")
{
	_camera.SetPosition(D3DXVECTOR3(-5.0f, 5.0f, -5.0f));
	_camera.SetRotation(D3DXVECTOR2(D3DX_PI / 4.0f, D3DX_PI / 8.0f));
	//_tankScene.SetScale(D3DXVECTOR3(100.0f, 100.0f, 100.0f));
}

Game::~Game()
{
}

void Game::OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown)
{
}

void Game::OnFrameMove(double totalTime, float dt)
{
}

void Game::OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext)
{
	HRESULT hr;

	V(_renderer.Begin());

	//_renderer.AddModel(&_powerPlant);
	_renderer.AddModel(&_tankScene);

	V(_renderer.End(pd3dDevice, pd3dImmediateContext, &_camera));
}

HRESULT Game::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(_renderer.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	//V_RETURN(_powerPlant.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_tankScene.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));

	return S_OK;
}

void Game::OnD3D11DestroyDevice()
{
	_renderer.OnD3D11DestroyDevice();
	//_powerPlant.OnD3D11DestroyDevice();
	_tankScene.OnD3D11DestroyDevice();
}

HRESULT Game::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	float fAspectRatio = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
	_camera.SetAspectRatio(fAspectRatio);

	V_RETURN(_renderer.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	//V_RETURN(_powerPlant.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_tankScene.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));

	return S_OK;
}
void Game::OnD3D11ReleasingSwapChain()
{
	_renderer.OnD3D11ReleasingSwapChain();
	//_powerPlant.OnD3D11ReleasingSwapChain();
	_tankScene.OnD3D11ReleasingSwapChain();
}