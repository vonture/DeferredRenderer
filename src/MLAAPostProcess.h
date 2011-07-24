#pragma once

#include "Defines.h"
#include "PostProcess.h"

class MLAAPostProcess : public PostProcess
{
private:
	float _depthThreshold;
	float _normalThreshold;
	float _luminanceThreshold;

	bool _depthDetect;
	bool _normalDetect;
	bool _luminanceDetect;

	UINT _maxSearchSteps;

	UINT _textureWidth;
	UINT _textureHeight;

	ID3D11Texture2D* _dsTexture;
	ID3D11DepthStencilView* _dsv;

	ID3D11Texture2D* _edgeDetectTexture;
	ID3D11RenderTargetView* _edgeDetectRTV;
	ID3D11ShaderResourceView* _edgeDetectSRV;
	
	ID3D11Texture2D* _blendWeightTexture;
	ID3D11RenderTargetView* _blendWeightRTV;
	ID3D11ShaderResourceView* _blendWeightSRV;

	static const UINT NUM_WEIGHT_TEXTURES = 5;
	static const UINT WEIGHT_TEXTURE_SIZES[NUM_WEIGHT_TEXTURES];

	static const WCHAR* WEIGHT_TEXTURE_PATH;
	ID3D11ShaderResourceView* _weightSRVs[NUM_WEIGHT_TEXTURES];
		
	ID3D11PixelShader* _edgeDetectPSs[2][2][2];
	ID3D11PixelShader* _blendWeightPSs[NUM_WEIGHT_TEXTURES];
	ID3D11PixelShader* _copyBackgroundPS;
	ID3D11PixelShader* _neighborhoodBlendPS;

	ID3D11Buffer* _mlaaPropertiesBuffer;

	struct CB_MLAA_PROPERTIES
	{
		XMFLOAT2 InverseSceneSize;
		float DepthThreshold;
		float NormalThreshold;
		float LuminanceThreshold;
		float CameraNearClip;
		float CameraFarClip;
		int MaxSearchSteps;
	};

public:
	MLAAPostProcess();
	~MLAAPostProcess();
	
	float GetDepthThreshold() const { return _depthThreshold; }
	void SetDepthThreshold(float threshold) { _depthThreshold = max(threshold, 0.0f); } 

	float GetNormalThreshold() const { return _normalThreshold; }
	void SetNormalThreshold(float threshold) { _normalThreshold = max(threshold, 0.0f); } 

	float GetLuminanceThreshold() const { return _luminanceThreshold; }
	void SetLuminanceThreshold(float threshold) { _luminanceThreshold = max(threshold, 0.0f); } 

	bool GetDepthDetectionEnabled() const { return _depthDetect; }
	void SetDepthDetectionEnabled(bool enabled) { _depthDetect = enabled; }

	bool GetNormalDetectionEnabled() const { return _normalDetect; }
	void SetNormalDetectionEnabled(bool enabled) { _normalDetect = enabled; }

	bool GetLuminanceDetectionEnabled() const { return _luminanceDetect; }
	void SetLuminanceDetectionEnabled(bool enabled) { _luminanceDetect = enabled; }

	UINT GetMaxSearchSteps() const { return _maxSearchSteps; }
	void SetMaxSearchSteps(UINT steps) { _maxSearchSteps = clamp(steps, (WEIGHT_TEXTURE_SIZES[0] - 1) / 2,
		(WEIGHT_TEXTURE_SIZES[NUM_WEIGHT_TEXTURES - 1] - 1) / 2); }

	HRESULT Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
		ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer, LightBuffer* lightBuffer);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};