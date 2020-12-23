#ifndef __ALEGG_LZMA_CODER_H__
#define __ALEGG_LZMA_CODER_H__

typedef int SRes;

#include "ICoder.h"
#include "UnknownImpl.h"

class CLZMACoder : public TUnknown<IDecoder>
{
public:
    CLZMACoder(ICodeBuffer* buffer, ICoderCallback* callback);
    virtual ~CLZMACoder();

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

    SRes sResult_;
};

#endif
