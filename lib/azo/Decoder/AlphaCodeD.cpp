#ifndef AZO_DECODER_ALPHACODE_CPP
#define AZO_DECODER_ALPHACODE_CPP

#include "AlphaCodeD.h"

namespace AZO {
namespace Decoder {

inline byte AlphaCode::Code(EntropyCode& entropy, u_int pos, byte pre)
{
    UNUSED_ARG(pos);

    return static_cast<byte>(alphaProb_.Code(entropy, pre));
}

} //namespaces Decoder
} //namespaces AZO

#endif /*AZO_DECODER_ALPHACODE_CPP*/
