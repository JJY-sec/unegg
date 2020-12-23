#ifndef __EGG_STRUCTURE_H__
#define __EGG_STRUCTURE_H__

namespace NArchive
{
    namespace NEgg
    {
        enum FormatHeaderSignature
        {
            fhsEgg = 0x41474745,
            fhsSplit = 0x24F5A262,
            fhsSolid = 0x24E5A060,
            //fhsGlobalEncryptHeader      = 0x08D144A8,       // 사용되지 않습니다.
            fhsFile = 0x0A8590E3,
            fhsFilename = 0x0A8591AC,
            fhsComment = 0x04C63672,
            fhsWindowsFileInfo = 0x2C86950B,
            //fhsPosixFileInfo            = 0x1EE922E5,
            fhsEncrypt = 0x08D1470F,
            fhsBlock = 0x02B50C13,
            fhsDummy = 0x07463307,
            fhsSkip = 0xFFFF0000,

            fhsEnd = 0x08E28222,

            fhsLast = 0x00000000
        };

        struct HEADER
        {
            UInt16 version;
            UInt32 program;
            UInt32 reserved;
        };

        struct SPLIT_INFO
        {
            UInt32 previous;
            UInt32 next;
        };

        struct SOLID_INFO
        {
        };

        struct ENCRYPT_INFO
        {
            enum EncryptionMethod
            {
                emZipCrypto, emAES128, emAES256,
                emDeprecated1, emDeprecated2,
                emLEA128, emLEA256
            };
            BYTE encryptionMethod;

            union
            {
                struct
                {
                    BYTE verifyData[12];
                    UInt32 crc;
                } zipCrypto;

                struct
                {
                    BYTE header[10];
                    BYTE footer[10];
                } aes_lea_128;

                struct
                {
                    BYTE header[18];
                    BYTE footer[10];
                } aes_lea_256;
            } data;
        };

        struct FILE_INFO
        {
            UInt32 index;
            UInt64 size;
        };

        struct FILENAME_INFO
        {
            std::basic_string<WCHAR> path;
        };

        struct COMMENT_INFO
        {
            std::basic_string<WCHAR> comment;
        };

        struct WINDOWS_FILE_INFO
        {
            UInt64 lastModified;
            DWORD attributes;
        };

        // TODO: 분석 후 추후 도입
        //struct POSIX_FILE_INFO : EXTRA_FIELD<UInt16>
        //{
        //    UInt32 mode;
        //    UInt32 userId;
        //    UInt32 groupId;
        //    UInt64 lastModified;
        //};

        struct BLOCK_INFO
        {
            enum CompressMethod
            {
                cmStore, cmDeflate, cmBzip2, cmAZO, cmLZMA
            };
            BYTE compressMethod;
            BYTE compressMethodHint;
            UInt32 unpackSize;
            UInt32 packSize;
            UInt32 crc;
        };

        struct DUMMY
        {
            UInt32 size;
        };

    }
}

#endif
