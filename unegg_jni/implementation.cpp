#include "StdAfx.h"

#ifndef _WIN32
#include <unistd.h>
#include <sys/stat.h>
#endif

#include "common/IArchive.h"
#include "common/IPassword.h"

#include "common/UnknownImpl.h"

#include "common/PropID.h"

#include "lib/PropVariant.h"
#include "lib/egg/EggArchiveInfo.h"

#include "common/GeneralFileStream.h"

#include "exports.h"

void CreateDirectories(LPCSTR path)
{
#ifdef _WIN32
    if (!CreateDirectory(path, NULL) && (GetLastError() == ERROR_PATH_NOT_FOUND))
#else
    if ((mkdir(path, 0700) == -1) && (errno == ENOENT))
#endif
    {
        LPSTR _path = new char[strlen(path) + 1];
        strcpy(_path, path);

        LPSTR p = strrchr(_path, '/');
        if (p)
        {
            *p = (char)NULL;
            CreateDirectories(_path);
            *p = '/';

#ifdef _WIN32
            CreateDirectory(path, NULL);
#else
            mkdir(path, 0700);
#endif
        }

        delete[] _path;
    }
}

class CArchiveOpenCallback: public IAddRefReleaseImpl<
    IArchiveOpenCallback, ICryptoGetTextPassword, IArchiveOpenVolumeCallback
>
{
public:
    START_QUERYINTERFACE
        QUERY_UNKNOWN(IArchiveOpenCallback)
        QUERY_INTERFACE_IID(IArchiveOpenCallback)
        QUERY_INTERFACE_IID(ICryptoGetTextPassword)
        QUERY_INTERFACE_IID(IArchiveOpenVolumeCallback)
    END_QUERYINTERFACE

    STDMETHOD(SetTotal)(const UInt64 *files, const UInt64 *bytes)
    {
        return S_OK;
    }
    STDMETHOD(SetCompleted)(const UInt64 *files, const UInt64 *bytes)
    {
        return S_OK;
    }

    STDMETHOD(CryptoGetTextPassword)(BSTR *password)
    {
        return ercInvalidPassword;
    }

    STDMETHOD(GetProperty)(PROPID propID, PROPVARIANT *value)
    {
        HRESULT ret = S_OK;
        NWindows::NCOM::CPropVariant prop;
        switch (propID)
        {
        case kpidPath:
        {
            string path;
            if (folderPath_.size())
            {
                path.format("%s/%s",
                    folderPath_.c_str(), fileName_.c_str()
                );
            }
            else
            {
                path = fileName_;
            }
            prop = path.c_str();
            break;
        }
        case kpidName:      prop = fileName_.c_str();                       break;
        default:            ret = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);  break;
        }
        prop.Detach(value);
        return ret;
    }

    STDMETHOD(GetStream)(const wchar_t *name, IInStream **inStream)
    {
        HRESULT ret = S_OK;
        if (name)
        {
            if (wcsrchr(name, L'/'))
            {
                LPWSTR path = new wchar_t[wcslen(name) + 1];
                wcscpy(path, name);

                LPCWSTR folderPath = path;
                LPWSTR fileName = wcsrchr(path, L'/');
                (*fileName) = (WCHAR)NULL;
                fileName++;

                folderPath_ = wstring(folderPath).toutf8();
                fileName_ = wstring(fileName).toutf8();
                delete[] path;
            }
            else
            {
                fileName_ = wstring(name).toutf8();
            }

            string path;
            if (folderPath_.size())
            {
                path.format("%s/%s",
                    folderPath_.c_str(), fileName_.c_str()
                );
            }
            else
            {
                path = fileName_;
            }

            CInGeneralFileStream* fileStream = new CInGeneralFileStream();
            CComPtr<IInStream> _inStream(fileStream);
            if (fileStream->Open(path))
            {
                *inStream = _inStream.Detach();
            }
            else
            {
                if (errno == ENOENT)
                {
                    ret = ercFileNotFound;
                }
                else
                {
                    ret = ercOpenFailed;
                }
            }
        }
        else
        {
            ret = ercInvalidParameters;
        }
        return ret;
    }

    CArchiveOpenCallback() {}

private:
    std::string folderPath_;
    std::string fileName_;
};

class CArchiveExtractCallback: public IAddRefReleaseImpl<
    IArchiveExtractCallback,
    ICryptoGetTextPassword
