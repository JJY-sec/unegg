#ifndef __ALEGG_BZIP2_CODER_H__
#define __ALEGG_BZIP2_CODER_H__

#include "ICoder.h"
#include "UnknownImpl.h"

class CBzip2Coder : public TUnknown<IDecoder>
{
public:
    CBzip2Coder(ICodeBuffer* buffer, ICoderCallback* callback);
    virtual ~CBzip2Coder();

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

    int bzResult_;
};

#endif
