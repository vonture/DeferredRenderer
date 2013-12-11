#include "PCH.h"
#include "DeferredRendererApplication.h"

#include "HDRConfigurationPane.h"
#include "MLAAConfigurationPane.h"
#include "FXAAConfigurationPane.h"
#include "SkyConfigurationPane.h"
#include "CameraConfigurationPane.h"
#include "SSAOConfigurationPane.h"
#include "HBAOConfigurationPane.h"
#include "DiscDoFMBConfigurationPane.h"
#include "MotionBlurConfigurationPane.h"
#include "DeviceManagerConfigurationPane.h"
#include "ProfilePane.h"

DeferredRendererApplication::DeferredRendererApplication()
    : Application(L"Deferred Renderer", NULL), _camera(0.1f, 45.0f, 1.0f, 1.0f), _selectedItem(NULL),
    _configWindow(NULL), _logWindow(NULL), _ppConfigPane(NULL)
{
    ModelInstance* tankScene = new ModelInstance(L"\\models\\tankscene\\TankScene.sdkmesh");
    tankScene->SetScale(1.0f);
    tankScene->SetPosition(XMFLOAT3(0, 0, 0));

    _models.push_back(tankScene);

    ModelInstance* squid = new ModelInstance(L"\\models\\Squid\\Squid.sdkmesh");
    squid->SetScale(0.05f);
    squid->SetPosition(XMFLOAT3(10.0f, 0.0f, 0.0f));
    _models.push_back(squid);

    ModelInstance* tree = new ModelInstance(L"\\models\\tree\\tree.obj");
    tree->SetScale(0.5f);
    tree->SetPosition(XMFLOAT3(0.0f, 2.0f, 0.0f));

    XMFLOAT4 orientation;
    XMStoreFloat4(&orientation, XMQuaternionRotationRollPitchYaw(0.0f, RandomBetween(-Pi, Pi), 0.0f));
    tree->SetOrientation(orientation);

    _models.push_back(tree);

    ParticleSystemInstance* smoke = new ParticleSystemInstance(L"\\particles\\smoke.xml");
    smoke->SetPosition(XMFLOAT3(-20, 0.0f, -20));
    smoke->SetScale(1.0f);
    _particles.push_back(smoke);

    _pointLightsShadowed.push_back(new PointLight(XMFLOAT3(3.0f, 4.0f, 0.0f), 10.0f, XMFLOAT3(1.0f, 1.0f, 1.0f), 1.0f));

    _camera.SetPosition(XMFLOAT3(1.0f, 4.0f, -6.0f));
    _camera.SetRotation(XMFLOAT2(-0.1f, 0.35f), 0.0f);

    _contentHolders.push_back(&_renderer);
    _contentHolders.push_back(&_pBufferCombinePP);
    _contentHolders.push_back(&_hdrPP);
    _contentHolders.push_back(&_skyPP);
    _contentHolders.push_back(&_mlaaPP);
    _contentHolders.push_back(&_fxaaPP);
    _contentHolders.push_back(&_ssaoPP);
    _contentHolders.push_back(&_hbaoPP);
    _contentHolders.push_back(&_discDoFPP);
    _contentHolders.push_back(&_motionBlurPP);
    _contentHolders.push_back(&_filmGrainPP);
    _contentHolders.push_back(&_boPP);
    _contentHolders.push_back(&_uiPP);
    _contentHolders.push_back(&_paraboloidPointLR);
    _contentHolders.push_back(&_cascadedDirectionalLR);
    _contentHolders.push_back(&_spotLR);

    for (UINT i = 0; i < _models.size(); i++)
    {
        _contentHolders.push_back(_models[i]);
        _dragables.push_back(_models[i]);
    }
    for (UINT i = 0; i < _particles.size(); i++)
    {
        _contentHolders.push_back(_particles[i]);
        _dragables.push_back(_particles[i]);
    }

    for (UINT i = 0; i < _pointLightsShadowed.size(); i++)
    {
        _dragables.push_back(_pointLightsShadowed[i]);
    }
    for (UINT i = 0; i < _pointLightsUnshadowed.size(); i++)
    {
        _dragables.push_back(_pointLightsUnshadowed[i]);
    }
}

