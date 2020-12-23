#ifndef AZO_DECODER_DISTANCECODE_H
#define AZO_DECODER_DISTANCECODE_H

#include "HistoryListD.h"
#include "EntropyBitProbD.h"

namespace AZO {
namespace Decoder {

class DistanceCode
{
public:
    DistanceCode() : 
        history_(MATCH_MIN_DIST) {}
    
    u_int Code(EntropyCode& entropy);

private:
    HistoryList<u_int, DISTANCE_HISTORY_SIZE>    history_;     
    EntropyBitProb<MATCH_DIST_CODE_SIZE>            prob_;
};

} //namespaces Decoder
} //namespaces AZO

#include "DistanceCodeD.cpp"

#endif /*AZO_DECODER_DISTANCECODE_H*/
