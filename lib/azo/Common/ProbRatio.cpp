#ifndef AZO_PROBRATIO_CPP
#define AZO_PROBRATIO_CPP

#include <limits>
#include "ProbRatio.h"

namespace AZO {

inline ProbRatio::ProbRatio() : 
    prob_(1), totalBit_(0)
{   
}

inline ProbRatio::ProbRatio(const ProbRatio& v) : 
    prob_(v.prob_), totalBit_(v.totalBit_)
{
}

inline ProbRatio::ProbRatio(PROBTYPE prob, u_int total) : 
    prob_(prob), totalBit_(total)
{    
}

inline void ProbRatio::Add(PROBTYPE prob, u_int total)
{
#ifdef _USE_DOUBLE_RATIO
    prob_ *= prob / (1<<total);
#else
    const u_int BITMAX = std::numeric_limits<PROBTYPE>::digits/2;

    //ASSERT(0 < prob && prob <= (1ull<<total));
    ASSERT(0 < prob && prob < (1ull<<BITMAX));

    totalBit_ += total;
    prob_ *= prob;
    if(totalBit_ > BITMAX) {
        prob_ >>= (totalBit_ - BITMAX);
        totalBit_ = BITMAX;
        if(prob_ == 0) prob_ = 1;
    }

    //ASSERT(0 < prob_ && prob_ <= (1ull<<totalBit_));
    ASSERT(0 < prob_ && prob_ < (1ull<<BITMAX));
    ASSERT(totalBit_ <= std::numeric_limits<PROBTYPE>::digits);

#endif /*_USE_DOUBLE_RATIO*/
}

inline ProbRatio& ProbRatio::operator*=(const ProbRatio& v)
{
    Add(v.prob_, v.totalBit_);

    return *this;
}

inline bool operator>(const ProbRatio& v1, const ProbRatio& v2)
{
#ifdef _USE_DOUBLE_RATIO
    return v1.prob_ > v2.prob_;
#else
    if(v1.totalBit_ > v2.totalBit_) {
        return v1.prob_ >> (v1.totalBit_ - v2.totalBit_) > v2.prob_;
    } else {
        return v1.prob_ > v2.prob_ >> (v2.totalBit_ - v1.totalBit_);
    }
#endif /*_USE_DOUBLE_RATIO*/
}

inline bool operator<(const ProbRatio& v1, const ProbRatio& v2)
{
#ifdef _USE_DOUBLE_RATIO
    return v1.prob_ < v2.prob_;
#else
    if(v1.totalBit_ > v2.totalBit_) {
        return v1.prob_ >> (v1.totalBit_ - v2.totalBit_) < v2.prob_;
    } else {
        return v1.prob_ < v2.prob_ >> (v2.totalBit_ - v1.totalBit_);
    }
#endif /*_USE_DOUBLE_RATIO*/
}

inline ProbRatio operator*(const ProbRatio& v1, const ProbRatio& v2)
{
    ProbRatio ret(v1);
    return (ret *= v2);
}

} //namespaces AZO

#endif /*AZO_PROBRATIO_CPP*/
