#ifndef __EXPORTS_H__
#define __EXPORTS_H__

#include "common/IArchive.h"

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

HRESULT OpenArchive(const char* path, IInArchive** archive);
HRESULT ExtractArchive(IInArchive* archive, const char* destination, const char* password);

#endif
