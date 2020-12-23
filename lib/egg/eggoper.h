#ifndef __EGG_OPERATOR_H__
#define __EGG_OPERATOR_H__

#include "../../common/seven_stream.h"
#include "eggstruct.h"

namespace NArchive
{
    namespace NEgg
    {
        seven_istream& operator >> (seven_istream& _Istr, HEADER& p);
        seven_istream& operator >> (seven_istream& _Istr, SPLIT_INFO& p);
        seven_istream& operator >> (seven_istream& _Istr, SOLID_INFO& p);
        seven_istream& operator >> (seven_istream& _Istr, ENCRYPT_INFO& p);
        seven_istream& operator >> (seven_istream& _Istr, FILE_INFO& p);
        seven_istream& operator >> (seven_istream& _Istr, FILENAME_INFO& p);
        seven_istream& operator >> (seven_istream& _Istr, COMMENT_INFO& p);
        seven_istream& operator >> (seven_istream& _Istr, WINDOWS_FILE_INFO& p);
        seven_istream& operator >> (seven_istream& _Istr, BLOCK_INFO& p);
        seven_istream& operator >> (seven_istream& _Istr, DUMMY& p);
    }
}

#endif
