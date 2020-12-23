#include "StdAfx.h"
#include "Bzip2Coder.h"

#include "../lib/bzip2/bzlib.h"

#include "crc32.h"

CBzip2Coder::CBzip2Coder(ICodeBuffer* buffer, ICoderCallback* callback)
    : buffer_(buffer), callback_(callback), crc_(0)
    , bzResult_(BZ_OK)
{
}

CBzip2Coder::~CBzip2Coder()
{
}

HRESULT CBzip2Coder::Decode(ISequentialInStream* inStream, ISequentialOutStream* outStream, UInt64 packedSize, UInt64, IDecryptor* decryptor)
{
    HRESULT ret = S_OK;

    bz_stream bzStream;
    memset(&bzStream, 0, sizeof(bzStream));

    bzResult_ = BZ2_bzDecompressInit(&bzStream, 0, 0);

    CRC32Init(&crc_);

    if (bzResult_ == BZ_OK)
    {
        char* inBuffer = reinterpret_cast<char*>(buffer_->GetInBuffer());
        char* outBuffer = reinterpret_cast<char*>(buffer_->GetOutBuffer());

        UInt64 remained = packedSize;
        UINT32 written;
        UINT in_done, out_done;

        do
        {
            if (bzStream.avail_in == 0)
            {
                ret = inStream->Read(inBuffer, (UInt32)(std::min<UInt64>(remained, buffer_->GetInBufferSize())), (UInt32*)&bzStream.avail_in);
                if (decryptor)
                {
                    decryptor->Decrypt(reinterpret_cast<LPBYTE>(inBuffer), bzStream.avail_in);
                }
                bzStream.next_in = inBuffer;
            }

            if (ret == S_OK)
            {
                bzStream.avail_out = buffer_->GetOutBufferSize();
                bzStream.next_out = outBuffer;

                in_done = bzStream.avail_in;
                out_done = bzStream.avail_out;
                bzResult_ = BZ2_bzDecompress(&bzStream);
                in_done -= bzStream.avail_in;
                out_done -= bzStream.avail_out;

                {
                    LPBYTE writeData = reinterpret_cast<LPBYTE>(outBuffer);
                    CRC32Update(&crc_, writeData, out_done);

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
                    bzResult_ = BZ_STREAM_END;
                }
            }
        } while ((ret == S_OK) && (bzResult_ == BZ_OK));
    }

    CRC32Finish(&crc_);
    BZ2_bzDecompressEnd(&bzStream);
    return ret;
}

int CBzip2Coder::GetResult(UInt32 crc)
{
    int ret = NArchive::NExtract::NOperationResult::kOK;
    switch (bzResult_)
    {
    case BZ_OK:
    case BZ_RUN_OK:
    case BZ_FLUSH_OK:
    case BZ_FINISH_OK:
    case BZ_STREAM_END:         ret = NArchive::NExtract::NOperationResult::kOK;            break;
    case BZ_SEQUENCE_ERROR:
    case BZ_PARAM_ERROR:
    case BZ_DATA_ERROR:
    case BZ_DATA_ERROR_MAGIC:   ret = NArchive::NExtract::NOperationResult::kDataError;     break;
    case BZ_UNEXPECTED_EOF:     ret = NArchive::NExtract::NOperationResult::kUnexpectedEnd; break;
    default:                    ret = NArchive::NExtract::NOperationResult::kUnavailable;   break;
    }

    if ((ret == NArchive::NExtract::NOperationResult::kOK) && (crc != crc_))
    {
        ret = NArchive::NExtract::NOperationResult::kCRCError;
    }
    return ret;
}
