#include "DeferredRendererApplication.h"

#include "HDRConfigurationPane.h"
#include "MLAAConfigurationPane.h"
#include "SkyConfigurationPane.h"
#include "CameraConfigurationPane.h"
#include "SSAOConfigurationPane.h"

DeferredRendererApplication::DeferredRendererApplication()
	: Application(L"Deferred Renderer", NULL), _renderer(), _camera(0.1f, 40.0f, 1.0f, 1.0f), _configWindow(NULL),
	  _scene(L"\\models\\tankscene\\tankscene.sdkmesh")
{
	_scene.SetScale(1.0f);
	_scene.SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));

	_camera.SetPosition(XMFLOAT3(1.0f, 4.0f, -6.0f));
	_camera.SetRotation(XMFLOAT2(-0.1f, 0.35f));

	_uiEnabled = true;
}

DeferredRendererApplication::~DeferredRendererApplication()
{
}

void DeferredRendererApplication::OnInitialize()
{		
}

void DeferredRendererApplication::OnPreparingDeviceSettings(DeviceManager* deviceManager)
{
	Application::OnPreparingDeviceSettings(deviceManager);

	deviceManager->SetBackBufferWidth(1360);
	deviceManager->SetBackBufferHeight(768);	
	//deviceManager->SetFullScreen(false);

	deviceManager->SetVSyncEnabled(false);
}

void DeferredRendererApplication::OnFrameMove(double totalTime, float dt)
{
	HWND hwnd = GetHWND();
	KeyboardState kb = KeyboardState::GetState();
	MouseState mouse = MouseState::GetState(hwnd);

	if (IsActive() && mouse.IsOverWindow())
	{
		if (kb.IsKeyJustPressed(Esc))
		{
			Exit();
		}

		if (kb.IsKeyJustPressed(B))
		{
			_renderer.SetDrawBoundingObjects(!_renderer.GetDrawBoundingObjects());
		}

		if (kb.IsKeyJustPressed(F11))
		{
			SetFullScreen(!GetFullScreen());
		}

		if (kb.IsKeyJustPressed(M))
		{
			SetMaximized(!GetMaximized());
		}
	
		if (kb.IsKeyJustPressed(U))
		{
			_uiEnabled = !_uiEnabled;
		}

		if (mouse.IsButtonDown(RightButton))
		{
			const float mouseRotateSpeed = _camera.GetRotationSpeed();
		
			XMFLOAT2 rotation = _camera.GetRotation();
			rotation.x += mouse.GetDX() * mouseRotateSpeed;
			rotation.y += mouse.GetDY() * mouseRotateSpeed;
			_camera.SetRotation(rotation);

			// Set the mouse back one frame
			//MouseState::SetCursorPosition(mouse.GetX() - mouse.GetDX(), mouse.GetY() - mouse.GetDY(), hwnd);

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
				
			const float cameraMoveSpeed = _camera.GetMovementSpeed();
			XMFLOAT3 camPos = _camera.GetPosition();
			XMFLOAT3 camForward = _camera.GetForward();
			XMFLOAT3 camRight = _camera.GetRight();

			XMVECTOR position = XMLoadFloat3(&camPos);
			XMVECTOR forward = XMLoadFloat3(&camForward);
			XMVECTOR right = XMLoadFloat3(&camRight);

			position += ((moveDir.y * forward) + (moveDir.x * right)) * (cameraMoveSpeed * dt);

			XMStoreFloat3(&camPos, position);
			_camera.SetPosition(camPos);
		}
	}

	_uiPP.OnFrameMove(totalTime, dt);
	_configWindow->OnFrameMove(totalTime, dt);
}

LRESULT DeferredRendererApplication::OnMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HRESULT hr;
	
	V_RETURN(_uiPP.OnMessage(hWnd, msg, wParam, lParam));

	return Application::OnMessage(hWnd, msg, wParam, lParam);
}

