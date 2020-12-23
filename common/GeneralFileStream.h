#ifndef __GENERIAL_FILE_STREAM_H__
#define __GENERIAL_FILE_STREAM_H__

#include "IStream.h"

#ifdef _WIN32
#include "Interfaces.h"
#endif

#include "UnknownImpl.h"

class COutGeneralFileStream : public IAddRefReleaseImpl<
    IOutStream
#ifdef _WIN32
    ,ISetModifiedTime
#endif
>
{
public:
    COutGeneralFileStream();
    ~COutGeneralFileStream();

    // IUnknown
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);

    // IOutStream
    virtual HRESULT STDMETHODCALLTYPE Write(const void *data, UInt32 size, UInt32 *processedSize);
    virtual HRESULT STDMETHODCALLTYPE Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition);
    virtual HRESULT STDMETHODCALLTYPE SetSize(UInt64 newSize);

#ifdef _WIN32
    // ISetModifiedTime
    virtual HRESULT STDMETHODCALLTYPE SetModifiedTime(const LPFILETIME modifiedTime);
#endif

    bool Create(LPCTSTR path);
#ifdef _WIN32
    bool Open(LPCTSTR path);
#endif
    void Close();

private:
#ifdef _WIN32
    HANDLE file_;
#else
    FILE* file_;
#endif
};

class CInGeneralFileStream : public IAddRefReleaseImpl<IInStream>
{
public:
    CInGeneralFileStream();
    ~CInGeneralFileStream();

    // IUnknown
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);

    // IInStream
    virtual HRESULT STDMETHODCALLTYPE Read(void *data, UInt32 size, UInt32 *processedSize);
    virtual HRESULT STDMETHODCALLTYPE Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition);

    bool Open(LPCTSTR path);
    void Close();

private:
#ifdef _WIN32
    HANDLE file_;
#else
    FILE* file_;
#endif
};

#endif
