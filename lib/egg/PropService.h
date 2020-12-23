#ifndef __ALEGG_PROP_SERVICE_H__
#define __ALEGG_PROP_SERVICE_H__

namespace PropService
{
    HRESULT GetPropertyCount(const std::vector<BYTE>& props, UINT32* numProps);
    HRESULT GetPropertyInfo(const std::vector<BYTE>& props, UINT32 index, BSTR *name, PROPID *propID, VARTYPE *varType);
}

#endif
