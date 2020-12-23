#ifndef AZO_Decoder_BoolState_CPP
#define AZO_Decoder_BoolState_CPP

#include "BoolStateD.h"

namespace AZO {
namespace Decoder {

template <u_int N>
inline BOOL BoolState<N>::Code(EntropyCode& entropy)
{
    BOOL b = entropy.Code(GetProb(), TotalBit());
    Update(b);

    return b;
}

} //namespaces Decoder
} //namespaces AZO

#endif /*AZO_Decoder_BoolState_CPP*/
