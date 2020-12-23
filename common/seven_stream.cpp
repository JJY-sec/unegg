#include "StdAfx.h"
#include "seven_stream.h"

#ifndef _WIN32

#if UNICODE
#define _tcsrchr        wcsrchr
#else
#define _tcsrchr        strrchr
#endif

#endif

seven_istream::seven_istream(IInStream* inStream)
    : inStream_(inStream)
{
}

seven_istream::~seven_istream()
{
}

HRESULT seven_istream::read(void* data, UINT32 size, UINT32* processedSize)
{
    UINT32 readSize;
    HRESULT result = inStream_->Read(data, size, &readSize);
    if (result == S_OK)
    {
        if (readSize == 0)
        {
            add_state(std::ios_base::eofbit);
            result = HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
        }

        if (processedSize)
        {
            *processedSize = readSize;
        }
    }
    else
    {
        add_state(std::ios_base::failbit);
    }

    set_result(result);
    return get_result();
}

HRESULT seven_istream::seek(LONGLONG offset, UINT32 origin, UINT64* newPosition)
{
    UINT64 _newPosition;
    HRESULT result = inStream_->Seek(offset, origin, &_newPosition);
    if (result == S_OK)
    {
        if (newPosition)
        {
            *newPosition = _newPosition;
        }
    }
    else
    {
        add_state(std::ios_base::failbit);
    }
    set_result(result);
    return get_result();
}


seven_ostream::seven_ostream(ISequentialOutStream* outStream)
    : outStream_(outStream)
{
}

seven_ostream::~seven_ostream()
{
}

HRESULT seven_ostream::write(const void* data, UINT32 size, UINT32* processedSize)
{
    UINT32 writtenSize;
    HRESULT result = outStream_->Write(data, size, &writtenSize);
    if (result == S_OK)
    {
        if (processedSize)
        {
            *processedSize = writtenSize;
        }
    }
    else
    {
        add_state(std::ios_base::failbit);
    }
    set_result(result);
    return get_result();
}

HRESULT seven_ostream::pad(UINT32 size, UINT32* processedSize)
{
    static unsigned char data[1024] = { 0, };

    HRESULT result = S_OK;
    UINT32 writtenSize = 0;
    UINT32 currentWritten;
#ifdef _WIN32
    while (size && ((result = outStream_->Write(data, min((UINT32)1024, size), &currentWritten)) == S_OK))
#else
    while (size && ((result = outStream_->Write(data, std::min((UINT32)1024, size), &currentWritten)) == S_OK))
#endif
    {
        writtenSize += currentWritten;
        size -= currentWritten;
    }

    if (result == S_OK)
    {
        if (processedSize)
        {
            *processedSize = writtenSize;
        }
    }
    else
    {
        add_state(std::ios_base::failbit);
    }

    set_result(result);
    return get_result();
}
