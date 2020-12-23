#ifndef AZO_PROBRATIO_H
#define AZO_PROBRATIO_H

#include "AZOPrivate.h"
//#define _USE_DOUBLE_RATIO

namespace AZO {

class ProbRatio
{
#ifdef _USE_DOUBLE_RATIO
    typedef double PROBTYPE;
#else
    typedef uint64_t PROBTYPE;
#endif /*_USE_DOUBLE_RATIO*/

public:
    ProbRatio();
    ProbRatio(const ProbRatio& v);
    ProbRatio(PROBTYPE prob, u_int totalBit);

    ProbRatio& operator*=(const ProbRatio& v);
  
private:
    void Add(PROBTYPE prob, u_int totalBit);

private: 
    PROBTYPE prob_;
    u_int totalBit_;

public:
    friend bool operator>(const ProbRatio& v1, const ProbRatio& v2);
    friend bool operator<(const ProbRatio& v1, const ProbRatio& v2);
    friend ProbRatio operator*(const ProbRatio& v1, const ProbRatio& v2);
};

} //namespaces AZO

#include "ProbRatio.cpp"

#endif /*AZO_PROBRATIO_H*/
