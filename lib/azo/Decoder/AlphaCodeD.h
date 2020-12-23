#ifndef AZO_DECODER_ALPHACODE_H
#define AZO_DECODER_ALPHACODE_H

#include "PredictProbD.h"

namespace AZO {
namespace Decoder {

class AlphaCode
{
public:
    AlphaCode() {}

    byte Code(EntropyCode& entropy, u_int pos, byte pre);

private:    
    PredictProb<ALPHA_SIZE, ALPHA_SIZE, ALPHACODE_PREDICT_SHIFT> alphaProb_;
};

} //namespaces Decoder
} //namespaces AZO

#include "AlphaCodeD.cpp"

#endif /*AZO_DECODER_ALPHACODE_H*/