DeferredRendererApplication::~DeferredRendererApplication()
{
    for (UINT i = 0; i < _models.size(); i++)
    {
        delete _models[i];
    }
    for (UINT i = 0; i < _particles.size(); i++)
    {
        delete _particles[i];
    }
    for (UINT i = 0; i < _pointLightsShadowed.size(); i++)
    {
        delete _pointLightsShadowed[i];
    }
    for (UINT i = 0; i < _pointLightsUnshadowed.size(); i++)
    {
        delete _pointLightsUnshadowed[i];
    }
    for (UINT i = 0; i < _dirLightsShadowed.size(); i++)
    {
        delete _dirLightsShadowed[i];
    }
    for (UINT i = 0; i < _dirLightsUnshadowed.size(); i++)
    {
        delete _dirLightsUnshadowed[i];
    }
}

void DeferredRendererApplication::OnInitialize()
{
    // Set some properties of the renderer
    _renderer.AddLightRenderer(&_paraboloidPointLR);
    _renderer.AddLightRenderer(&_cascadedDirectionalLR);
    _renderer.AddLightRenderer(&_spotLR);

    // Create all the UI elements
    Gwen::Controls::Canvas* canvas = _uiPP.GetCanvas();

    // Create the configuration window and its panes
    _configWindow = new ConfigurationWindow(canvas);

    _ppConfigPane = new PostProcessSelectionPane(_configWindow);
    _ppConfigPane->AddPostProcess(&_ssaoPP, L"SSAO", true, true);
    _ppConfigPane->AddPostProcess(&_hbaoPP, L"HBAO", false, false);
    _ppConfigPane->AddPostProcess(&_skyPP, L"Sky", true, true);
    _ppConfigPane->AddPostProcess(&_mlaaPP, L"MLAA", false, true);
    _ppConfigPane->AddPostProcess(&_pBufferCombinePP, L"Particle Combine", true, true);
    _ppConfigPane->AddPostProcess(&_hdrPP, L"HDR", true, true);
    _ppConfigPane->AddPostProcess(&_discDoFPP, L"Disc DoF", false, true);
    _ppConfigPane->AddPostProcess(&_fxaaPP, L"FXAA", true, true);
    _ppConfigPane->AddPostProcess(&_filmGrainPP, L"Film grain/vignette", true, true);
    _ppConfigPane->AddPostProcess(&_boPP, L"Bounding objects", true, true);
    _ppConfigPane->AddPostProcess(&_uiPP, L"UI", true, false);
    _ppConfigPane->AddPostProcess(&_motionBlurPP, L"Motion blur", false, false);

    _modelConfigPane = new ModelConfigurationPane(_configWindow);
    _particleConfigPane = new ParticleConfigurationPane(_configWindow);
    _boConfigPane = new BoundingObjectConfigurationPane(_configWindow, &_boPP);

    new ProfilePane(_configWindow, Logger::GetInstance());
    new DeviceManagerConfigurationPane(_configWindow, GetDeviceManager());
    new CameraConfigurationPane(_configWindow, &_camera);
    new HDRConfigurationPane(_configWindow, &_hdrPP);
    new MLAAConfigurationPane(_configWindow, &_mlaaPP);
    new FXAAConfigurationPane(_configWindow, &_fxaaPP);
    new SSAOConfigurationPane(_configWindow, &_ssaoPP);
    new HBAOConfigurationPane(_configWindow, &_hbaoPP);
    new SkyConfigurationPane(_configWindow, &_skyPP);
    new DiscDoFMBConfigurationPane(_configWindow, &_discDoFPP);
    new MotionBlurConfigurationPane(_configWindow, &_motionBlurPP);

    // Create the log window
    _logWindow = new LogWindow(canvas, Logger::GetInstance());
}

void DeferredRendererApplication::OnPreparingContentManager(ContentManager* contentManager)
{
    contentManager->AddContentSearchPath(L"\\..\\..\\media");
    contentManager->AddContentSearchPath(L"\\..\\deferred-renderer");
    contentManager->SetCompiledContentPath(L"\\..\\..\\compiledmedia");

    contentManager->AddContentLoader(&_textureLoader);
    contentManager->AddContentLoader(&_psLoader);
    contentManager->AddContentLoader(&_gsLoader);
    contentManager->AddContentLoader(&_vsLoader);
    contentManager->AddContentLoader(&_modelLoader);
    contentManager->AddContentLoader(&_particleLoader);
    contentManager->AddContentLoader(&_fontLoader);
}

void DeferredRendererApplication::OnPreparingDeviceSettings(DeviceManager* deviceManager)
{
    Application::OnPreparingDeviceSettings(deviceManager);

    deviceManager->SetAutoDepthStencilEnabled(false);
    deviceManager->SetBackBufferWidth(1920);
    deviceManager->SetBackBufferHeight(1080);
    deviceManager->SetVSyncEnabled(false);
}