>
{
public:
    START_QUERYINTERFACE
        QUERY_UNKNOWN(IArchiveExtractCallback)
        QUERY_INTERFACE_IID(IArchiveExtractCallback)
        QUERY_INTERFACE_IID(ICryptoGetTextPassword)
    END_QUERYINTERFACE

    // IProgress
    STDMETHOD(SetTotal)(UInt64 size)
    {
        return S_OK;
    }
    STDMETHOD(SetCompleted)(const UInt64 *completeValue)
    {
        return S_OK;
    }

    // IArchiveExtractCallback
    STDMETHOD(GetStream)(UInt32 index, ISequentialOutStream **outStream, Int32 askExtractMode)
    {
        *outStream = 0;

        if (askExtractMode != NArchive::NExtract::NAskMode::kExtract)
        {
            return S_OK;
        }

        string fullPath;
        {
            // Get Name
            NWindows::NCOM::CPropVariant prop;
            RINOK(archive_->GetProperty(index, kpidPath, &prop));

            if (prop.vt != VT_BSTR)
            {
                return E_FAIL;
            }
            fullPath = wstring(prop.bstrVal).toutf8();
        }

        bool isDir = false;
        {
            NWindows::NCOM::CPropVariant prop;
            RINOK(archive_->GetProperty(index, kpidIsDir, &prop));
            if (prop.vt == VT_BOOL)
            {
                isDir = (prop.boolVal != VARIANT_FALSE);
            }
            else if (prop.vt != VT_EMPTY)
            {
                return E_FAIL;
            }
        }

        string realPath = destPath_ + '/' + fullPath;
        if (isDir)
        {
            CreateDirectories(realPath);
        }
        else
        {
            CreateDirectories(realPath.substr(0, realPath.find_last_of('/')).c_str());

            remove(realPath);

            COutGeneralFileStream* outFileStreamSpec = new COutGeneralFileStream;
            outStream_ = outFileStreamSpec;
            if (outFileStreamSpec->Create(realPath))
            {
                *outStream = outFileStreamSpec;
                (*outStream)->AddRef();
            }
        }

        return S_OK;
    }

    STDMETHOD(PrepareOperation)(Int32 askExtractMode)
    {
        return S_OK;
    }
    STDMETHOD(SetOperationResult)(Int32 resultEOperationResult)
    {
        outStream_.Release();
        if (resultEOperationResult == NArchive::NExtract::NOperationResult::kWrongPassword)
        {
            return ercInvalidPassword;
        }
        return S_OK;
    }

    // ICryptoGetTextPassword
    STDMETHOD(CryptoGetTextPassword)(BSTR *aPassword)
    {
        if (password_.size() != 0)
        {
            *aPassword = SysAllocString(wstring(password_).c_str());
            return S_OK;
        }
        return ercInvalidPassword;
    }

    CArchiveExtractCallback(IInArchive* archive, const char *destPath, const char* password)
        : archive_(archive), destPath_(destPath), password_(password ? password : "")
    {}

private:
    CComPtr<IInArchive> archive_;
    std::string destPath_;
    std::string password_;
    CComPtr<ISequentialOutStream> outStream_;
};

HRESULT OpenArchive(const char* path, IInArchive** archive)
{
    HRESULT ret = S_OK;

    CComPtr<IInStream> file;

    CArchiveOpenCallback *openCallbackSpec = new CArchiveOpenCallback();
    CComPtr<IArchiveOpenCallback> openCallback(openCallbackSpec);

    ret = openCallbackSpec->GetStream(wstring().fromutf8(path).c_str(), &file);
    if (ret != S_OK)
    {
        return ret;
    }

    CComPtr<IInArchive> _archive = new NArchive::NEgg::CInArchive();

    const UInt64 scanSize = 0;
    ret = _archive->Open(file, &scanSize, openCallback);

    if (ret == S_OK)
    {
        *archive = _archive.Detach();
    }

    return ret;
}

HRESULT ExtractArchive(IInArchive* archive, const char* destination, const char* password)
{
    CComPtr<IInArchive> _archive(archive);
    CArchiveExtractCallback *extractCallbackSpec = new CArchiveExtractCallback(archive, destination, password);
    CComPtr<IArchiveExtractCallback> extractCallback(extractCallbackSpec);
    return archive->Extract(NULL, -1, false, extractCallback);
}
