#include "PCH.h"
#include "ShaderContent.h"

HRESULT CompileShaderFromFile(const WCHAR* szFileName, const char* szEntryPoint, const char* szShaderModel,
	D3D_SHADER_MACRO* defines, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    // find the file
    WCHAR str[MAX_PATH];
    V_RETURN(DXUTFindDXSDKMediaFileCch(str, MAX_PATH, szFileName));

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DX11CompileFromFile(str, defines, NULL, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL );
    if( FAILED(hr) )
    {
        if( pErrorBlob != NULL )
            OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
        SAFE_RELEASE( pErrorBlob );
        return hr;
    }
    SAFE_RELEASE( pErrorBlob );

    return S_OK;
}

HRESULT PixelShaderContent::LoadContent(ID3D11Device* device, const WCHAR* path, ShaderOptions* options)
{
	if (!options)
	{
		return E_FAIL;
	}

	HRESULT hr;
	
	ID3DBlob* pShaderBlob = NULL;
	V_RETURN(CompileShaderFromFile(path, options->EntryPoint, "ps_5_0", options->Defines, &pShaderBlob));		
	V_RETURN(device->CreatePixelShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), NULL, &_ps));
	
	SAFE_RELEASE(pShaderBlob);

	return hr;
}

void PixelShaderContent::UnloadContent()
{
	SAFE_RELEASE(_ps);
}