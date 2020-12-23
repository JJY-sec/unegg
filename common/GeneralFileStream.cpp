#include "StdAfx.h"
#include "GeneralFileStream.h"

COutGeneralFileStream::COutGeneralFileStream()
#ifdef _WIN32
    : file_(INVALID_HANDLE_VALUE)
#else
    : file_(NULL)
#endif
{
}

COutGeneralFileStream::~COutGeneralFileStream()
{
    Close();
}

HRESULT COutGeneralFileStream::QueryInterface(REFIID riid, void **ppvObject)
{
    HRESULT ret = S_OK;
    if (riid == IID_IUnknown)
    {
        *ppvObject = (void*)(IUnknown*)(ISequentialOutStream*)this;
        AddRef();
    }
    else if (riid == IID_ISequentialOutStream)
    {
        *ppvObject = (void*)(ISequentialOutStream*)this;
        AddRef();
    }
    else if (riid == IID_IOutStream)
    {
        *ppvObject = (void*)(IOutStream*)this;
        AddRef();
    }
#ifdef _WIN32
    else if (riid == __uuidof(ISetModifiedTime))
    {
        *ppvObject = (void*)(ISetModifiedTime*)this;
        AddRef();
    }
#endif
    else
    {
        ret = E_NOINTERFACE;
        *ppvObject = NULL;
    }
    return ret;
}

HRESULT COutGeneralFileStream::Write(const void *data, UInt32 size, UInt32 *processedSize)
{
    HRESULT ret = S_OK;
#ifdef _WIN32
    if (file_ != INVALID_HANDLE_VALUE)
    {
        DWORD written;
        if (WriteFile(file_, data, size, &written, NULL))
        {
            if (processedSize)
            {
                *processedSize = written;
            }
        }
        else
        {
            ret = HRESULT_FROM_WIN32(GetLastError());
        }
    }
#else
    if (file_ != NULL)
    {
        UInt32 written = fwrite(data, 1, size, file_);
        if (written == size)
        {
            if (processedSize)
            {
                *processedSize = written;
            }
        }
        else
        {
            ret = E_FAIL;
        }
    }
#endif
    else
    {
        ret = HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE);
    }
    return ret;
}

HRESULT COutGeneralFileStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition)
{
    HRESULT ret = S_OK;
#ifdef _WIN32
    if (file_ != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER liOffset, liNewPosition;
        liOffset.QuadPart = offset;
        if (SetFilePointerEx(file_, liOffset, &liNewPosition, seekOrigin))
        {
            if (newPosition)
            {
                *newPosition = (UINT64)liNewPosition.QuadPart;
            }
        }
        else
        {
            ret = HRESULT_FROM_WIN32(GetLastError());
        }
    }
#else
    if (file_ != NULL)
    {
        if (fseek(file_, (Int32)offset, seekOrigin) == 0)
        {
            if (newPosition)
            {
                *newPosition = (UInt64)ftell(file_);
            }
        }
        else
        {
            ret = E_FAIL;
        }
    }
#endif
    else
    {
        ret = HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE);
    }
    return ret;
}

HRESULT COutGeneralFileStream::SetSize(UInt64 newSize)
{
#ifdef _WIN32
    HRESULT ret = S_OK;
    if (file_ != INVALID_HANDLE_VALUE)
    {
        UINT64 current;
        if (true
            && ((ret = Seek(0, FILE_CURRENT, &current)) == S_OK)
            && ((ret = Seek(newSize, FILE_BEGIN, NULL)) == S_OK)
            )
        {
            if (SetEndOfFile(file_))
            {
                Seek(current, FILE_BEGIN, NULL);
            }
            else
            {
                ret = HRESULT_FROM_WIN32(GetLastError());
            }
        }
    }
    else
    {
        ret = HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE);
    }
    return ret;
#else
    return E_FAIL;
#endif
}

#ifdef _WIN32
HRESULT COutGeneralFileStream::SetModifiedTime(const LPFILETIME modifiedTime)
{
    if (SetFileTime(file_, NULL, NULL, modifiedTime))
    {
        return S_OK;
    }
    return HRESULT_FROM_WIN32(GetLastError());
}
#endif

