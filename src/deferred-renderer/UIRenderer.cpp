#include "PCH.h"
#include "UIRenderer.h"
#include "FontLoader.h"
#include "Gwen/Utility.h"
#include "Gwen/Font.h"
#include "Gwen/Texture.h"
#include "TextureLoader.h"

UIRenderer::UIRenderer()
	: _uiFont(NULL), _graphicsDevice(NULL), _immediateContext(NULL), _swapChain(NULL)
{
}

UIRenderer::~UIRenderer()
{
}

void UIRenderer::SetImmediateContext(ID3D11DeviceContext* context)
{
	_immediateContext = context;
}

void UIRenderer::Begin()
{
	HRESULT hr;
	V(_spriteRenderer.Begin());
}

void UIRenderer::End()
{
	HRESULT hr;
	V(_spriteRenderer.End(_immediateContext));
}

void UIRenderer::Release()
{
}

void UIRenderer::SetDrawColor(Gwen::Color color)
{
	_drawColor.x = color.r / 255.0f;
	_drawColor.y = color.g / 255.0f;
	_drawColor.z = color.b / 255.0f;
	_drawColor.w = color.a / 255.0f;
}

void UIRenderer::DrawLine(int x, int y, int a, int b)
{
}

void UIRenderer::DrawFilledRect(Gwen::Rect rect)
{
	Translate(rect);

	SPRITE_DRAW_DATA spriteData;
	spriteData.TopLeft = XMFLOAT2(rect.x, rect.y);
	spriteData.Size = XMFLOAT2(rect.w, rect.h);
	spriteData.Color = _drawColor;

	_spriteRenderer.AddColoredRectangles(&spriteData, 1);
}

void UIRenderer::LoadFont(Gwen::Font* pFont)
{
	pFont->data = _uiFont;
}

void UIRenderer::FreeFont(Gwen::Font* pFont)
{
	pFont->data = NULL;
}

void UIRenderer::RenderText(Gwen::Font* pFont, Gwen::Point pos, const Gwen::UnicodeString& text)
{
	Translate(pos.x, pos.y);

	const WCHAR* string = text.c_str();
	XMFLOAT2 textSize = _uiFont->MeasureString(string);
	
	SPRITE_DRAW_DATA spriteData;
	spriteData.TopLeft = XMFLOAT2(pos.x, pos.y);
	spriteData.Size = XMFLOAT2(textSize.x, textSize.y);
	spriteData.Color = _drawColor;

	_spriteRenderer.AddTextScreenSpace(_uiFont, text.c_str(), spriteData);
}

Gwen::Point UIRenderer::MeasureText(Gwen::Font* pFont, const Gwen::UnicodeString& text)
{
	if (_uiFont)
	{
		XMFLOAT2 size = _uiFont->MeasureString(text.c_str());
		return Gwen::Point(size.x, size.y);
	}
	else
	{
		return Gwen::Point(0, 0);
	}
}

void UIRenderer::StartClip()
{
}

void UIRenderer::EndClip()
{
}

void UIRenderer::DrawTexturedRect(Gwen::Texture* pTexture, Gwen::Rect pTargetRect, float u1, float v1, float u2, float v2)
{
	Translate(pTargetRect);

	ID3D11ShaderResourceView* srv = (ID3D11ShaderResourceView*)pTexture->data;

	SPRITE_DRAW_DATA spriteData;
	spriteData.TopLeft = XMFLOAT2(pTargetRect.x, pTargetRect.y);
	spriteData.Size = XMFLOAT2(pTargetRect.w, pTargetRect.h);
	spriteData.TopLeftUV = XMFLOAT2(u1, v1);
	spriteData.SizeUV = XMFLOAT2(u2 - u1, v2 - v1);
	spriteData.Color = _drawColor;

	_spriteRenderer.AddTexturedRectangles(srv, &spriteData, 1);
}

void UIRenderer::LoadTexture(Gwen::Texture* pTexture)
{
	HRESULT hr;

	TextureContent* pContent;
	V(_contentManager->LoadContent(_graphicsDevice, pTexture->name.GetUnicode().c_str(), 
		(TextureOptions*)NULL, &pContent));

	pContent->ShaderResourceView->AddRef();

	pTexture->data = pContent->ShaderResourceView;
	pTexture->failed = false;
	pTexture->width = pContent->Info.Width;
	pTexture->height = pContent->Info.Height;

	SAFE_RELEASE(pContent);
}

void UIRenderer::FreeTexture(Gwen::Texture* pTexture)
{
	ID3D11ShaderResourceView* srv = (ID3D11ShaderResourceView*)pTexture->data;
	SAFE_RELEASE(srv);
}

HRESULT UIRenderer::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	_graphicsDevice = pd3dDevice;
	_contentManager = pContentManager;
	_backBufferSurfaceDesc = *pBackBufferSurfaceDesc;

	V_RETURN(pContentManager->LoadContent(pd3dDevice, L"UI\\consolas_14_font.xml", (FontOptions*)NULL, &_uiFont));

	V_RETURN(_spriteRenderer.OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));

	return S_OK;
}

void UIRenderer::OnD3D11DestroyDevice()
{
	_spriteRenderer.OnD3D11DestroyDevice();
	SAFE_RELEASE(_uiFont);
}

HRESULT UIRenderer::OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	_backBufferSurfaceDesc = *pBackBufferSurfaceDesc;

	_swapChain = pSwapChain;

	V_RETURN(_spriteRenderer.OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));

	return S_OK;
}

void UIRenderer::OnD3D11ReleasingSwapChain()
{
	_spriteRenderer.OnD3D11ReleasingSwapChain();
}