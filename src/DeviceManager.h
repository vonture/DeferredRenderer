#pragma once

#include "PCH.h"

class DeviceManager
{
private:
	IDXGIFactory1* _factory;
    IDXGIAdapter1* _adapter;
    IDXGIOutput* _output;

	ID3D11Device* _device;
	ID3D11DeviceContext* _immediateContext;
	IDXGISwapChain*	_swapChain;

	ID3D11Texture2D* _backBufferTexture;
	ID3D11RenderTargetView* _backBufferRTV;

	DXGI_SURFACE_DESC _backBufferSurfaceDesc;
	DXGI_FORMAT _backBufferFormat;
	UINT _backBufferWidth;
	UINT _backBufferHeight;
	UINT _msCount;
	UINT _msQuality;

	bool _enableAutoDS;
	ID3D11Texture2D* _autoDSTexture;
    ID3D11DepthStencilView* _autoDSView;
	DXGI_FORMAT _autoDSFormat;
    ID3D11ShaderResourceView* _autoDSSRView;

    bool _useAutoDSAsSR;
	bool _fullScreen;
	bool _vsync;
    DXGI_RATIONAL _refreshRate;

	D3D_FEATURE_LEVEL _featureLevel;
    D3D_FEATURE_LEVEL _minFeatureLevel;

	HRESULT checkForSuitableOutput();
    HRESULT afterReset();

public:
	DeviceManager();

	HRESULT Initialize(HWND outputWindow);
	HRESULT Reset();
	HRESULT Present();

	void Destroy();

	ID3D11Device* GetDevice() const { return _device; }
	ID3D11DeviceContext* GetImmediateContext() const { return _immediateContext; }
	IDXGISwapChain*	GetSwapChain() const { return _swapChain; }
	ID3D11RenderTargetView* GetBackBuffer() const { return _backBufferRTV; }
	ID3D11DepthStencilView* GetAutoDepthStencilView() const { return _autoDSView; }
    ID3D11ShaderResourceView* GetAutoDepthStencilSRView() const { return _autoDSSRView; }
	D3D_FEATURE_LEVEL GetFeatureLevel() const { return _featureLevel; }
    D3D_FEATURE_LEVEL GetMinFeatureLevel() const { return _minFeatureLevel; }
	const DXGI_SURFACE_DESC* GetBackBufferSurfaceDesc() const { return &_backBufferSurfaceDesc; }
	DXGI_FORMAT GetBackBufferFormat() const	{ return _backBufferFormat; }
	UINT GetBackBufferWidth() const	{ return _backBufferWidth; }
	UINT GetBackBufferHeight() const	{ return _backBufferHeight; }
	UINT GetBackBufferMSCount() const { return _msCount; }
	UINT GetBackBufferMSQuality() const { return _msQuality; }
	bool GetAutoDepthStencilEnabled() const { return _enableAutoDS; }
	DXGI_FORMAT	GetAutoDepthStencilFormat() const { return _autoDSFormat; }
    bool GetUseAutoDepthStencilAsSR() const { return _useAutoDSAsSR; }
	bool GetFullScreen() const { return _fullScreen; }
	bool GetVSyncEnabled() const { return _vsync; }

    void SetBackBufferFormat(DXGI_FORMAT format) { _backBufferFormat = format; }
    void SetBackBufferWidth(UINT width) { _backBufferWidth = width; }
    void SetBackBufferHeight(UINT height) { _backBufferHeight = height; }
    void SetBackBufferMSCount(UINT count) { _msCount = count; }
    void SetBackBufferMSQuality(UINT quality) { _msQuality = quality; }
    void SetAutoDepthStencilEnabled(bool enabled) { _enableAutoDS = enabled; }
    void SetAutoDepthStencilFormat(DXGI_FORMAT fmt)	{ _autoDSFormat = fmt; }
    void SetUseAutoDSAsSR(bool useAsSR) { _useAutoDSAsSR = useAsSR; }
    void SetFullScreen(bool enabled) { _fullScreen = enabled; }
    void SetVSyncEnabled(bool enabled) { _vsync = enabled; }
    void SetMinFeatureLevel(D3D_FEATURE_LEVEL level) { _minFeatureLevel = level; }
};