HRESULT DeferredRendererApplication::OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext)
{
	HRESULT hr;

	V_RETURN(_renderer.Begin());

	_renderer.AddModel(&_scene);	
	
	/*
	PointLight greenLight = 
	{
		XMFLOAT3(-8.5f, 9.5f, 9.2f),
		10.0f,
		XMFLOAT3(1.5f, 3.0f, 1.0f)
	};
	_renderer.AddLight(&greenLight, true);

	PointLight redLight = 
	{
		XMFLOAT3(-1.6f, 4.5f, -4.2f),
		12.0f,
		XMFLOAT3(2.5f, 1.0f, 1.0f)
	};
	_renderer.AddLight(&redLight, true);

	PointLight purpleLight = 
	{
		XMFLOAT3(9.6f, 3.6f, 4.5f),
		15.0f,
		XMFLOAT3(1.5f, 0.0f, 3.0f)
	};
	_renderer.AddLight(&purpleLight, true);
	*/

	if (_skyPP.GetSunEnabled())
	{
		const float sunIntensity = _skyPP.GetSunIntensity();
		const XMFLOAT3 sunColor = _skyPP.GetSunColor();
		const XMFLOAT3 sunDir = _skyPP.GetSunDirection();
		DirectionalLight sun = 
		{
			sunDir,
			XMFLOAT3(sunColor.x * sunIntensity, sunColor.y * sunIntensity, sunColor.z * sunIntensity)
		};
		_renderer.AddLight(&sun, true);
	}
	
	float ambientIntesity = 1.0f;
	AmbientLight ambientLight = 
	{
		XMFLOAT3(ambientIntesity, ambientIntesity, ambientIntesity)
	};
	_renderer.AddLight(&ambientLight);

	_renderer.AddPostProcess(&_ssaoPP);
	_renderer.AddPostProcess(&_skyPP);	
	_renderer.AddPostProcess(&_mlaaPP);
	_renderer.AddPostProcess(&_hdrPP);

	// Unimplimented post processes
	//_renderer.AddPostProcess(&_dofPP);
	//_renderer.AddPostProcess(&_motionBlurPP);

	if (_uiEnabled)
	{
		_renderer.AddPostProcess(&_uiPP);
	}

	V_RETURN(_renderer.End(pd3dImmediateContext, &_camera));

	return S_OK;
}

HRESULT DeferredRendererApplication::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(Application::OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));

	V_RETURN(_renderer.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));	
	V_RETURN(_hdrPP.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_skyPP.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_mlaaPP.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_ssaoPP.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_dofPP.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_motionBlurPP.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_uiPP.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_scene.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));

	Gwen::Controls::Canvas* canvas = _uiPP.GetCanvas();

	_configWindow = new ConfigurationWindow(canvas);
	_configWindow->SetBounds(10, 10, 260, pBackBufferSurfaceDesc->Height - 20);

	CameraConfigurationPane* cameraPane = new CameraConfigurationPane(_configWindow, &_camera);
	_configWindow->AddConfigPane(cameraPane);

	MLAAConfigurationPane* mlaaPane = new MLAAConfigurationPane(_configWindow, &_mlaaPP);
	_configWindow->AddConfigPane(mlaaPane);

	SSAOConfigurationPane* ssaoPane = new SSAOConfigurationPane(_configWindow, &_ssaoPP);
	_configWindow->AddConfigPane(ssaoPane);

	HDRConfigurationPane* hdrPane = new HDRConfigurationPane(_configWindow, &_hdrPP);
	_configWindow->AddConfigPane(hdrPane);
		
	SkyConfigurationPane* skyPane = new SkyConfigurationPane(_configWindow, &_skyPP);
	_configWindow->AddConfigPane(skyPane);

	return S_OK;
}

void DeferredRendererApplication::OnD3D11DestroyDevice()
{
	Application::OnD3D11DestroyDevice();

	_renderer.OnD3D11DestroyDevice();	
	_hdrPP.OnD3D11DestroyDevice();
	_skyPP.OnD3D11DestroyDevice();
	_mlaaPP.OnD3D11DestroyDevice();
	_ssaoPP.OnD3D11DestroyDevice();
	_dofPP.OnD3D11DestroyDevice();
	_motionBlurPP.OnD3D11DestroyDevice();
	_uiPP.OnD3D11DestroyDevice();
	_scene.OnD3D11DestroyDevice();	
}

HRESULT DeferredRendererApplication::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(Application::OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));

	float fAspectRatio = pBackBufferSurfaceDesc->Width / (float)pBackBufferSurfaceDesc->Height;
	_camera.SetAspectRatio(fAspectRatio);
	
	V_RETURN(_renderer.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));	
	V_RETURN(_hdrPP.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_skyPP.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_mlaaPP.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_ssaoPP.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_dofPP.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_motionBlurPP.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_uiPP.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_scene.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));

	return S_OK;
}
void DeferredRendererApplication::OnD3D11ReleasingSwapChain()
{
	Application::OnD3D11ReleasingSwapChain();

	_renderer.OnD3D11ReleasingSwapChain();	
	_hdrPP.OnD3D11ReleasingSwapChain();
	_skyPP.OnD3D11ReleasingSwapChain();
	_mlaaPP.OnD3D11ReleasingSwapChain();
	_ssaoPP.OnD3D11ReleasingSwapChain();
	_dofPP.OnD3D11ReleasingSwapChain();
	_motionBlurPP.OnD3D11ReleasingSwapChain();
	_uiPP.OnD3D11ReleasingSwapChain();
	_scene.OnD3D11ReleasingSwapChain();
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	DeferredRendererApplication app;
	app.Start();
}