void DeferredRendererApplication::OnFrameMove(double totalTime, float dt)
{
    BEGIN_EVENT(L"Gather input");
    HWND hwnd = GetHWND();
    KeyboardState kb = KeyboardState::GetState();
    MouseState mouse = MouseState::GetState(hwnd);
    END_EVENT(L"");

    _camera.StoreMatrices();

    BEGIN_EVENT(L"Update Models");
    for (UINT i = 0; i < _models.size(); i++)
    {
        _models[i]->StoreWorld();
    }
    END_EVENT(L"");

    BEGIN_EVENT(L"Update Particles");
    XMFLOAT3 wind = _particleConfigPane->GetWindVector();
    XMFLOAT3 grav = _particleConfigPane->GetGravityVector();
    for (UINT i = 0; i < _particleConfigPane->GetParticleInstanceCount(); i++)
    {
        _particleConfigPane->GetParticleInstance(i)->AdvanceSystem(wind, grav, dt);
    }
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

            float rollSpeed = _camera.GetRollSpeed();
            float roll = _camera.GetRoll();
            if (kb.IsKeyDown(Keys::Q))
            {
                roll += rollSpeed * dt;
            }
            if (kb.IsKeyDown(Keys::E))
            {
                roll -= rollSpeed * dt;
            }

            _camera.SetRotation(rotation, roll);

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

        if (mouse.IsButtonJustPressed(MouseButton::LeftButton))
        {
            XMFLOAT2 mousePos = XMFLOAT2(mouse.GetX(), mouse.GetY());
            XMFLOAT2 viewSize = XMFLOAT2(GetWidth(), GetHeight());

            Ray mouseRay = _camera.UnprojectRay(mousePos, viewSize);

            _selectedDepth = FLT_MAX;
            _selectedItem = NULL;

            for (UINT i = 0; i < _dragables.size(); i++)
            {
                float dist;
                if (_dragables[i]->RayIntersect(mouseRay, &dist) && dist < _selectedDepth)
                {
                    _selectedDepth = dist;
                    _selectedItem = _dragables[i];
                }
            }
        }

        if (_selectedItem && mouse.IsButtonDown(MouseButton::LeftButton))
        {
            if (kb.IsKeyDown(Keys::LeftControl) || kb.IsKeyDown(Keys::RightControl))
            {
            }
            else
            {
                XMFLOAT3 curModelPos = _selectedItem->GetPosition();

                XMFLOAT3 camUp = _camera.GetUp();
                XMFLOAT3 camRight = _camera.GetRight();
                XMFLOAT3 camForward = _camera.GetForward();

                XMVECTOR pos = XMLoadFloat3(&curModelPos);
                XMVECTOR up = XMLoadFloat3(&camUp);
                XMVECTOR right = XMLoadFloat3(&camRight);
                XMVECTOR forward = XMLoadFloat3(&camForward);

                pos += mouse.GetDX() * right * 0.01f;
                if (kb.IsKeyDown(Keys::LeftAlt) || kb.IsKeyDown(Keys::RightAlt))
                {
                    pos += mouse.GetDY() * up * -0.01f;
                }
                else
                {
                    pos += mouse.GetDY() * forward * -0.01f;
                }

                XMStoreFloat3(&curModelPos, pos);
                _selectedItem->SetPosition(curModelPos);
            }
        }

        END_EVENT(L"");
    }

    if (_ppConfigPane->IsPostProcessEnabled(&_filmGrainPP))
    {
        _filmGrainPP.SetTime(totalTime);
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

    BoundingObjectSet boSet;
    if (_selectedItem)
    {
        _selectedItem->FillBoundingObjectSet(&boSet);
    }

    for (UINT i = 0; i < _modelConfigPane->GetModelInstanceCount(); i++)
    {
        ModelInstance* model = _modelConfigPane->GetModelInstance(i);

        _renderer.AddModel(model);
        if (_boConfigPane->GetModelsEnabled())
        {
            model->FillBoundingObjectSet(&boSet);
        }
    }

    for (UINT i = 0; i < _particleConfigPane->GetParticleInstanceCount(); i++)
    {
        ParticleSystemInstance* particle = _particleConfigPane->GetParticleInstance(i);

        _renderer.AddParticleSystem(particle);
        if (_boConfigPane->GetParticlesEnabled())
        {
            particle->FillBoundingObjectSet(&boSet);
        }
    }

    for (UINT i = 0; i < _pointLightsShadowed.size(); i++)
    {
        _renderer.AddLight(_pointLightsShadowed[i], true);

        if (_boConfigPane->GetLightsEnabled())
        {
            _pointLightsShadowed[i]->FillBoundingObjectSet(&boSet);
        }
    }
    for (UINT i = 0; i < _pointLightsUnshadowed.size(); i++)
    {
        _renderer.AddLight(_pointLightsUnshadowed[i], false);

        if (_boConfigPane->GetLightsEnabled())
        {
            _pointLightsUnshadowed[i]->FillBoundingObjectSet(&boSet);
        }
    }

    for (UINT i = 0; i < _dirLightsShadowed.size(); i++)
    {
        _renderer.AddLight(_dirLightsShadowed[i], true);
    }
    for (UINT i = 0; i < _dirLightsUnshadowed.size(); i++)
    {
        _renderer.AddLight(_dirLightsUnshadowed[i], false);
    }

    _boPP.Clear();
    _boPP.Add(&boSet);

    if (_ppConfigPane->IsPostProcessEnabled(&_skyPP) && _skyPP.GetSunEnabled())
    {
        DirectionalLight sun = DirectionalLight(_skyPP.GetSunDirection(), _skyPP.GetSunColor(),
            _skyPP.GetSunBrightness());

        _renderer.AddLight(&sun, true);
    }

    AmbientLight ambientLight = AmbientLight(XMFLOAT3(1.0f, 1.0f, 1.0f), 0.4f);
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

HRESULT DeferredRendererApplication::OnD3D11CreateDevice(ID3D11Device* pd3dDevice,
                                                         ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
    HRESULT hr;

    V_RETURN(Application::OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));
    for (UINT i = 0; i < _contentHolders.size(); i++)
    {
        V_RETURN(_contentHolders[i]->OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));
    }

    const int configWidth = 260;
    const int logHeight = 125;
    const int padding = 10;

    _configWindow->SetBounds(padding, padding, configWidth, pBackBufferSurfaceDesc->Height - (padding * 2));
    _logWindow->SetBounds(_configWindow->Right() + padding, pBackBufferSurfaceDesc->Height - logHeight - padding,
        pBackBufferSurfaceDesc->Width - _configWindow->Right() - (padding * 2), logHeight);

    for (UINT i = 0; i < _models.size(); i++)
    {
        _modelConfigPane->AddModelInstance(_models[i]);
    }
    for (UINT i = 0; i < _particles.size(); i++)
    {
        _particleConfigPane->AddParticleInstance(_particles[i]);
    }

    return S_OK;
}