bool COutGeneralFileStream::Create(LPCTSTR path)
{
    Close();
#ifdef _WIN32
    file_ = CreateFile(path, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    return (file_ != INVALID_HANDLE_VALUE);
#else
    file_ = fopen(path, "wb");
    return (file_ != NULL);
#endif
}

#ifdef _WIN32
bool COutGeneralFileStream::Open(LPCTSTR path)
{
    Close();
    file_ = CreateFile(path, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    return (file_ != INVALID_HANDLE_VALUE);
}
#endif

void COutGeneralFileStream::Close()
{
#ifdef _WIN32
    if (file_ != INVALID_HANDLE_VALUE)
    {
        CloseHandle(file_);
        file_ = INVALID_HANDLE_VALUE;
    }
#else
    if (file_ != NULL)
    {
        fclose(file_);
        file_ = NULL;
    }
#endif
}


CInGeneralFileStream::CInGeneralFileStream()
#ifdef _WIN32
    : file_(INVALID_HANDLE_VALUE)
#else
    : file_(NULL)
#endif
{
}

CInGeneralFileStream::~CInGeneralFileStream()
{
    Close();
}

HRESULT CInGeneralFileStream::QueryInterface(REFIID riid, void **ppvObject)
{
    HRESULT ret = S_OK;
    if (riid == IID_IUnknown)
    {
        *ppvObject = (void*)(IUnknown*)this;
        AddRef();
    }
    else if (riid == IID_IInStream)
    {
        *ppvObject = (void*)(IInStream*)this;
        AddRef();
    }
    else if (riid == IID_ISequentialInStream)
    {
        *ppvObject = (void*)(ISequentialInStream*)this;
        AddRef();
    }
    else
    {
        ret = E_NOINTERFACE;
        *ppvObject = NULL;
    }
    return ret;
}

HRESULT CInGeneralFileStream::Read(void *data, UInt32 size, UInt32 *processedSize)
{
    HRESULT ret = S_OK;
#ifdef _WIN32
    if (file_ != INVALID_HANDLE_VALUE)
    {
        DWORD dwRead;
        if (ReadFile(file_, data, size, &dwRead, NULL))
        {
            if (processedSize)
            {
                *processedSize = dwRead;
            }
        }
        else
        {
            ret = HRESULT_FROM_WIN32(GetLastError());
        }
    }
#else
    if (file_ != NULL)
    {
        UInt32 uiRead = fread(data, 1, size, file_);
        if (processedSize)
        {
            *processedSize = uiRead;
        }
    }
#endif
    else
    {
        ret = HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE);
    }
    return ret;
}

HRESULT CInGeneralFileStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition)
{
    HRESULT ret = S_OK;
#ifdef _WIN32
    if (file_ != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER liOffset, liNewOffset;
        liOffset.QuadPart = offset;
        if (SetFilePointerEx(file_, liOffset, &liNewOffset, seekOrigin))
        {
            if (newPosition)
            {
                *newPosition = (UINT64)liNewOffset.QuadPart;
            }
        }
        else
        {
            ret = HRESULT_FROM_WIN32(GetLastError());
        }
    }
#else
    if (file_ != NULL)
    {
        if (fseek(file_, (Int32)offset, seekOrigin) == 0)
        {
            if (newPosition)
            {
                *newPosition = (UInt64)ftell(file_);
            }
        }
        else
        {
            ret = E_FAIL;
        }
    }
#endif
    else
    {
        ret = HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE);
    }
    return ret;
}

bool CInGeneralFileStream::Open(LPCTSTR path)
{
    Close();
#ifdef _WIN32
    file_ = CreateFile(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    return (file_ != INVALID_HANDLE_VALUE);
#else
    file_ = fopen(path, "rb");
    return (file_ != NULL);
#endif
}

void CInGeneralFileStream::Close()
{
#ifdef _WIN32
    if (file_ != INVALID_HANDLE_VALUE)
    {
        CloseHandle(file_);
        file_ = INVALID_HANDLE_VALUE;
    }
#else
    if (file_ != NULL)
    {
        fclose(file_);
        file_ = NULL;
    }
#endif
}
