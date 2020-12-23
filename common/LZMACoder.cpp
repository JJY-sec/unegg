#include "StdAfx.h"
#include "LZMACoder.h"

#include "../lib/lzma/LzmaDec.h"

#include "crc32.h"

typedef void* ISzAllocPtr;

void* SzAlloc(ISzAllocPtr, size_t size) { return malloc(size); }
void SzFree(ISzAllocPtr, void* address) { free(address); }
ISzAlloc g_SzAlloc = { SzAlloc, SzFree };

CLZMACoder::CLZMACoder(ICodeBuffer* buffer, ICoderCallback* callback)
    : buffer_(buffer), callback_(callback), crc_(0)
    , sResult_(SZ_OK)
{
}

CLZMACoder::~CLZMACoder()
{
}

HRESULT CLZMACoder::Decode(ISequentialInStream* inStream, ISequentialOutStream* outStream, UInt64, UInt64 unpackedSize, IDecryptor* decryptor)
{
    UINT32 uiRead;
    BYTE header[4 + LZMA_PROPS_SIZE];

    CRC32Init(&crc_);

    HRESULT ret = inStream->Read(header, sizeof(header), &uiRead);
    if (decryptor)
    {
        decryptor->Decrypt(header, uiRead);
    }

    if (ret == S_OK)
    {
        CLzmaDec state;
        LzmaDec_Construct(&state);
        sResult_ = LzmaDec_Allocate(&state, header + 4, LZMA_PROPS_SIZE, &g_SzAlloc);

        if (sResult_ == SZ_OK)
        {
            LzmaDec_Init(&state);

            LPBYTE inBuffer = buffer_->GetInBuffer();
            LPBYTE outBuffer = buffer_->GetOutBuffer();

            DWORD inPos = 0, inSize = 0;
            UInt64 remained = unpackedSize;
            UINT32 written;
            do
            {
                if (inPos == inSize)
                {
                    ret = inStream->Read(inBuffer, buffer_->GetInBufferSize(), (UInt32*)&inSize);
                    if (decryptor)
                    {
                        decryptor->Decrypt(inBuffer, inSize);
                    }
                    inPos = 0;
                }

                if (ret == S_OK)
                {
                    ELzmaFinishMode finishMode = LZMA_FINISH_ANY;
                    ELzmaStatus status;
                    SizeT inProcessed = inSize - inPos;
                    SizeT outProcessed = buffer_->GetOutBufferSize();

                    if (remained < outProcessed)
                    {
                        outProcessed = (UInt32)remained;
                        finishMode = LZMA_FINISH_END;
                    }

                    sResult_ = LzmaDec_DecodeToBuf(&state, outBuffer, &outProcessed, inBuffer + inPos, &inProcessed, finishMode, &status);
                    inPos += inProcessed;

                    {
                        LPBYTE writeData = outBuffer;
                        CRC32Update(&crc_, writeData, outProcessed);

                        while ((ret == S_OK) && (outProcessed > 0))
                        {
                            ret = outStream->Write(writeData, outProcessed, &written);
                            if (ret == S_OK)
                            {
                                writeData += written;
                                outProcessed -= written;
                                remained -= written;

                                ret = callback_->AddCompleted(written);
                            }
                        }
                    }
                }
            } while ((ret == S_OK) && (sResult_ == SZ_OK) && (remained > 0));
        }

        LzmaDec_Free(&state, &g_SzAlloc);
    }

    CRC32Finish(&crc_);

    return ret;
}

int CLZMACoder::GetResult(UInt32 crc)
{
    int ret = NArchive::NExtract::NOperationResult::kOK;
    switch (sResult_)
    {
    case SZ_OK:                 ret = NArchive::NExtract::NOperationResult::kOK;                    break;
    case SZ_ERROR_PARAM:
    case SZ_ERROR_DATA:         ret = NArchive::NExtract::NOperationResult::kDataError;             break;
    case SZ_ERROR_CRC:          ret = NArchive::NExtract::NOperationResult::kCRCError;              break;
    case SZ_ERROR_UNSUPPORTED:  ret = NArchive::NExtract::NOperationResult::kUnsupportedMethod;     break;
    case SZ_ERROR_INPUT_EOF:
    case SZ_ERROR_OUTPUT_EOF:   ret = NArchive::NExtract::NOperationResult::kUnexpectedEnd;         break;
    default:                    ret = NArchive::NExtract::NOperationResult::kUnavailable;           break;
    }

    if ((ret == NArchive::NExtract::NOperationResult::kOK) && (crc != crc_))
    {
        ret = NArchive::NExtract::NOperationResult::kCRCError;
    }
    return ret;
}
