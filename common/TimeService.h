#ifndef __EGG_TIME_SERVICE_H__
#define __EGG_TIME_SERVICE_H__

class CTimeService
{
public:
    static void UnixTimeToFileTime(UINT unixTime, FILETIME& ft);
    static BOOL MillisecondsToSystemTime(ULONGLONG ullMilliseconds, SYSTEMTIME& st);
#ifdef _WIN32
    static BOOL FileTimeToLocalSystemTime(const FILETIME& ftUTC, SYSTEMTIME& stLocal);
#endif

private:
    CTimeService();
};

#endif
