#include "StdAfx.h"
#include "DeflateCoder.h"

#include "../lib/zlib/zlib.h"

#include "crc32.h"

CDeflateCoder::CDeflateCoder(ICodeBuffer* buffer, ICoderCallback* callback)
    : buffer_(buffer), callback_(callback), crc_(0)
    , zResult_(Z_OK)
{
}

CDeflateCoder::~CDeflateCoder()
{
}

HRESULT CDeflateCoder::Decode(ISequentialInStream* inStream, ISequentialOutStream* outStream, UInt64 packedSize, UInt64, IDecryptor* decryptor)
{
    HRESULT ret = S_OK;

    z_stream zStream;
    memset(&zStream, 0, sizeof(z_stream));
    /*
        이 스트림이 zlib 헤더를 포함하는것이 아니라면
        windowsBits에 -MAX_WBITS를 전달해야 합니다.
    */
    inflateInit2(&zStream, -MAX_WBITS);

    CRC32Init(&crc_);

    LPBYTE inBuffer = buffer_->GetInBuffer();
    LPBYTE outBuffer = buffer_->GetOutBuffer();

    UInt64 remained = packedSize;
    UINT32 written;
    ULONG in_done, out_done;

    do
    {
        if (zStream.avail_in == 0)
        {
            ret = inStream->Read(inBuffer, (UInt32)(std::min<UInt64>(remained, buffer_->GetInBufferSize())), (UInt32*)&zStream.avail_in);
            if (decryptor)
            {
                decryptor->Decrypt(inBuffer, zStream.avail_in);
            }
            zStream.next_in = inBuffer;
        }

        if (ret == S_OK)
        {
            zStream.avail_out = buffer_->GetOutBufferSize();
            zStream.next_out = outBuffer;

            in_done = zStream.avail_in;
            out_done = zStream.avail_out;
            zResult_ = inflate(&zStream, Z_NO_FLUSH);
            in_done -= zStream.avail_in;
            out_done -= zStream.avail_out;

            {
                LPBYTE writeData = outBuffer;
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
                zResult_ = Z_STREAM_END;
            }
        }
    } while ((ret == S_OK) && (zResult_ == Z_OK));

    CRC32Finish(&crc_);
    inflateEnd(&zStream);

    return ret;
}

int CDeflateCoder::GetResult(UInt32 crc)
{
    int ret = NArchive::NExtract::NOperationResult::kOK;
    switch (zResult_)
    {
    case Z_OK:
    case Z_STREAM_END:      ret = NArchive::NExtract::NOperationResult::kOK;            break;
    case Z_STREAM_ERROR:
    case Z_DATA_ERROR:      ret = NArchive::NExtract::NOperationResult::kDataError;     break;
    default:                ret = NArchive::NExtract::NOperationResult::kUnavailable;   break;
    }

    if ((ret == NArchive::NExtract::NOperationResult::kOK) && (crc != crc_))
    {
        ret = NArchive::NExtract::NOperationResult::kCRCError;
    }
    return ret;
}
