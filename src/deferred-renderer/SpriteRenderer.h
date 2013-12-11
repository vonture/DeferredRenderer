#pragma once

#include "PCH.h"
#include "IHasContent.h"
#include "DeviceStates.h"
#include "SpriteFont.h"
#include "PixelShaderLoader.h"
#include "VertexShaderLoader.h"

struct SPRITE_DRAW_DATA
{
    XMFLOAT2 TopLeft;
    XMFLOAT2 Size;
    XMFLOAT2 TopLeftUV;
    XMFLOAT2 SizeUV;
    XMFLOAT4 Color;
};

class SpriteRenderer : public IHasContent
{
private:
    DepthStencilStates _dsStates;
    SamplerStates _samplerStates;
    BlendStates _blendStates;
    RasterizerStates _rasterStates;

    UINT _bbWidth;
    UINT _bbHeight;

    static const float SPRITE_DEPTH;

    struct SPRITE_VERTEX
    {
        XMFLOAT4 Position;
        XMFLOAT2 TexCoord;
        XMFLOAT4 Color;
    };

    typedef WORD SpriteIndex;
    static const SpriteIndex MAX_SPRITES = 1 << 13;
    SpriteIndex* _indices;
    SPRITE_VERTEX* _vertices;
    SpriteIndex _nextSprite;

    struct TEXTURE_INDEX
    {
        SpriteIndex StartSprite;
        SpriteIndex SpriteCount;
        ID3D11ShaderResourceView* Texture;

        bool Scissor;
        D3D11_RECT ScissorRect;
    };
    TEXTURE_INDEX* _textures;
    int _curTexture;

    ID3D11Buffer* _indexBuffer;
    ID3D11Buffer* _vertexBuffer;

    VertexShaderContent* _spriteVS;
    PixelShaderContent* _spritePS;

    ID3D11ShaderResourceView* _blankSRV;

    bool _begun;

public:
    SpriteRenderer();
    ~SpriteRenderer();

    HRESULT Begin();
    HRESULT End(ID3D11DeviceContext* pd3d11DeviceContext);

    void AddTextScreenSpace(SpriteFont* font, const WCHAR* text, SPRITE_DRAW_DATA& drawData);

    void SetScissorRectangle(const D3D11_RECT& rect);
    void UnsetScissorRectangle();

    void AddTexturedRectangles(ID3D11ShaderResourceView* texture, SPRITE_DRAW_DATA* spriteData,
        UINT numSprites);

    void AddColoredRectangles(SPRITE_DRAW_DATA* spriteData, UINT numSprites);

    HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
    void OnD3D11DestroyDevice(ContentManager* pContentManager);

    HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
    void OnD3D11ReleasingSwapChain(ContentManager* pContentManager);
};