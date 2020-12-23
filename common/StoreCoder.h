#ifndef __ALEGG_STORE_CODER_H__
#define __ALEGG_STORE_CODER_H__

#include "ICoder.h"
#include "UnknownImpl.h"

class CStoreCoder : public TUnknown<IDecoder>
{
public:
    CStoreCoder(ICodeBuffer* buffer, ICoderCallback* callback);
    virtual ~CStoreCoder();

#ifndef _WIN32
    START_QUERYINTERFACE
        QUERY_UNKNOWN(IDecoder)
        QUERY_INTERFACE(IDecoder)
    END_QUERYINTERFACE
#endif

    // IDecoder
    virtual HRESULT Decode(ISequentialInStream* inStream, ISequentialOutStream* outStream,
        UInt64 packedSize, UInt64 unpackedSize, IDecryptor* decryptor
    );
    virtual int GetResult(UInt32 crc);

private:
    CComPtr<ICodeBuffer> buffer_;
    CComPtr<ICoderCallback> callback_;
    UInt32 crc_;
};

#endif
