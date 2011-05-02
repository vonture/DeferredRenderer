#pragma once

#include "Light.h"
#include "Camera.h"
#include "PostProcess.h"
#include "ModelInstance.h"

class Renderer : public IHasContent
{
private:


public:
	Renderer(ID3D11Device* device);
	~Renderer();

	void AddModel(ModelInstance* model);
	void AddLight(Light* light, bool shadowed);
	void AddPostProcess(PostProcess* postProcess);
	void AddDebugText(WCHAR* text);

	void Begin(Camera* camera);
	void End();

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                     void* pUserContext);	
	void OnD3D11DestroyDevice(void* pUserContext);

	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
	void OnD3D11ReleasingSwapChain(void* pUserContext);
};