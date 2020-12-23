#ifndef __ALEGG_ICODER_H__
#define __ALEGG_ICODER_H__

#include "IStream.h"
#include "IArchive.h"
#include "ICrypto.h"

// {30D956BB-48C1-416B-BB2C-66651D5B69B5}
DEFINE_GUID(IID_IEncoder, 0x30d956bb, 0x48c1, 0x416b, 0xbb, 0x2c, 0x66, 0x65, 0x1d, 0x5b, 0x69, 0xb5);
struct DECLSPEC_UUID("30D956BB-48C1-416B-BB2C-66651D5B69B5") IEncoder : public IUnknown
{
    virtual HRESULT Encode() = 0;
};

// {1331650C-0C94-4E3A-AE6F-51B22FA5C203}
DEFINE_GUID(IID_IDecoder, 0x1331650c, 0xc94, 0x4e3a, 0xae, 0x6f, 0x51, 0xb2, 0x2f, 0xa5, 0xc2, 0x3);
struct DECLSPEC_UUID("1331650C-0C94-4E3A-AE6F-51B22FA5C203") IDecoder : public IUnknown
{
    virtual HRESULT Decode(ISequentialInStream* inStream, ISequentialOutStream* outStream,
        UInt64 packedSize, UInt64 unpackedSize, IDecryptor* decryptor
    ) = 0;

    // return is NArchive::NExtract::NOperationResult
    virtual int GetResult(UInt32 crc) = 0;
};

// {D6ED59C4-C12D-4369-9227-4D16E6EAC4F2}
DEFINE_GUID(IID_ICodeBuffer, 0xd6ed59c4, 0xc12d, 0x4369, 0x92, 0x27, 0x4d, 0x16, 0xe6, 0xea, 0xc4, 0xf2);
struct DECLSPEC_UUID("D6ED59C4-C12D-4369-9227-4D16E6EAC4F2") ICodeBuffer : public IUnknown
{
    virtual UInt32 GetInBufferSize() = 0;
    virtual LPBYTE GetInBuffer() = 0;
    virtual UInt32 GetOutBufferSize() = 0;
    virtual LPBYTE GetOutBuffer() = 0;
};

// {FA28D9FE-5791-4608-9445-A4B03E48EECA}
DEFINE_GUID(IID_ICoderCallback, 0xfa28d9fe, 0x5791, 0x4608, 0x94, 0x45, 0xa4, 0xb0, 0x3e, 0x48, 0xee, 0xca);
struct DECLSPEC_UUID("FA28D9FE-5791-4608-9445-A4B03E48EECA") ICoderCallback : public IUnknown
{
    virtual HRESULT AddCompleted(UInt32 processed) = 0;
};

#endif
