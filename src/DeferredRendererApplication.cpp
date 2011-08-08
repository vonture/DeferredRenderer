#include "PCH.h"
#include "DeferredRendererApplication.h"

#include "HDRConfigurationPane.h"
#include "MLAAConfigurationPane.h"
#include "FXAAConfigurationPane.h"
#include "SkyConfigurationPane.h"
#include "CameraConfigurationPane.h"
#include "SSAOConfigurationPane.h"
#include "PoissonDoFConfigurationPane.h"
#include "DeviceManagerConfigurationPane.h"
#include "ProfilePane.h"

DeferredRendererApplication::DeferredRendererApplication()
	: Application(L"Deferred Renderer", NULL), _renderer(), _camera(0.1f, 40.0f, 1.0f, 1.0f), 
	  _configWindow(NULL), _logWindow(NULL), _ppConfigPane(NULL),
	  _scene(L"\\models\\tankscene\\tankscene.sdkmesh")
{
	_scene.SetScale(1.0f);
	_scene.SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
	
	//XMFLOAT4 orientation;
	//XMStoreFloat4(&orientation, XMQuaternionRotationRollPitchYaw(PiOver2, 0.0f, 0.0f));
	//_scene.SetOrientation(orientation);

	_camera.SetPosition(XMFLOAT3(1.0f, 4.0f, -6.0f));
	_camera.SetRotation(XMFLOAT2(-0.1f, 0.35f));
}

DeferredRendererApplication::~DeferredRendererApplication()
{
}

void DeferredRendererApplication::OnInitialize()
{		
	// Set some properties of the renderer
	_renderer.SetBoundingObjectDrawTypes(BoundingObjectDrawType::None);

	_renderer.AddLightRenderer(&_paraboloidPointLR);
	_renderer.AddLightRenderer(&_cascadedDirectionalLR);
	_renderer.AddLightRenderer(&_spotLR);

	// Create all the UI elements
	Gwen::Controls::Canvas* canvas = _uiPP.GetCanvas();

	// Create the configuration window and its panes
	_configWindow = new ConfigurationWindow(canvas);	

	_ppConfigPane = new PostProcessSelectionPane(_configWindow);
	_ppConfigPane->AddPostProcess(&_ssaoPP, L"SSAO", true, true);
	_ppConfigPane->AddPostProcess(&_skyPP, L"Sky", true, true);
	_ppConfigPane->AddPostProcess(&_mlaaPP, L"MLAA", true, true);
	_ppConfigPane->AddPostProcess(&_fxaaPP, L"FXAA", false, true);
	_ppConfigPane->AddPostProcess(&_hdrPP, L"HDR", true, true);
	_ppConfigPane->AddPostProcess(&_pdofPP, L"Poisson DoF", true, true);
	_ppConfigPane->AddPostProcess(&_uiPP, L"UI", true, false);
	_ppConfigPane->AddPostProcess(&_motionBlurPP, L"Motion blur", false, false);	

	new ProfilePane(_configWindow, Logger::GetInstance());
	new DeviceManagerConfigurationPane(_configWindow, GetDeviceManager());		
	new CameraConfigurationPane(_configWindow, &_camera);
	new HDRConfigurationPane(_configWindow, &_hdrPP);
	new MLAAConfigurationPane(_configWindow, &_mlaaPP);
	new FXAAConfigurationPane(_configWindow, &_fxaaPP);
	new SSAOConfigurationPane(_configWindow, &_ssaoPP);
	new SkyConfigurationPane(_configWindow, &_skyPP);
	new PoissonDoFConfigurationPane(_configWindow, &_pdofPP);

	// Create the log window
	_logWindow = new LogWindow(canvas, Logger::GetInstance());
}

void DeferredRendererApplication::OnPreparingDeviceSettings(DeviceManager* deviceManager)
{
	Application::OnPreparingDeviceSettings(deviceManager);

	deviceManager->SetBackBufferWidth(1360);
	deviceManager->SetBackBufferHeight(768);
	deviceManager->SetVSyncEnabled(false);
}

