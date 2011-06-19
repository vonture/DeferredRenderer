#include "DeviceManager.h"

DeviceManager::DeviceManager()
	: _backBufferFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB), _backBufferWidth(1280),
	  _backBufferHeight(720), _msCount(1), _msQuality(0), _enableAutoDS(true), _fullScreen(false),
	  _featureLevel(D3D_FEATURE_LEVEL_11_0), _minFeatureLevel(D3D_FEATURE_LEVEL_10_0), 
	  _autoDSFormat(DXGI_FORMAT_D24_UNORM_S8_UINT), _useAutoDSAsSR(false), _vsync(true),
	  _device(NULL), _immediateContext(NULL), _swapChain(NULL), _backBufferTexture(NULL), _backBufferRTV(NULL),
	  _autoDSTexture(NULL), _autoDSView(NULL), _autoDSSRView(NULL), _factory(NULL), _adapter(NULL),
	  _output(NULL)
{
    _refreshRate.Numerator = 60;
    _refreshRate.Denominator = 1;
}

DeviceManager::~DeviceManager()
{
	if (_immediateContext)
    {
		_immediateContext->ClearState();
        _immediateContext->Flush();
    }
}

HRESULT DeviceManager::Initialize(HWND outputWindow)
{
	HRESULT hr;

	V_RETURN(checkForSuitableOutput());

	DXGI_SWAP_CHAIN_DESC desc;
	ZeroMemory(&desc, sizeof(DXGI_SWAP_CHAIN_DESC));

    if(_fullScreen)
	{
        V_RETURN(prepareFullScreenSettings());
	}
    else
    {
        _refreshRate.Numerator = 60;
        _refreshRate.Denominator = 1;
    }

	desc.BufferCount = 2;
	desc.BufferDesc.Format = _backBufferFormat;
	desc.BufferDesc.Width = _backBufferWidth;
	desc.BufferDesc.Height = _backBufferHeight;
	desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	desc.BufferDesc.RefreshRate = _refreshRate;
	desc.SampleDesc.Count = _msCount;
	desc.SampleDesc.Quality = _msQuality;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	desc.OutputWindow = outputWindow;
	desc.Windowed = !_fullScreen;

	UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED;
#ifdef _DEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	V_RETURN(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags,
		NULL, 0, D3D11_SDK_VERSION, &desc, &_swapChain, &_device, NULL, &_immediateContext));

    _featureLevel = _device->GetFeatureLevel();
    if (_featureLevel < _minFeatureLevel)
    {
       return E_FAIL;
    }

    V_RETURN(afterReset());

	return S_OK;
}

