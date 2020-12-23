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

#define START_ERROR_CODE 79

enum ExtractResultCode
{
    ercSucceeded = 0,                   // S_OK

    ercBadFormat = START_ERROR_CODE,    // S_FALSE or HRESULT_CODE(hr) == ERROR_BAD_FORMAT
    ercNoInterface,                     // E_NOINTERFACE
    ercFileNotFound,                    // HRESULT_CODE(hr) == ERROR_FILE_NOT_FOUND
    ercOutOfMemory,                     // E_OUTOFMEMORY
    ercEndOfFile,                       // HRESULT_CODE(hr) == ERROR_HANDLE_EOF
    ercUnsupported,                     // HRESULT_CODE(hr) == ERROR_NOT_SUPPORTED
    ercInvalidParameters,               // HRESULT_CODE(hr) == ERROR_INVALID_PARAMETER
    ercInvalidFlags,                    // HRESULT_CODE(hr) == ERROR_INVALID_FLAGS

    ercInvalidPassword,                 // Invalid password
    ercOpenFailed,                      // Failed to file open

    ercUndefined,                       // Other

    ercLast = ercUndefined
};

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
#ifdef _WIN32
                if (GetLastError() == ERROR_FILE_NOT_FOUND)
#else
                if (errno == ENOENT)
#endif
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
    STDMETHOD(GetStream)(UInt32 index, ISequentialOutStream **outStream, Int32 askExtractMode);
    STDMETHOD(PrepareOperation)(Int32 askExtractMode)
    {
        return S_OK;
    }
    STDMETHOD(SetOperationResult)(Int32 resultEOperationResult)
    {
        printf("\n");
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

    CArchiveExtractCallback(IInArchive* archive, const char *s, const char* p)
        : archive_(archive), destPath_(s), password_(p)
    {}

private:
    IInArchive* archive_;
    std::string destPath_;
    std::string password_;
    CComPtr<ISequentialOutStream> outStream_;
};

STDMETHODIMP CArchiveExtractCallback::GetStream(UInt32 index,
    ISequentialOutStream **outStream, Int32 askExtractMode)
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

#ifdef _WIN32
        DeleteFile(realPath);
#else
        remove(realPath);
#endif

        COutGeneralFileStream* outFileStreamSpec = new COutGeneralFileStream;
        outStream_ = outFileStreamSpec;
        if (outFileStreamSpec->Create(realPath))
        {
            printf("%s", fullPath.c_str());
            *outStream = outFileStreamSpec;
            (*outStream)->AddRef();
        }
    }

    return S_OK;
}

typedef struct _COMMAND_LINE_INFO
{
    enum CommandList
    {
        clList,
        clExtract,
        clVersion,
        clHelp,

        clCount
    };
    int command;

    string archivePath;
    string destination;
    string password;

    _COMMAND_LINE_INFO() : command(clCount) {}
} COMMAND_LINE_INFO, *PCOMMAND_LINE_INFO;

HRESULT Extract(IInArchive* archive, const COMMAND_LINE_INFO& cli)
{
    CComPtr<IInArchive> _archive(archive);
    CArchiveExtractCallback *extractCallbackSpec = new CArchiveExtractCallback(archive, cli.destination.c_str(), cli.password.c_str());
    CComPtr<IArchiveExtractCallback> extractCallback(extractCallbackSpec);
    return archive->Extract(NULL, -1, false, extractCallback);
}

HRESULT List(IInArchive* archive, const COMMAND_LINE_INFO& cli)
{
    CComPtr<IInArchive> _archive(archive);
    UInt32 numItems = 0;
    HRESULT ret = archive->GetNumberOfItems(&numItems);
    for (UInt32 i = 0; (ret == S_OK) && (i < numItems); i++)
    {
        NWindows::NCOM::CPropVariant prop;
        if ((ret = archive->GetProperty(i, kpidPath, &prop)) == S_OK)
        {
            if (prop.vt == VT_BSTR)
            {
                printf("%s", wstring(prop.bstrVal).toutf8().c_str());
                printf("\n");
            }
            else if (prop.vt != VT_EMPTY)
            {
                ret = E_FAIL;
            }
        }
    }
    return ret;
}