void DeferredRendererApplication::OnFrameMove(double totalTime, float dt)
{
	BEGIN_EVENT(L"Gather input");
	HWND hwnd = GetHWND();
	KeyboardState kb = KeyboardState::GetState();
	MouseState mouse = MouseState::GetState(hwnd);
	END_EVENT(L"");

	if (IsActive() && mouse.IsOverWindow())
	{
		BEGIN_EVENT(L"Process input");
		if (kb.IsKeyJustPressed(Keys::Esc))
		{
			Exit();
		}
		
		if (kb.IsKeyJustPressed(Keys::F11))
		{
			SetFullScreen(!GetFullScreen());
		}

		if (kb.IsKeyJustPressed(Keys::M))
		{
			SetMaximized(!GetMaximized());
		}
	
		if (kb.IsKeyJustPressed(Keys::U))
		{
			bool uiEnabled = _ppConfigPane->IsPostProcessEnabled(&_uiPP);
			_ppConfigPane->SetPostProcessEnabled(&_uiPP, !uiEnabled);
		}

		if (mouse.IsButtonDown(MouseButton::RightButton))
		{
			const float mouseRotateSpeed = _camera.GetRotationSpeed();
		
			XMFLOAT2 rotation = _camera.GetRotation();
			rotation.x += mouse.GetDX() * mouseRotateSpeed;
			rotation.y += mouse.GetDY() * mouseRotateSpeed;
			_camera.SetRotation(rotation);

			// Set the mouse back one frame
			//MouseState::SetCursorPosition(mouse.GetX() - mouse.GetDX(), mouse.GetY() - mouse.GetDY(), hwnd);

			XMFLOAT2 moveDir = XMFLOAT2(0.0f, 0.0f);
			if (kb.IsKeyDown(Keys::W) || kb.IsKeyDown(Keys::Up))
			{
				moveDir.y++;
			}
			if (kb.IsKeyDown(Keys::S) || kb.IsKeyDown(Keys::Down))
			{
				moveDir.y--;
			}
			if (kb.IsKeyDown(Keys::A) || kb.IsKeyDown(Keys::Left))
			{
				moveDir.x--;
			}
			if (kb.IsKeyDown(Keys::D) || kb.IsKeyDown(Keys::Right))
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
		END_EVENT(L"");
	}
	
	if (_ppConfigPane->IsPostProcessEnabled(&_uiPP))
	{
		BEGIN_EVENT(L"Update UI");

		_uiPP.OnFrameMove(totalTime, dt);
		_configWindow->OnFrameMove(totalTime, dt);

		END_EVENT(L"");
	}
}

LRESULT DeferredRendererApplication::OnMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HRESULT hr;
	
	if (_ppConfigPane->IsPostProcessEnabled(&_uiPP))
	{
		V_RETURN(_uiPP.OnMessage(hWnd, msg, wParam, lParam));
	}

	return Application::OnMessage(hWnd, msg, wParam, lParam);
}

HRESULT DeferredRendererApplication::OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext)
{
	HRESULT hr;

	BEGIN_EVENT(L"Prepare scene");

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

	if (_ppConfigPane->IsPostProcessEnabled(&_skyPP) && _skyPP.GetSunEnabled())
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

	UINT ppCount = _ppConfigPane->GetSelectedPostProcessCount();
	for (UINT i = 0; i < ppCount; i++)
	{
		_renderer.AddPostProcess(_ppConfigPane->GetSelectedPostProcesses(i));
	}

	END_EVENT(L"");

	BEGIN_EVENT(L"Render scene");
	V_RETURN(_renderer.End(pd3dImmediateContext, &_camera));
	END_EVENT(L"");

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
	V_RETURN(_fxaaPP.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_ssaoPP.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_pdofPP.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_motionBlurPP.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_uiPP.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));

	V_RETURN(_paraboloidPointLR.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_cascadedDirectionalLR.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_spotLR.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));

	V_RETURN(_scene.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));

	const int configWidth = 260;
	const int logHeight = 125;
	const int padding = 10;

	_configWindow->SetBounds(padding, padding, configWidth, pBackBufferSurfaceDesc->Height - (padding * 2));
	_logWindow->SetBounds(_configWindow->Right() + padding, pBackBufferSurfaceDesc->Height - logHeight - padding, 
		pBackBufferSurfaceDesc->Width - _configWindow->Right() - (padding * 2), logHeight);

	return S_OK;
}

