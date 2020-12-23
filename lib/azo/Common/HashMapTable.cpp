#ifndef AZO_HASHMAPTABLE_CPP
#define AZO_HASHMAPTABLE_CPP

#ifndef _WIN32
#include <string.h>
#endif

#include <algorithm>
#include <limits>
#include "HashMapTable.h"
#include "Allocator.h"

namespace AZO {
    
template <u_int N> struct HashLength {
    static const u_int l = HashLength<N-1>::l + (1u<<N);
};

template <> struct HashLength<0> {
    static const u_int l = MATCH_MIN_LENGTH;
};

static const u_int HASH_LENGTH[] = {
    HashLength<0>::l,
    HashLength<1>::l,
    HashLength<2>::l,
    HashLength<3>::l,
    HashLength<4>::l,
    HashLength<5>::l,
    HashLength<6>::l,
    HashLength<7>::l,
    HashLength<8>::l,
    HashLength<9>::l,    
};

inline HashMapTable::HashMapTable(const byte* buf, u_int size) : 
    buf_(buf), bufSize_(size), scanPos_(0),
    matchHashLevel_(0), matchHashSize_(0), matchTable_(NULL)
{
}

inline HashMapTable::~HashMapTable()
{
}

inline void HashMapTable::SetMapOption(byte* buf, u_int hashLevel, u_int hashSize)
{
    ASSERT(buf);
    ASSERT(hashLevel < 8);
    ASSERT(hashLevel == 0 || hashSize >= 16);

    matchTable_ = reinterpret_cast<uint32_t*>(buf);
    matchHashLevel_ = hashLevel;
    matchHashSize_ = hashSize;    
    
    const u_int s = (matchHashLevel_ << matchHashSize_);
    const uint32_t initValue = INVALID_POS;
    std::fill_n(matchTable_, static_cast<size_t>(s), initValue);

    std::fill_n(hashKey_, static_cast<size_t>(matchHashLevel_), 0);

    SetHashKey(scanPos_);
}

inline u_int HashMapTable::GetHashLen(u_int level)
{
    ASSERT(level < matchHashLevel_);

    return HASH_LENGTH[level];
}

inline void HashMapTable::CalcHash(const byte*& buf, u_int size, u_int& hash)
{
    for(size_t i=0; i<size; ++i)
    {
        const char c = static_cast<char>(*buf++);
        hash = ((hash << 5) + hash) + c;
    }
}

inline void HashMapTable::CalcHash2(const byte*& buf, u_int& hash)
{
    const short c = *reinterpret_cast<const short*>(buf);
    hash = ((hash << 5) + hash) + c;
    buf += 2;
}

inline void HashMapTable::CalcHash4(const byte*& buf, u_int size, u_int& hash)
{
    for(size_t i=0; i<(size >> 2); ++i)
    {
        const int c = *reinterpret_cast<const int*>(buf);
        hash = ((hash << 5) + hash) + c;
        buf += 4;
    }
}

inline u_int HashMapTable::GetAdjustHashValue(u_int hash)
{
    return (hash ^ (hash >> matchHashSize_)) & ((1u<<matchHashSize_)-1);
}

inline void HashMapTable::SetHashKey(u_int pos)
{
    static const u_int HASH_LENGTH_DIFF[] = {
        HASH_LENGTH[0] - 2,
        HASH_LENGTH[1] - HASH_LENGTH[0],
        HASH_LENGTH[2] - HASH_LENGTH[1],
        HASH_LENGTH[3] - HASH_LENGTH[2],
        HASH_LENGTH[4] - HASH_LENGTH[3],
        HASH_LENGTH[5] - HASH_LENGTH[4],
        HASH_LENGTH[6] - HASH_LENGTH[5],
        HASH_LENGTH[7] - HASH_LENGTH[6],
        HASH_LENGTH[8] - HASH_LENGTH[7],
        HASH_LENGTH[9] - HASH_LENGTH[8],
    };

    const byte* buf = buf_+pos;
    const byte* const end = buf_+bufSize_;

    //uint32_t hash(5381);
    u_int hash = (buf[0]) << 8 | (buf[1]); buf += 2;
/*
    for(u_int i=0; i<matchHashLevel_; ++i)
    {        
        const u_int incLen = HASH_LENGTH_DIFF[i]; 
        if(buf + HASH_LENGTH_DIFF[i] > end) {
            break;
        }

        ASSERT(buf + incLen <= buf_+bufSize_);

        CalcHash(buf, incLen, hash);
        //hashKey_[i] = (hash & (MATCH_HASH_SIZE-1)) ^ (hash / MATCH_HASH_SIZE);
        hashKey_[i] = (hash ^ (hash >> matchHashSize_)) & ((1u<<matchHashSize_)-1);
        //hashKey_[i] = hash & ((1u<<MATCH_HASH_BITSIZE)-1);
    }
 */

    ASSERT(matchHashLevel_ >= 1);
     
    hashKey_[0] = GetAdjustHashValue(hash);
    
    CalcHash2(buf, hash);
    hashKey_[1] = GetAdjustHashValue(hash);

    for(u_int i=2; i<matchHashLevel_; ++i)
    {        
        const u_int incLen = HASH_LENGTH_DIFF[i]; 
        if(buf + HASH_LENGTH_DIFF[i] > end) {
            break;
        }

        ASSERT(buf + incLen <= buf_+bufSize_);
    
        ASSERT(incLen % 4 == 0);
        CalcHash4(buf, incLen, hash);
        
        hashKey_[i] = GetAdjustHashValue(hash);
    }
}

inline void HashMapTable::UpdateMap(u_int pos)
{
    for(u_int i=0; i<matchHashLevel_; ++i)
    {
        matchTable_[(i<<matchHashSize_) + hashKey_[i]] = static_cast<uint32_t>(pos);
    }
}


inline void HashMapTable::UpdateMatchMaps(u_int pos)
{
    while(scanPos_ < pos)
    {
        UpdateMap(scanPos_);
        scanPos_++;

        //if(scanPos_ + GetHashLen(MATCH_HASH_LEVEL) > bufSize_) {
        //    break;
        //}

        SetHashKey(scanPos_);
    }
/*
    UpdateMap(scanPos_);
    scanPos_ = pos;
    SetHashKey(scanPos_);
*/
    ASSERT(scanPos_ == pos);
}

inline u_int HashMapTable::GetMatchPos(u_int level)
{
    ASSERT(hashKey_[level] < 1u<<matchHashSize_);
    ASSERT(matchTable_);

    const u_int pos = matchTable_[(level<<matchHashSize_) + hashKey_[level]];
    if(pos == INVALID_POS)
        return pos;

    const u_int len = GetHashLen(level);

    if(scanPos_ + len > bufSize_)
        return INVALID_POS;
    
    if(len == 2)
        return pos;
    
    ASSERT(len >= 4);

    if(buf_[pos + len-1] != buf_[scanPos_ + len-1] || 
       buf_[pos + len-2] != buf_[scanPos_ + len-2]/* ||
       buf_[pos + len-3] != buf_[scanPos_ + len-3] ||
       buf_[pos + len-4] != buf_[scanPos_ + len-4]*/)
    //if(*reinterpret_cast<const uint32_t*>(buf_ + pos + len-4) 
    //    != *reinterpret_cast<const  uint32_t*>(buf_ + scanPos_ + len-4))
        return INVALID_POS;

    if(memcmp(buf_+scanPos_, buf_+pos, static_cast<size_t>(len-2)))
        return INVALID_POS;

    return pos;
}


} //namespaces AZO

#endif /*AZO_HASHMAPTABLE_CPP*/
