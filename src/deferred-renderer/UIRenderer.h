#pragma once

#include "PCH.h"
#include "Gwen\Gwen.h"
#include "DeviceStates.h"
#include "IHasContent.h"
#include "SpriteRenderer.h"
#include "SpriteFont.h"

class UIRenderer : public Gwen::Renderer::Base, public IHasContent
{
private:
    ContentManager* _contentManager;
    ID3D11Device* _graphicsDevice;
    ID3D11DeviceContext* _immediateContext;

    IDXGISwapChain* _swapChain;
    DXGI_SURFACE_DESC _backBufferSurfaceDesc;

    XMFLOAT4 _drawColor;

    SpriteFont* _uiFont;
    SpriteRenderer _spriteRenderer;

public:
    UIRenderer();
    ~UIRenderer();

    void SetImmediateContext(ID3D11DeviceContext* context);

    void Begin();
    void End();
    void Release();

    void SetDrawColor(Gwen::Color color);

    void DrawLine(int x, int y, int a, int b);
    void DrawFilledRect(Gwen::Rect rect);

    virtual void LoadFont(Gwen::Font* pFont);
    virtual void FreeFont(Gwen::Font* pFont);
    void RenderText(Gwen::Font* pFont, Gwen::Point pos, const Gwen::UnicodeString& text);
    Gwen::Point MeasureText(Gwen::Font* pFont, const Gwen::UnicodeString& text);

    void StartClip();
    void EndClip();

    void DrawTexturedRect(Gwen::Texture* pTexture, Gwen::Rect pTargetRect, float u1=0.0f, float v1=0.0f, float u2=1.0f, float v2=1.0f);
    void LoadTexture(Gwen::Texture* pTexture);
    void FreeTexture(Gwen::Texture* pTexture);

    HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
    void OnD3D11DestroyDevice(ContentManager* pContentManager);

    HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
    void OnD3D11ReleasingSwapChain(ContentManager* pContentManager);
};