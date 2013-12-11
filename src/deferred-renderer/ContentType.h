#pragma once

#include "PCH.h"

struct ContentType : public IUnknown
{
private:
    UINT _refCount;

public:
    ContentType();
    virtual ~ContentType();

    UINT GetRefCount() const;

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();
};