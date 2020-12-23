#include "StdAfx.h"
#include "CoderImpl.h"

CCoderCallback::CCoderCallback(IArchiveExtractCallback* callback)
    : completed_(0), callback_(callback)
{
}

CCoderCallback::~CCoderCallback()
{
}

HRESULT CCoderCallback::AddCompleted(UInt32 processed)
{
    completed_ += processed;
    return callback_->SetCompleted(&completed_);
}


enum { bufferSize = 0x100000 };

CCodeBuffer::CCodeBuffer()
    : inBufferSize_(bufferSize), outBufferSize_(bufferSize)
{
    inBuffer_ = reinterpret_cast<LPBYTE>(malloc(inBufferSize_));
    outBuffer_ = reinterpret_cast<LPBYTE>(malloc(outBufferSize_));
}

CCodeBuffer::~CCodeBuffer()
{
    free(outBuffer_);
    free(inBuffer_);
}

UInt32 CCodeBuffer::GetInBufferSize()
{
    return inBufferSize_;
}

LPBYTE CCodeBuffer::GetInBuffer()
{
    return inBuffer_;
}

UInt32 CCodeBuffer::GetOutBufferSize()
{
    return outBufferSize_;
}

LPBYTE CCodeBuffer::GetOutBuffer()
{
    return outBuffer_;
}
