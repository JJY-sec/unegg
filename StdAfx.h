#ifndef __UNEGG_STDAFX_H__
#define __UNEGG_STDAFX_H__

#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <memory>

#include "lib/stringt/stringt.h"
#if UNICODE
typedef wstring tstring;
#else
typedef string tstring;
#endif

#ifdef _WIN32
#include <Windows.h>
#include <cguid.h>
#include <atlbase.h>

#include "lib/MyType.h"

#ifndef RINOK
#define RINOK(x) { HRESULT __result_ = (x); if (__result_ != S_OK) return __result_; }
#endif
#else
#include "lib/MyType.h"

#include "lib/MyCom.h"
#define CComPtr CMyComPtr

#include "lib/MyWindows.h"

#define HRESULT LONG

#define DECLSPEC_UUID(s)
#define __uuidof(id) IID_##id

#define FACILITY_WIN32                        7
constexpr inline HRESULT HRESULT_FROM_WIN32(unsigned int x)
{
    return (HRESULT)x > 0 ? ((HRESULT) ((x & 0x0000FFFF) | (FACILITY_WIN32 << 16) | 0x80000000)) : (HRESULT)x;
}

#define HRESULT_CODE(hr)    ((hr) & 0xFFFF)

#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)

#define ERROR_FILE_NOT_FOUND             2L
#define ERROR_INVALID_HANDLE             6L
#define ERROR_BAD_FORMAT                 11L
#define ERROR_HANDLE_EOF                 38L
#define ERROR_NOT_SUPPORTED              50L
#define ERROR_INVALID_PARAMETER          87L    // dderror
#define ERROR_BUFFER_OVERFLOW            111L
#define ERROR_INVALID_FLAGS              1004L
#define ERROR_CANCELLED                  1223L

#define FILE_ATTRIBUTE_READONLY             1
#define FILE_ATTRIBUTE_HIDDEN               2
#define FILE_ATTRIBUTE_SYSTEM               4
#define FILE_ATTRIBUTE_DIRECTORY           16
#define FILE_ATTRIBUTE_ARCHIVE             32
#define FILE_ATTRIBUTE_DEVICE              64
#define FILE_ATTRIBUTE_NORMAL             128
#define FILE_ATTRIBUTE_TEMPORARY          256
#define FILE_ATTRIBUTE_SPARSE_FILE        512
#define FILE_ATTRIBUTE_REPARSE_POINT     1024
#define FILE_ATTRIBUTE_COMPRESSED        2048
#define FILE_ATTRIBUTE_OFFLINE          0x1000
#define FILE_ATTRIBUTE_ENCRYPTED        0x4000
#define FILE_ATTRIBUTE_UNIX_EXTENSION   0x8000   /* trick for Unix */

#if UNICODE
typedef wchar_t TCHAR;
#define TEXT(s) L##s
#else
typedef char TCHAR;
#define TEXT(s) s
#endif

typedef char* LPSTR;
typedef const char* LPCSTR;

typedef wchar_t* LPWSTR;
typedef const wchar_t* LPCWSTR;

typedef TCHAR* LPTSTR;
typedef const TCHAR* LPCTSTR;

#if UNICODE
#define _tcsrchr        wcsrchr
#else
#define _tcsrchr        strrchr
#endif

template<typename T>
T InterlockedIncrement(T* p)
{
    ++(*p);
    return *p;
}

template<typename T>
T InterlockedDecrement(T* p)
{
    --(*p);
    return *p;
}

typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME;

#define Int32x32To64(a, b)  ((LONGLONG)(((LONGLONG)((long)(a))) * ((long)(b))))
#define UInt32x32To64(a, b) ((ULONGLONG)(((ULONGLONG)((unsigned int)(a))) * ((unsigned int)(b))))
#endif

#endif
