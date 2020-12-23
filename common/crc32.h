#ifndef __CRC32_H__
#define __CRC32_H__

#include "../lib/MyType.h"

#if __cplusplus
extern "C" {
#endif

extern UInt32 g_crc32Table[256];

inline void CRC32Init(UInt32 *pCrc32)
{
    *pCrc32 = 0xFFFFFFFF;
}

inline void CRC32UpdateByte(UInt32 *pCrc32, unsigned char data)
{
    *pCrc32 = g_crc32Table[(data ^ (*pCrc32)) & 0xFF] ^ ((*pCrc32) >> 8);
}

inline void CRC32Update(UInt32 *pCrc32, const unsigned char* lpData, const UInt32 uSize)
{
    UInt32 i = 0;

    for (i = 0; i < uSize; i++)
    {
        CRC32UpdateByte(pCrc32, lpData[i]);
    }
}

inline void CRC32Finish(UInt32 *pCrc32)
{
    *pCrc32 = ~(*pCrc32);
}

#if __cplusplus
}
#endif

#endif
