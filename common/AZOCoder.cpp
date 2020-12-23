#include "StdAfx.h"
#include "AZOCoder.h"

#include "../lib/azo/AZO.h"

#include "crc32.h"

CAZOCoder::CAZOCoder(ICodeBuffer* buffer, ICoderCallback* callback)
    : buffer_(buffer), callback_(callback), crc_(0)
    , azoResult_(AZO_OK)
{
}

CAZOCoder::~CAZOCoder()
{
}

HRESULT CAZOCoder::Decode(ISequentialInStream* inStream, ISequentialOutStream* outStream, UInt64 packedSize, UInt64, IDecryptor* decryptor)
{
    HRESULT ret = S_OK;

    AZO_HDECOMPRESS handle;
    azoResult_ = AZO_DecompressInit(&handle);

    UINT32 crc;
    CRC32Init(&crc);

    if (azoResult_ == AZO_OK)
    {
        AZO_Stream azoStream;
        memset(&azoStream, 0, sizeof(azoStream));

        char* inBuffer = reinterpret_cast<char*>(buffer_->GetInBuffer());
        char* outBuffer = reinterpret_cast<char*>(buffer_->GetOutBuffer());

        UInt64 remained = packedSize;
        UINT32 written;
        UINT in_done, out_done;

        do
        {
            if (azoStream.avail_in == 0)
            {
                ret = inStream->Read(inBuffer, (UInt32)(std::min<UInt64>(remained, buffer_->GetInBufferSize())), (UInt32*)&azoStream.avail_in);
                if (decryptor)
                {
                    decryptor->Decrypt(reinterpret_cast<LPBYTE>(inBuffer), azoStream.avail_in);
                }
                azoStream.next_in = inBuffer;
            }

            if (ret == S_OK)
            {
                azoStream.avail_out = buffer_->GetOutBufferSize();
                azoStream.next_out = outBuffer;

                in_done = azoStream.avail_in;
                out_done = azoStream.avail_out;
                azoResult_ = AZO_Decompress(handle, &azoStream);
                in_done -= azoStream.avail_in;
                out_done -= azoStream.avail_out;

                {
                    LPBYTE writeData = reinterpret_cast<LPBYTE>(outBuffer);
                    CRC32Update(&crc, writeData, out_done);

                    while ((ret == S_OK) && (out_done > 0))
                    {
                        ret = outStream->Write(writeData, out_done, &written);
                        if (ret == S_OK)
                        {
                            writeData += written;
                            out_done -= written;

                            ret = callback_->AddCompleted(written);
                        }
                    }
                }

                remained -= in_done;
                if (remained == 0)
                {
                    azoResult_ = AZO_STREAM_END;
                }
            }
        } while ((ret == S_OK) && (azoResult_ == AZO_OK));
    }

    CRC32Finish(&crc);
    AZO_DecompressEnd(handle);

    return ret;
}

int CAZOCoder::GetResult(UInt32 crc)
{
    int ret = NArchive::NExtract::NOperationResult::kOK;
    switch (azoResult_)
    {
    case AZO_OK:
    case AZO_STREAM_END:            ret = NArchive::NExtract::NOperationResult::kOK;            break;
    case AZO_PARAM_ERROR:
    case AZO_DATA_ERROR:
    case AZO_DATA_ERROR_VERSON:
    case AZO_DATA_ERROR_BLOCKSIZE:  ret = NArchive::NExtract::NOperationResult::kDataError;     break;
    default:                        ret = NArchive::NExtract::NOperationResult::kUnavailable;   break;
    }

    if ((ret == NArchive::NExtract::NOperationResult::kOK) && (crc != crc_))
    {
        ret = NArchive::NExtract::NOperationResult::kCRCError;
    }
    return ret;
}