HRESULT DeviceManager::Reset()
{
	HRESULT hr;

	if (!_swapChain)
	{
		return E_FAIL;
	}

	SAFE_RELEASE(_backBufferTexture);
	SAFE_RELEASE(_backBufferRTV);

	SAFE_RELEASE(_autoDSTexture);
	SAFE_RELEASE(_autoDSView);
	SAFE_RELEASE(_autoDSSRView);

	_immediateContext->ClearState();

	if(_fullScreen)
	{
        V_RETURN(prepareFullScreenSettings());
	}
    else
    {
        _refreshRate.Numerator = 60;
        _refreshRate.Denominator = 1;
    }

	V_RETURN(_swapChain->SetFullscreenState(_fullScreen, NULL));

    V_RETURN(_swapChain->ResizeBuffers(2, _backBufferWidth, _backBufferHeight, _backBufferFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

    if(_fullScreen)
    {
        DXGI_MODE_DESC mode;
        mode.Format = _backBufferFormat;
        mode.Width = _backBufferWidth;
        mode.Height = _backBufferHeight;
        mode.RefreshRate.Numerator = 0;
        mode.RefreshRate.Denominator = 0;
        mode.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        mode.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        V_RETURN(_swapChain->ResizeTarget(&mode));
    }

    V_RETURN(afterReset());

	return S_OK;
}

HRESULT DeviceManager::Present()
{
	HRESULT hr;

	if (!_swapChain)
	{
		return E_FAIL;
	}

	V_RETURN(_swapChain->Present(_vsync ? 1 : 0, 0));

	return S_OK;
}

HRESULT DeviceManager::checkForSuitableOutput()
{
	HRESULT hr;
	
	V_RETURN(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&_factory));

    // Look for an adapter that supports D3D11
    IDXGIAdapter1* curAdapter = NULL;
    UINT adapterIdx = 0;
    while(!_adapter && SUCCEEDED(_factory->EnumAdapters1(adapterIdx, &_adapter)))
	{
        if(SUCCEEDED(_adapter->CheckInterfaceSupport(__uuidof(ID3D11Device), NULL)))
		{
            _adapter = curAdapter;
		}
		adapterIdx++;
	}

    if (!_adapter)
	{
		V_RETURN(E_FAIL);
	}

    // use the first output
    V_RETURN(_adapter->EnumOutputs(0, &_output));

	return S_OK;
}

HRESULT DeviceManager::afterReset()
{
	HRESULT hr;

	V_RETURN(_swapChain->GetBuffer(0, __uuidof(_backBufferTexture), (void**)(&_backBufferTexture)));
    V_RETURN(_device->CreateRenderTargetView(_backBufferTexture, NULL, &_backBufferRTV));

    // Create a default DepthStencil buffer
    if(_enableAutoDS)
    {
        UINT bindFlags = D3D11_BIND_DEPTH_STENCIL;
        if (_useAutoDSAsSR)
		{
            bindFlags |= D3D11_BIND_SHADER_RESOURCE;
		}

        DXGI_FORMAT dsTexFormat = DXGI_FORMAT_R32_TYPELESS;
        if (!_useAutoDSAsSR)
		{
            dsTexFormat = _autoDSFormat;
		}
        else
		{
			switch (_autoDSFormat)
			{
				case DXGI_FORMAT_D16_UNORM:
					 dsTexFormat = DXGI_FORMAT_R16_TYPELESS;
					 break;
				case DXGI_FORMAT_D24_UNORM_S8_UINT:
					dsTexFormat = DXGI_FORMAT_R24G8_TYPELESS;
					break;
			}
		}

        D3D11_TEXTURE2D_DESC dsDesc = 
		{
			_backBufferWidth,//UINT Width;
			_backBufferHeight,//UINT Height;
			1,//UINT MipLevels;
			1,//UINT ArraySize;
			dsTexFormat,//DXGI_FORMAT Format;
			0,//DXGI_SAMPLE_DESC SampleDesc;
			0,
			D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
			bindFlags,//UINT BindFlags;
			0,//UINT CPUAccessFlags;
			0//UINT MiscFlags;    
		};
        dsDesc.SampleDesc.Count = _msCount;
        dsDesc.SampleDesc.Quality = _msQuality;

        V_RETURN(_device->CreateTexture2D(&dsDesc, NULL, &_autoDSTexture));

        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
        dsvDesc.Format = _autoDSFormat;
        dsvDesc.ViewDimension = (_msCount > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0;
        dsvDesc.Flags = 0;

        V_RETURN(_device->CreateDepthStencilView(_autoDSTexture, &dsvDesc, &_autoDSView));

        if (_useAutoDSAsSR)
        {
            DXGI_FORMAT dsSRVFormat = DXGI_FORMAT_R32_FLOAT;
			switch (dsSRVFormat)
			{
				case DXGI_FORMAT_D16_UNORM:
					 dsSRVFormat = DXGI_FORMAT_R16_UNORM;
					 break;
				case DXGI_FORMAT_D24_UNORM_S8_UINT:
					dsSRVFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
					break;
			}

            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
            srvDesc.Format = dsSRVFormat;
            srvDesc.ViewDimension = (_msCount > 1) ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;
            srvDesc.Texture2D.MostDetailedMip = 0;
            V_RETURN(_device->CreateShaderResourceView(_autoDSTexture, &srvDesc, &_autoDSSRView));
        }
        else
		{
            _autoDSSRView = NULL;
		}
    }

    // Set default render targets
    _immediateContext->OMSetRenderTargets(1, &_backBufferRTV, _autoDSView);

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (float)_backBufferWidth;
    vp.Height = (float)_backBufferHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    _immediateContext->RSSetViewports(1, &vp);

	// Save everything to the backbuffer surface desc structure
	_backBufferSurfaceDesc.Width = _backBufferWidth;
	_backBufferSurfaceDesc.Height = _backBufferHeight;
	_backBufferSurfaceDesc.Format = _backBufferFormat;
	_backBufferSurfaceDesc.SampleDesc.Count = _msCount;
	_backBufferSurfaceDesc.SampleDesc.Quality = _msQuality;

	return S_OK;
}

HRESULT DeviceManager::prepareFullScreenSettings()
{
	HRESULT hr;
	
	if (!_output)
	{
		return E_FAIL;
	}

    // Have the Output look for the closest matching mode
    DXGI_MODE_DESC desiredMode;
    desiredMode.Format = _backBufferFormat;
    desiredMode.Width = _backBufferWidth;
    desiredMode.Height = _backBufferHeight;
    desiredMode.RefreshRate.Numerator = 0;
    desiredMode.RefreshRate.Denominator = 0;
    desiredMode.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    desiredMode.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

    DXGI_MODE_DESC closestMatch;
    V_RETURN(_output->FindClosestMatchingMode(&desiredMode, &closestMatch, _device));

    _backBufferFormat = closestMatch.Format;
    _backBufferWidth = closestMatch.Width;
    _backBufferHeight = closestMatch.Height;
    _refreshRate = closestMatch.RefreshRate;

	return S_OK;
}