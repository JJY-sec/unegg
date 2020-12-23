#include "StdAfx.h"
#include "eggoper.h"

namespace NArchive
{
    namespace NEgg
    {
        template<unsigned char f, typename size_type>
        struct EXTRA_COMMON_FIELD
        {
            const static unsigned char flags;
            size_type size;

            EXTRA_COMMON_FIELD(size_type s = 0)
                : size(s) {}
        };

        template<unsigned char f, typename size_type>
        const unsigned char EXTRA_COMMON_FIELD<f, size_type>::flags = f;

        typedef EXTRA_COMMON_FIELD<0, UInt16> EXTRA_FIELD_2BYTE;
        typedef EXTRA_COMMON_FIELD<1, UInt32> EXTRA_FIELD_4BYTE;

        template<unsigned char f, typename size_type>
        seven_istream& operator >> (seven_istream& _Istr, EXTRA_COMMON_FIELD<f, size_type>& field)
        {
            unsigned char c;
            _Istr >> c >> field.size;
            if (c != field.flags)
            {
                throw operation_exception(HRESULT_FROM_WIN32(ERROR_INVALID_FLAGS));
            }
            return _Istr;
        }

        seven_istream& operator >> (seven_istream& _Istr, HEADER& p)
        {
            _Istr >> p.version >> p.program >> p.reserved;
            return _Istr;
        }

        seven_istream& operator >> (seven_istream& _Istr, SPLIT_INFO& p)
        {
            EXTRA_FIELD_2BYTE f;
            _Istr >> f >> p.previous >> p.next;
            return _Istr;
        }

        seven_istream& operator >> (seven_istream& _Istr, SOLID_INFO& /*p*/)
        {
            EXTRA_FIELD_2BYTE f;
            _Istr >> f;
            return _Istr;
        }

        seven_istream& operator >> (seven_istream& _Istr, ENCRYPT_INFO& p)
        {
            EXTRA_FIELD_2BYTE f;
            _Istr >> f >> p.encryptionMethod;
            switch (p.encryptionMethod)
            {
            case ENCRYPT_INFO::emZipCrypto:   f.size = 17;    break;
            case ENCRYPT_INFO::emAES128:
            case ENCRYPT_INFO::emLEA128:      f.size = 21;    break;
            case ENCRYPT_INFO::emAES256:
            case ENCRYPT_INFO::emLEA256:      f.size = 29;    break;
            default:
                throw operation_exception(HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER));
                break;
            }

            UInt32 dataSize = f.size - 1;
            UInt32 processedSize;
            HRESULT result = _Istr.read((void*)(&p.data), dataSize, &processedSize);
            if ((result == S_OK) && (dataSize != processedSize))
            {
                result = HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
            }

            if (result != S_OK)
            {
                throw operation_exception(result);
            }

            return _Istr;
        }

        seven_istream& operator >> (seven_istream& _Istr, FILE_INFO& p)
        {
            _Istr >> p.index >> p.size;
            return _Istr;
        }

        seven_istream& operator >> (seven_istream& _Istr, FILENAME_INFO& p)
        {
            EXTRA_FIELD_2BYTE f;
            _Istr >> f;

            LPSTR readString = new char[(UInt32)f.size + 1];
            UInt32 processedSize;
            HRESULT result = _Istr.read(readString, f.size, &processedSize);
            if (result == S_OK)
            {
                if (f.size == processedSize)
                {
                    readString[f.size] = 0;
                    {
                        LPSTR pszTemp;
#ifdef _WIN32
                        while ((pszTemp = strchr(readString, '/')) != NULL)
                        {
                            *pszTemp = '\\';
                        }
#else
                        while ((pszTemp = strchr(readString, '\\')) != NULL)
                        {
                            *pszTemp = '/';
                        }
#endif
                    }
                    p.path = wstring().fromutf8(readString);
                }
                else
                {
                    result = HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
                }
            }
            delete[] readString;

            if (result != S_OK)
            {
                throw operation_exception(result);
            }

            return _Istr;
        }

        seven_istream& operator >> (seven_istream& _Istr, COMMENT_INFO& p)
        {
            EXTRA_FIELD_2BYTE f;
            _Istr >> f;

            LPSTR readString = new char[(UInt32)f.size + 1];
            UInt32 processedSize;
            HRESULT result = _Istr.read(readString, f.size, &processedSize);
            if (result == S_OK)
            {
                if (f.size == processedSize)
                {
                    readString[f.size] = 0;
                    p.comment = wstring().fromutf8(readString);
                }
                else
                {
                    result = HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
                }
                
            }
            delete[] readString;

            if (result != S_OK)
            {
                throw operation_exception(result);
            }

            return _Istr;
        }

        enum WindowsFileAttributes
        {
            wfaReadonly = 0x01,
            wfaHidden = 0x02,
            wfaSystem = 0x04,
            wfaSymbolicLink = 0x08,
            wfaDirectory = 0x80,

            wfaLast = 0x00
        };

        seven_istream& operator >> (seven_istream& _Istr, WINDOWS_FILE_INFO& p)
        {
            EXTRA_FIELD_2BYTE f;
            unsigned char attributes;
            _Istr >> f >> p.lastModified >> attributes;

            p.attributes = 0;
            if (attributes & wfaReadonly)
            {
                p.attributes |= FILE_ATTRIBUTE_READONLY;
            }
            if (attributes & wfaHidden)
            {
                p.attributes |= FILE_ATTRIBUTE_HIDDEN;
            }
            if (attributes & wfaSystem)
            {
                p.attributes |= FILE_ATTRIBUTE_SYSTEM;
            }
            if (attributes & wfaDirectory)
            {
                p.attributes |= FILE_ATTRIBUTE_DIRECTORY;
            }

            return _Istr;
        }

        seven_istream& operator >> (seven_istream& _Istr, BLOCK_INFO& p)
        {
            _Istr >> p.compressMethod >> p.compressMethodHint >> p.unpackSize >> p.packSize >> p.crc;
            return _Istr;
        }

        seven_istream& operator >> (seven_istream& _Istr, DUMMY& /*p*/)
        {
            EXTRA_FIELD_2BYTE f;
            _Istr >> f;
            _Istr.seek(f.size, SEEK_CUR, NULL);
            return _Istr;
        }
    }
}