void DeferredRendererApplication::OnD3D11DestroyDevice(ContentManager* pContentManager)
{
    Application::OnD3D11DestroyDevice(pContentManager);
    for (UINT i = 0; i < _contentHolders.size(); i++)
    {
        _contentHolders[i]->OnD3D11DestroyDevice(pContentManager);
    }
}

HRESULT DeferredRendererApplication::OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager,
                                                             IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
    HRESULT hr;

    float fAspectRatio = pBackBufferSurfaceDesc->Width / (float)pBackBufferSurfaceDesc->Height;
    _camera.SetAspectRatio(fAspectRatio);

    Gwen::Controls::Canvas* canvas = _uiPP.GetCanvas();
    XMFLOAT2 sizePerc = XMFLOAT2((float)pBackBufferSurfaceDesc->Width / canvas->Width(),
        (float)pBackBufferSurfaceDesc->Height / canvas->Height());

    V_RETURN(Application::OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));
    for (UINT i = 0; i < _contentHolders.size(); i++)
    {
        V_RETURN(_contentHolders[i]->OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));
    }

    // Resize the UI by scaling with the resolution change
    _configWindow->SetBounds(_configWindow->X() * sizePerc.x, _configWindow->Y() * sizePerc.y,
        _configWindow->Width() * sizePerc.x, _configWindow->Height() * sizePerc.y);
    _logWindow->SetBounds(_logWindow->X() * sizePerc.x, _logWindow->Y() * sizePerc.y,
        _logWindow->Width() * sizePerc.x, _logWindow->Height() * sizePerc.y);

    return S_OK;
}
void DeferredRendererApplication::OnD3D11ReleasingSwapChain(ContentManager* pContentManager)
{
    Application::OnD3D11ReleasingSwapChain(pContentManager);
    for (UINT i = 0; i < _contentHolders.size(); i++)
    {
        _contentHolders[i]->OnD3D11ReleasingSwapChain(pContentManager);
    }
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    DeferredRendererApplication app;
    app.Start();
}