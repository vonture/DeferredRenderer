#include "PCH.h"
#include "ContentType.h"

ContentType::ContentType()
	: _refCount(1)
{
}

STDMETHODIMP ContentType::QueryInterface(REFIID riid, void** ppvObject)
{
    IUnknown *punk = nullptr;

    if (riid == IID_IUnknown)
	{
		punk = static_cast<IUnknown*>(this);
	}

    if (!punk)
	{
        return E_NOINTERFACE;
	}

    punk->AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) ContentType::AddRef()
{
    return ++_refCount;
}

STDMETHODIMP_(ULONG) ContentType::Release()
{
    ULONG cRef = --_refCount;

    if (cRef == 0)
        delete this;

    return cRef;
}