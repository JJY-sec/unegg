#include "StdAfx.h"
#include "StoreCoder.h"

#include "crc32.h"

CStoreCoder::CStoreCoder(ICodeBuffer* buffer, ICoderCallback* callback)
    : buffer_(buffer), callback_(callback), crc_(0)
{
}

CStoreCoder::~CStoreCoder()
{
}

HRESULT CStoreCoder::Decode(ISequentialInStream* inStream, ISequentialOutStream* outStream, UInt64 packedSize, UInt64, IDecryptor* decryptor)
{
    HRESULT ret = S_OK;

    CRC32Init(&crc_);
    LPBYTE inBuffer = buffer_->GetInBuffer();
    UInt64 remained = packedSize;
    UINT32 read, written;
    while ((ret == S_OK) && (remained > 0))
    {
        ret = inStream->Read(inBuffer, (UInt32)(std::min<UInt64>(remained, buffer_->GetInBufferSize())), &read);
        if (ret == S_OK)
        {
            if (decryptor)
            {
                decryptor->Decrypt(inBuffer, read);
            }
            ret = outStream->Write(inBuffer, read, &written);
            if (ret == S_OK)
            {
                remained -= read;
                ret = callback_->AddCompleted(written);
                CRC32Update(&crc_, inBuffer, read);
            }
        }
    }
    CRC32Finish(&crc_);
    return ret;
}

int CStoreCoder::GetResult(UInt32 crc)
{
    if (crc != crc_)
    {
        return NArchive::NExtract::NOperationResult::kCRCError;
    }
    return NArchive::NExtract::NOperationResult::kOK;
}
