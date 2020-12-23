#ifndef AZO_DECODER_DISTANCECODE_CPP
#define AZO_DECODER_DISTANCECODE_CPP

#include "../Common/MatchCodeTable.h"
#include "DistanceCodeD.h"

namespace AZO {
namespace Decoder {

inline u_int DistanceCode::Code(EntropyCode& entropy)
{
    u_int dist(0);

    if(history_.Code(entropy, dist) == false)
    {
        u_int idxCode = prob_.Code(entropy);
        
        dist = MATCH_DIST_CODE_TABLE[idxCode];
        if(MATCH_DIST_EXTRABIT_TABLE[idxCode]) {
            dist += entropy.Code(MATCH_DIST_EXTRABIT_TABLE[idxCode]);
        }

        history_.Add(dist);
    }  

    return dist;
}

} //namespaces Decoder
} //namespaces AZO

#endif /*AZO_DECODER_DISTANCECODE_CPP*/
