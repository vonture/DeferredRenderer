#include "Game.h"

Game::Game()
	: _renderer(), _camera(0.1f, 40.0f, 1.0f, 1.0f),
	  _scene(L"\\models\\tankscene\\tankscene.sdkmesh")
	  //_scene(L"\\models\\sponza\\sponzanoflag.sdkmesh")
{
	_scene.SetScale(1.0f);
	_scene.SetPosition(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f));
	//_scene.SetOrientation(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f));

	_camera.SetPosition(XMVectorSet(1.0f, 4.0f, -6.0f, 1.0f));
	_camera.SetXRotation(-0.1f);
	_camera.SetYRotation(0.35f);

	_directionalLights.push_back(
		DirectionalLight(XMVectorSet(1.0f, 0.8f, 0.5f, 1.0f), 5.0f, XMVectorSet(0.5f, 0.6f, 0.5f, 1.0f)));
	//_directionalLights.push_back(
	//	DirectionalLight(XMVectorSet(0.6f, 1.0f, 0.3f, 1.0f), 1.2f, XMVectorSet(0.5f, 0.5f, -0.5f, 1.0f)));
	//_directionalLights.push_back(
	//	DirectionalLight(XMVectorSet(1.0f, 1.0f, 0.3f, 1.0f), 0.4f, XMVectorSet(0.8f, 0.9, 0.8f, 1.0f)));
	
	_pointLights.push_back(
		PointLight(XMVectorSet(0.6f, 1.0f, 0.5f, 1.0f), 8.0f, XMVectorSet(3.0f, 3.0f, -4.0f, 1.0f), 10.0f));
	//_pointLights.push_back(
	//	PointLight(XMVectorSet(1.0f, 0.0f, 0.3f, 1.0f), 0.9f, XMVectorSet(11.0f, 5.0f, 6.5f, 1.0f), 9.0f));
	//_pointLights.push_back(
	//	PointLight(XMVectorSet(0.0f, 1.0f, 1.0f, 1.0f), 0.8f, XMVectorSet(-7.0f, 6.0f, 5.0f, 1.0f), 12.0f));
		
}

Game::~Game()
{
}

void Game::OnFrameMove(double totalTime, float dt)
{
	KeyboardState kb = KeyboardState::GetState();
	MouseState mouse = MouseState::GetState();

	if (kb.IsKeyJustPressed(B))
	{
		_renderer.SetDrawBoundingObjects(!_renderer.GetDrawBoundingObjects());
	}

	if (kb.IsKeyJustPressed(F11))
	{
		DXUTToggleFullScreen();
	}
	
	if (mouse.IsButtonDown(LeftButton))
	{
		const float mouseRotateSpeed = 0.002f;
		_camera.SetRotation(_camera.GetXRotation() + (mouse.GetDX() * mouseRotateSpeed),
						    _camera.GetYRotation() + (mouse.GetDY() * mouseRotateSpeed));

		// Set the mouse back one frame 
		MouseState::SetCursorPosition(mouse.GetX() - mouse.GetDX(), mouse.GetY() - mouse.GetDY());

		XMFLOAT2 moveDir = XMFLOAT2(0.0f, 0.0f);
		if (kb.IsKeyDown(W) || kb.IsKeyDown(Up))
		{
			moveDir.y++;
		}
		if (kb.IsKeyDown(S) || kb.IsKeyDown(Down))
		{
			moveDir.y--;
		}
		if (kb.IsKeyDown(A) || kb.IsKeyDown(Left))
		{
			moveDir.x--;
		}
		if (kb.IsKeyDown(D) || kb.IsKeyDown(Right))
		{
			moveDir.x++;
		}
				
		const float cameraMoveSpeed = 5.0f;
		XMVECTOR cameraMove = ((moveDir.y * _camera.GetForward()) + (moveDir.x * _camera.GetRight())) * 
			(cameraMoveSpeed * dt);

		_camera.SetPosition(XMVectorAdd(_camera.GetPosition(), cameraMove));
	}

	_hdrPP.SetTimeDelta(dt);

	XMVECTOR skyColor = XMVectorSet(0.2f, 0.5f, 1.0f, 1.0f);
	_skyPP.SetSkyColor(skyColor);
	
	if (_directionalLights.size() > 0)
	{
		XMVECTOR sunCol = _directionalLights[0].GetColor();
		XMVECTOR sunDir = _directionalLights[0].GetDirection();
		float intensity = _directionalLights[0].GetItensity();

		_skyPP.SetSunColor(sunCol);
		_skyPP.SetSunDirection(sunDir);
		_skyPP.SetSunWidth(0.01f * intensity);
		_skyPP.SetSunEnabled(true);
	}
	else
	{
		_skyPP.SetSunEnabled(false);
	}
}

void Game::OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext)
{
	HRESULT hr;

	V(_renderer.Begin());

	_renderer.AddModel(&_scene);
		
	for (UINT i = 0; i < _directionalLights.size(); i++)
	{
		_renderer.AddLight(&_directionalLights[i], true);
	}

	for (UINT i = 0; i < _pointLights.size(); i++)
	{
		_renderer.AddLight(&_pointLights[i], true);
	}

	_renderer.AddPostProcess(&_skyPP);
	_renderer.AddPostProcess(&_hdrPP);

	V(_renderer.End(pd3dDevice, pd3dImmediateContext, &_camera));
}

HRESULT Game::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(_renderer.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));	
	V_RETURN(_hdrPP.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_skyPP.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_scene.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));

	return S_OK;
}

void Game::OnD3D11DestroyDevice()
{
	_renderer.OnD3D11DestroyDevice();	
	_hdrPP.OnD3D11DestroyDevice();
	_skyPP.OnD3D11DestroyDevice();
	_scene.OnD3D11DestroyDevice();	
}

HRESULT Game::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	float fAspectRatio = pBackBufferSurfaceDesc->Width / (float)pBackBufferSurfaceDesc->Height;
	_camera.SetAspectRatio(fAspectRatio);
	
	V_RETURN(_renderer.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));	
	V_RETURN(_hdrPP.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_skyPP.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_scene.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));

	return S_OK;
}
void Game::OnD3D11ReleasingSwapChain()
{
	_renderer.OnD3D11ReleasingSwapChain();	
	_hdrPP.OnD3D11ReleasingSwapChain();
	_skyPP.OnD3D11ReleasingSwapChain();
	_scene.OnD3D11ReleasingSwapChain();
}