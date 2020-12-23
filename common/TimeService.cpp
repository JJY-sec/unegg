#include "StdAfx.h"
#include "TimeService.h"

void CTimeService::UnixTimeToFileTime(UINT unixTime, FILETIME& ft)
{
    /*
        UNIX 플랫폼의 파일 시간은 1970년 1월 1일 자정부터 경과된 초로 32비트 크기
        Win32 플랫폼의 파일 시간은 1601년 1월 1일 자정부터 경과된 100 나노초로 64비트 크기

        1970 - 1601 = 369년 동안 경과된 초는 11,636,784,000 초이지만
        윤년 규칙에 따라
        1604, 1608, ..., 1968 까지 총 92년에서
        1700년, 1800, 1900년 까지 총 3년을 뺀 89년이 윤년이 되므로
        89 * 24 * 60 * 60 = 7,689,600초를 더한 11,644,473,600초가 실제 경과된 초가 됩니다.

        나노초는 1 / 1,000,000,000 초이므로
        11,644,473,600,000,000,000 나노초이며, 이를 다시 100으로 나눈
        116,444,736,000,000,000를 Unix 시간에 더하면 Win32 플랫폼의 파일 시간을 구할 수 있습니다.

        참고
        https://support.microsoft.com/en-us/help/167296/how-to-convert-a-unix-time-t-to-a-win32-filetime-or-systemtime
        https://ko.wikipedia.org/wiki/윤년
    */

    LARGE_INTEGER li;
    li.QuadPart = Int32x32To64(unixTime, 10000000) + 116444736000000000;

    ft.dwLowDateTime = li.LowPart;
    ft.dwHighDateTime = li.HighPart;
}

BOOL CTimeService::MillisecondsToSystemTime(ULONGLONG ullMilliseconds, SYSTEMTIME& st)
{
    memset(&st, 0, sizeof(SYSTEMTIME));
    st.wHour = (WORD)(ullMilliseconds / 36000000);
    st.wMinute = (WORD)((ullMilliseconds % 36000000) / 60000);
    st.wSecond = (WORD)((ullMilliseconds % 60000) / 1000);
    st.wMilliseconds = (WORD)(ullMilliseconds % 1000);
    return TRUE;
}

#ifdef _WIN32
BOOL CTimeService::FileTimeToLocalSystemTime(const FILETIME& ftUTC, SYSTEMTIME& stLocal)
{
    FILETIME ftLocal;
    return FileTimeToLocalFileTime(&ftUTC, &ftLocal)
        && FileTimeToSystemTime(&ftLocal, &stLocal);
}
#endif
