#ifndef __IPASSWORD_H
#define __IPASSWORD_H

#include "../lib/MyGuidDef.h"
#include "../lib/MyWindows.h"

// {B44B323E-56BA-4FA6-8D50-AE44F99E063D}
DEFINE_GUID(IID_ICryptoGetTextPassword, 0xb44b323e, 0x56ba, 0x4fa6, 0x8d, 0x50, 0xae, 0x44, 0xf9, 0x9e, 0x6, 0x3d);
class ICryptoGetTextPassword : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE CryptoGetTextPassword(BSTR *password) = 0;
};

#endif