void DeferredRendererApplication::OnD3D11DestroyDevice()
{
	Application::OnD3D11DestroyDevice();

	_renderer.OnD3D11DestroyDevice();	
	_hdrPP.OnD3D11DestroyDevice();
	_skyPP.OnD3D11DestroyDevice();
	_mlaaPP.OnD3D11DestroyDevice();
	_fxaaPP.OnD3D11DestroyDevice();
	_ssaoPP.OnD3D11DestroyDevice();
	_pdofPP.OnD3D11DestroyDevice();
	_motionBlurPP.OnD3D11DestroyDevice();
	_uiPP.OnD3D11DestroyDevice();

	_paraboloidPointLR.OnD3D11DestroyDevice();
	_cascadedDirectionalLR.OnD3D11DestroyDevice();
	_spotLR.OnD3D11DestroyDevice();

	_scene.OnD3D11DestroyDevice();	
}

HRESULT DeferredRendererApplication::OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice,
	IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	Gwen::Controls::Canvas* canvas = _uiPP.GetCanvas();
	XMFLOAT2 sizePerc = XMFLOAT2((float)pBackBufferSurfaceDesc->Width / canvas->Width(),
		(float)pBackBufferSurfaceDesc->Height / canvas->Height());
	
	V_RETURN(Application::OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));

	float fAspectRatio = pBackBufferSurfaceDesc->Width / (float)pBackBufferSurfaceDesc->Height;
	_camera.SetAspectRatio(fAspectRatio);
	
	V_RETURN(_renderer.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));	
	V_RETURN(_hdrPP.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_skyPP.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_mlaaPP.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_fxaaPP.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_ssaoPP.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_pdofPP.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_motionBlurPP.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_uiPP.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	
	V_RETURN(_paraboloidPointLR.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_cascadedDirectionalLR.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_spotLR.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	
	V_RETURN(_scene.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));

	// Resize the UI by scaling with the resolution change
	_configWindow->SetBounds(_configWindow->X() * sizePerc.x, _configWindow->Y() * sizePerc.y,
		_configWindow->Width() * sizePerc.x, _configWindow->Height() * sizePerc.y);
	_logWindow->SetBounds(_logWindow->X() * sizePerc.x, _logWindow->Y() * sizePerc.y,
		_logWindow->Width() * sizePerc.x, _logWindow->Height() * sizePerc.y);

	return S_OK;
}
void DeferredRendererApplication::OnD3D11ReleasingSwapChain()
{
	Application::OnD3D11ReleasingSwapChain();

	_renderer.OnD3D11ReleasingSwapChain();	
	_hdrPP.OnD3D11ReleasingSwapChain();
	_skyPP.OnD3D11ReleasingSwapChain();
	_mlaaPP.OnD3D11ReleasingSwapChain();
	_fxaaPP.OnD3D11ReleasingSwapChain();
	_ssaoPP.OnD3D11ReleasingSwapChain();
	_pdofPP.OnD3D11ReleasingSwapChain();
	_motionBlurPP.OnD3D11ReleasingSwapChain();
	_uiPP.OnD3D11ReleasingSwapChain();

	_paraboloidPointLR.OnD3D11ReleasingSwapChain();
	_cascadedDirectionalLR.OnD3D11ReleasingSwapChain();
	_spotLR.OnD3D11ReleasingSwapChain();

	_scene.OnD3D11ReleasingSwapChain();
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	DeferredRendererApplication app;
	app.Start();
}