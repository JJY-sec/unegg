#ifndef __ALEGG_CODER_IMPL_H__
#define __ALEGG_CODER_IMPL_H__

#include "ICoder.h"
#include "UnknownImpl.h"

class CCoderCallback : public TUnknown<ICoderCallback>
{
public:
    CCoderCallback(IArchiveExtractCallback* callback);
    virtual ~CCoderCallback();

#ifndef _WIN32
    START_QUERYINTERFACE
        QUERY_UNKNOWN(ICoderCallback)
        QUERY_INTERFACE(ICoderCallback)
    END_QUERYINTERFACE
#endif

    // ICoderCallback
    virtual HRESULT AddCompleted(UInt32 processed);

private:
    UInt64 completed_;
    CComPtr<IArchiveExtractCallback> callback_;
};


class CCodeBuffer : public TUnknown<ICodeBuffer>
{
public:
    CCodeBuffer();
    virtual ~CCodeBuffer();

#ifndef _WIN32
    START_QUERYINTERFACE
        QUERY_UNKNOWN(ICodeBuffer)
        QUERY_INTERFACE(ICodeBuffer)
    END_QUERYINTERFACE
#endif

    // ICodeBuffer
    virtual UInt32 GetInBufferSize();
    virtual LPBYTE GetInBuffer();
    virtual UInt32 GetOutBufferSize();
    virtual LPBYTE GetOutBuffer();

private:
    LPBYTE inBuffer_;
    UInt32 inBufferSize_;
    LPBYTE outBuffer_;
    UInt32 outBufferSize_;
};

#endif