HRESULT CommandWithArchive(const COMMAND_LINE_INFO& cli)
{
    HRESULT ret = S_OK;
    CComPtr<IInStream> file;

    CArchiveOpenCallback *openCallbackSpec = new CArchiveOpenCallback();
    CComPtr<IArchiveOpenCallback> openCallback(openCallbackSpec);

    ret = openCallbackSpec->GetStream(wstring().fromutf8(cli.archivePath).c_str(), &file);
    if (ret != S_OK)
    {
        return ret;
    }

    CComPtr<IInArchive> archive = new NArchive::NEgg::CInArchive();

    const UInt64 scanSize = 1 << 23;
    ret = archive->Open(file, &scanSize, openCallback);
    if (ret != S_OK)
    {
        return ret;
    }

    switch (cli.command)
    {
    case COMMAND_LINE_INFO::clExtract:      ret = Extract(archive, cli);        break;
    case COMMAND_LINE_INFO::clList:         ret = List(archive, cli);           break;
    }

    return ret;
}

void ShowVersion()
{
    printf("\nunegg v1.0\n");
    printf("Copyright(c) 2010 - present ESTsoft Corp. All rights reserved.\n\n");
}

void ShowUsage()
{
    printf("Usage : unegg [commands] [archive filename] [destination path].\n\n");
    printf("Available commands.\n");
    printf("-h\tDisplay this message.\n");
    printf("-v\tDisplay version.\n");
    printf("-l\tDisplay file list in archive.\n");
    printf("-x\tExtract all files to destination path.\n");
    printf("-pPwd\tSpecify password as 'Pwd'.\n");
}

int main(int argc, char* argv[])
{
    HRESULT ret = S_OK;

    COMMAND_LINE_INFO cli;
    if (1 < argc)
    {
        char currentPath[200];
#ifdef _WIN32
        GetCurrentDirectory(200, currentPath);
#else
        getcwd(currentPath, 200);
#endif

        for (int i = 1; (ret == S_OK) && (i < argc); i++)
        {
            if (argv[i][0] == '-')
            {
                switch (argv[i][1])
                {
                case 'h':
                case 'H':       cli.command = COMMAND_LINE_INFO::clHelp;        break;
                case 'v':
                case 'V':       cli.command = COMMAND_LINE_INFO::clVersion;     break;
                case 'l':
                case 'L':       cli.command = COMMAND_LINE_INFO::clList;        break;
                case 'x':
                case 'X':       cli.command = COMMAND_LINE_INFO::clExtract;     break;
                case 'p':
                case 'P':       cli.password = &argv[i][2];                     break;
                default:        ret = E_INVALIDARG;                             break;
                }
            }
            else
            {
                if (cli.archivePath.size() == 0)
                {
                    cli.archivePath = argv[i];
                }
                else if (cli.destination.size() == 0)
                {
                    cli.destination = argv[i];
                }
            }
        }

        if (cli.destination.size() == 0)
        {
            cli.destination = currentPath;
        }
    }

    if (ret == S_OK)
    {
        switch (cli.command)
        {
        case COMMAND_LINE_INFO::clExtract:
        case COMMAND_LINE_INFO::clList:
            if (cli.archivePath.size() == 0)
            {
                ret = E_INVALIDARG;
            }
            break;
        case COMMAND_LINE_INFO::clVersion:
        case COMMAND_LINE_INFO::clHelp:
            break;
        default:
            ret = E_INVALIDARG;
            break;
        }
    }

    if (ret == S_OK)
    {
        switch (cli.command)
        {
        case COMMAND_LINE_INFO::clExtract:
        case COMMAND_LINE_INFO::clList:         ret = CommandWithArchive(cli);      break;
        case COMMAND_LINE_INFO::clVersion:      ShowVersion();                      break;
        case COMMAND_LINE_INFO::clHelp:         ShowUsage();                        break;
        }
    }
    else if (ret == E_INVALIDARG)
    {
        printf("Invalid argumets\n\n");
        ShowUsage();
        ret = S_OK;
    }

    switch (ret)
    {
    case S_FALSE:
    case HRESULT_FROM_WIN32(ERROR_BAD_FORMAT):          ret = ercBadFormat;         break;
    case E_NOINTERFACE:                                 ret = ercNoInterface;       break;
    case HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND):      ret = ercFileNotFound;      break;
    case E_OUTOFMEMORY:                                 ret = ercOutOfMemory;       break;
    case HRESULT_FROM_WIN32(ERROR_HANDLE_EOF):          ret = ercEndOfFile;         break;
    case HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED):       ret = ercUnsupported;       break;
    case HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER):   ret = ercInvalidParameters; break;
    case HRESULT_FROM_WIN32(ERROR_INVALID_FLAGS):       ret = ercInvalidFlags;      break;
    }

    if ((ret != 0) && ((ret < START_ERROR_CODE) || (ercLast < ret)))
    {
        ret = ercUndefined;
    }

    return ret;
}
