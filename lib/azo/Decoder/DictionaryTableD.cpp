#ifndef AZO_DECODER_DICTIONARYCODE_CPP
#define AZO_DECODER_DICTIONARYCODE_CPP

#include "DictionaryTableD.h"

namespace AZO {
namespace Decoder {

inline bool DictionaryTable::Code(EntropyCode& entropy, u_int& pos, u_int& length)
{
    if(findState_.Code(entropy))
    {
        u_int n = prob_.Code(entropy);
        if(n >= N) {
            return false;
        }

        Get(n, pos, length);

        Update(pos, length, n);

        return true;
    }
    else
    {
        return false;
    }
}

inline void DictionaryTable::Add(u_int pos, u_int length)
{
    Update(pos, length);
}

inline void DictionaryTable::Get(u_int idx, u_int& pos, u_int& len)
{
    ASSERT(idx < N);

    len = data_[idx].len;
    pos = data_[idx].pos;
}

} //namespaces Decoder
} //namespaces AZO

#endif /*AZO_DECODER_DICTIONARYCODE_CPP*/
