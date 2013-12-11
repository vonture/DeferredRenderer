#include "PCH.h"
#include "SpriteFont.h"
#include "tinyxml.h"

SpriteFont::SpriteFont()
    : _textureWidth(1), _textureHeight(1), _fontSRV(NULL)
{
}

SpriteFont::~SpriteFont()
{
    Destroy();
}

bool SpriteFont::ContainsCharacter(WCHAR character)
{
    return _charMap.find(character) != _charMap.end();
}

XMFLOAT2 SpriteFont::MeasureString(const WCHAR* text)
{
    UINT numChars = (int)wcslen(text);

    XMFLOAT2 size = XMFLOAT2(0.0f, (float)_lineSpacing);
    float rowLen = 0.0f;
    for (UINT i = 0; i < numChars; i++)
    {
        if(text[i] == '\n')
        {
            size.y += _lineSpacing;
            rowLen = 0.0f;
        }
        else if (!ContainsCharacter(text[i]))
        {
            continue;
        }

        rowLen += _charMap[text[i]].PixelWidth;
        size.x = max(size.x, rowLen);
    }

    return size;
}

bool SpriteFont::GetCharacterInfo(WCHAR character, XMFLOAT2* outTexCoord, XMFLOAT2* outTexCoordSize, UINT* outWidth)
{
    if (!ContainsCharacter(character))
    {
        return false;
    }

    CharInfo info = _charMap[character];
    *outTexCoord = XMFLOAT2(info.X, info.Y);
    *outTexCoordSize = XMFLOAT2(info.Width, info.Height);
    *outWidth = info.PixelWidth;

    return true;
}

UINT SpriteFont::GetLineSpacing()
{
    return _lineSpacing;
}

ID3D11ShaderResourceView* SpriteFont::GetFontShaderResourceView()
{
    return _fontSRV;
}

void SpriteFont::Destroy()
{
    SAFE_RELEASE(_fontSRV);
    _charMap.clear();
}

HRESULT SpriteFont::Compile(ID3D11Device* device, const WCHAR* fileName, std::ostream* output)
{
    HRESULT hr;

    char sPath[MAX_PATH];
    if (!WStringToAnsi(fileName, sPath, MAX_PATH))
    {
        return E_FAIL;
    }

    TiXmlDocument doc = TiXmlDocument(sPath);
    if (!doc.LoadFile())
    {
        return E_FAIL;
    }

    TiXmlElement* root = doc.FirstChildElement("fontMetrics");
    if (!root)
    {
        return E_FAIL;
    }

    const char* texName = root->Attribute("file");
    if (!texName)
    {
        return E_FAIL;
    }

    WCHAR wTextureName[MAX_PATH];
    if (!AnsiToWString(texName, wTextureName, MAX_PATH))
    {
        return E_FAIL;
    }

    WCHAR wDirName[MAX_PATH];
    if (!GetDirectoryFromFileNameW(fileName, wDirName, MAX_PATH))
    {
        return E_FAIL;
    }

    WCHAR fullPath[MAX_PATH];
    wcsncpy_s(fullPath, wDirName, MAX_PATH);
    wcsncat_s(fullPath, L"\\", MAX_PATH);
    wcsncat_s(fullPath, wTextureName, MAX_PATH);

    std::ifstream file;
    file.open(fullPath, std::ios::in | std::ios::binary);

    if(!file.is_open())
    {
        E_FAIL;
    }

    file.seekg(0, ios::end);
    int fileSize = file.tellg();
    file.seekg(0, ios::beg);

    if (fileSize > 0)
    {
        BYTE* buf = new BYTE[fileSize];

        file.read((char*)buf, fileSize);
        output->write((const char*)&fileSize, sizeof(int));
        output->write((const char*)buf, fileSize);

        SAFE_DELETE_ARRAY(buf);
    }
    else
    {
        return E_FAIL;
    }

    D3DX11_IMAGE_INFO imgInfo;
    V_RETURN(D3DX11GetImageInfoFromFile(fullPath, NULL, &imgInfo, NULL));

    output->write((const char*)&imgInfo.Width, sizeof(UINT));
    output->write((const char*)&imgInfo.Height, sizeof(UINT));

    std::map<UINT, CharInfo> charInfoMap;
    UINT lineSpacing;

    TiXmlElement* child = root->FirstChildElement("character");
    while (child)
    {
        int character;
        if (!child->Attribute("key", &character))
        {
            return E_FAIL;
        }

        int x, y, width, height;

        TiXmlElement* element = child->FirstChildElement("x");
        if (!element)
        {
            return E_FAIL;
        }
        x = atoi(element->GetText());

        element = child->FirstChildElement("y");
        if (!element)
        {
            return E_FAIL;
        }
        y = atoi(element->GetText());

        element = child->FirstChildElement("width");
        if (!element)
        {
            return E_FAIL;
        }
        width = atoi(element->GetText());

        element = child->FirstChildElement("height");
        if (!element)
        {
            return E_FAIL;
        }
        height = atoi(element->GetText());

        CharInfo info;
        info.X = x / (float)imgInfo.Width;
        info.Y = y / (float)imgInfo.Height;
        info.Width = width / (float)imgInfo.Width;
        info.Height = height / (float)imgInfo.Height;
        info.PixelWidth = width;
        lineSpacing = height;

        charInfoMap[(UINT)character] = info;

        child = child->NextSiblingElement("character");
    }

    UINT numChars = charInfoMap.size();
    output->write((const char*)&numChars, sizeof(UINT));

    for (std::map<UINT, CharInfo>::iterator i = charInfoMap.begin(); i != charInfoMap.end(); i++)
    {
        output->write((const char*)&(i->first), sizeof(UINT));
        output->write((const char*)&(i->second), sizeof(CharInfo));
    }

    output->write((const char*)&lineSpacing, sizeof(UINT));

    return S_OK;
}

HRESULT SpriteFont::Create(ID3D11Device* device, std::istream* input, SpriteFont** output)
{
    HRESULT hr;

    SpriteFont* result = new SpriteFont();

    int imgSize;
    input->read((char*)&imgSize, sizeof(int));

    BYTE* imgData = new BYTE[imgSize];
    input->read((char*)imgData, imgSize);

    V_RETURN(D3DX11CreateShaderResourceViewFromMemory(device, imgData, imgSize, NULL, NULL, &result->_fontSRV,
        NULL));

    input->read((char*)&result->_textureWidth, sizeof(UINT));
    input->read((char*)&result->_textureHeight, sizeof(UINT));

    UINT charCount;
    input->read((char*)&charCount, sizeof(UINT));

    for (UINT i = 0; i < charCount; i++)
    {
        UINT item;
        input->read((char*)&item, sizeof(UINT));

        CharInfo info;
        input->read((char*)&info, sizeof(CharInfo));

        result->_charMap[item] = info;
    }

    input->read((char*)&result->_lineSpacing, sizeof(UINT));

    *output = result;
    return S_OK;
}