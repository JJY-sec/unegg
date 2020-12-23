#ifndef __EGG_MULTIPLE_STREAM_H__
#define __EGG_MULTIPLE_STREAM_H__

#include "IArchive.h"
#include "UnknownImpl.h"

typedef struct _INSTREAM_INFORMATION
{
    CComPtr<IInStream> inStream;
    UInt64 startOffset;
    UInt64 endOffset;

    _INSTREAM_INFORMATION(IInStream* is, UInt64 sizePrev)
        : inStream(is)
    {
        UInt64 size;
        UInt64 current;
        is->Seek(0, SEEK_CUR, &current);
        is->Seek(0, SEEK_END, &size);
        is->Seek(current, SEEK_SET, NULL);

        startOffset = sizePrev;
        endOffset = sizePrev + size;
    }
} INSTREAM_INFORMATION, *PINSTREAM_INFORMATION;

typedef std::list<INSTREAM_INFORMATION> CInStreamList;

class CInMultipleStream : public IAddRefReleaseImpl<IInStream>
{
public:
    CInMultipleStream(IInStream* firstStream);
    ~CInMultipleStream();

    // IUnknown
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);

    // ISequentialInStream
    STDMETHOD(Read)(void *data, UInt32 size, UInt32 *processedSize);

    // IInStream
    STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition);

    void AddStream(IInStream* inStream);

private:
    void Seek(Int64 offset);

    CInStreamList streams_;
    CInStreamList::iterator current_;
    UInt64 currentPos_;
};

#endif
