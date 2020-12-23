#ifndef AZO_DECODER_BOOLSTATE_H
#define AZO_DECODER_BOOLSTATE_H

#include "../Base/BoolStateB.h"
#include "EntropyCodeD.h"

namespace AZO {
namespace Decoder {

template <u_int N = 8>
class BoolState : Base::BoolState<N>
{
    typedef Base::BoolState<N> super;
public:
    BoolState() {}
    ~BoolState() {}
    
    BOOL Code(EntropyCode& entropy);

private:
    using super::GetProb;
    using super::Update;
    using super::TotalBit;
};

} //namespaces Decoder
} //namespaces AZO

#include "BoolStateD.cpp"

#endif /*AZO_DECODER_BOOLSTATE_H*/
