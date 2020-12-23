#include "StdAfx.h"
#include "PropService.h"

#include "../../common/PropID.h"

namespace PropService
{
    HRESULT GetPropertyCount(const std::vector<BYTE>& props, UINT32* numProps)
    {
        if (numProps)
        {
            *numProps = props.size();
            return S_OK;
        }
        return E_INVALIDARG;
    }

    HRESULT GetPropertyInfo(const std::vector<BYTE>& props, UINT32 index, BSTR *name, PROPID *propID, VARTYPE *varType)
    {
        if (index < props.size())
        {
            if (name)       *name = 0;
            if (propID)     *propID = props[index];
            if (varType)    *varType = k7z_PROPID_To_VARTYPE[props[index]];
            return S_OK;
        }
        return E_INVALIDARG;
    }
}
