#include "StdAfx.h"
#include "MultipleStream.h"

CInMultipleStream::CInMultipleStream(IInStream* firstStream)
    : currentPos_(0)
{
    firstStream->Seek(0, SEEK_CUR, &currentPos_);
    streams_.push_back(INSTREAM_INFORMATION(firstStream, 0));
    current_ = streams_.begin();
}

CInMultipleStream::~CInMultipleStream()
{
}

HRESULT CInMultipleStream::QueryInterface(REFIID riid, void **ppvObject)
{
    HRESULT ret = S_OK;
    if (riid == IID_IUnknown)
    {
        *ppvObject = (void*)(IUnknown*)this;
        AddRef();
    }
    else if (riid == IID_ISequentialInStream)
    {
        *ppvObject = (void*)(ISequentialInStream*)this;
        AddRef();
    }
    else if (riid == IID_IInStream)
    {
        *ppvObject = (void*)(IInStream*)this;
        AddRef();
    }
    else
    {
        ret = E_NOINTERFACE;
        *ppvObject = NULL;
    }
    return ret;
}

HRESULT CInMultipleStream::Read(void *data, UInt32 size, UInt32 *processedSize)
{
    HRESULT ret = S_OK;
    UInt32 processedTotal = 0;
    UInt32 processedCurrent = 0;
    while ((ret == S_OK) && size && (current_ != streams_.end()))
    {
        ret = current_->inStream->Read((unsigned char*)data + processedTotal, size, &processedCurrent);

        if (ret == S_OK)
        {
            if (processedCurrent < size)
            {
                ++current_;
                if (current_ != streams_.end())
                {
                    current_->inStream->Seek(0, SEEK_SET, NULL);
                }
            }
            processedTotal += processedCurrent;
            size -= processedCurrent;

            currentPos_ += processedCurrent;
        }
    }
    if (processedSize)
    {
        *processedSize = processedTotal;
    }
    return ret;
}

HRESULT CInMultipleStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition)
{
    HRESULT ret = S_OK;
    Int64 newPos = 0;
    Int64 totalSize = (Int64)(streams_.rbegin()->endOffset);
    switch (seekOrigin)
    {
    case SEEK_SET:      newPos = abs(offset);                       break;
    case SEEK_CUR:      newPos = (Int64)currentPos_ + offset;       break;
    case SEEK_END:      newPos = totalSize - abs(offset);           break;
    default:            ret = E_INVALIDARG;                         break;
    }
    if (ret == S_OK)
    {
        if (newPos < 0)
        {
            newPos = 0;
        }
        else if (totalSize < newPos)
        {
            newPos = totalSize;
        }

        if (currentPos_ != (UInt64)newPos)
        {
            Seek(newPos);
        }

        if (newPosition)
        {
            *newPosition = currentPos_;
        }
    }
    return ret;
}

void CInMultipleStream::AddStream(IInStream* inStream)
{
    streams_.push_back(INSTREAM_INFORMATION(inStream, streams_.rbegin()->endOffset));
}

void CInMultipleStream::Seek(Int64 offset)
{
    currentPos_ = (UInt64)offset;

    auto it = streams_.begin();
    while (it->endOffset < currentPos_)
    {
        ++it;
    }

    current_ = it;
    if (current_ != streams_.end())
    {
        current_->inStream->Seek((Int64)(currentPos_ - current_->startOffset), SEEK_SET, NULL);
    }
}
