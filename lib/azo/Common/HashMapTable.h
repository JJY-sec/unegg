#ifndef AZO_HASHMAPTABLE_H
#define AZO_HASHMAPTABLE_H

#include "AZOPrivate.h"

namespace AZO {

class HashMapTable
{
public:
    HashMapTable(const byte* buf, u_int size);
    ~HashMapTable();

    static const uint32_t INVALID_POS = static_cast<uint32_t>(-1);
  
public:
    void SetMapOption(byte* buf, u_int hashLevel, u_int hashSize);
    static u_int GetRequiredBufSize(u_int hashLevel, u_int hashSize) {
        return (hashLevel << hashSize) * sizeof(uint32_t);
    }

    u_int GetLevelSize() { return matchHashLevel_; }
    u_int GetHashLen(u_int level);
    void UpdateMatchMaps(u_int pos);
    u_int GetMatchPos(u_int level);

private:
    void CalcHash(const byte*& buf, u_int size, u_int& hash);
    void CalcHash2(const byte*& buf, u_int& hash);
    void CalcHash4(const byte*& buf, u_int size, u_int& hash);
    u_int GetAdjustHashValue(u_int hash);

    void SetHashKey(u_int pos);
    void UpdateMap(u_int pos);

private:
    const byte* buf_;
    u_int bufSize_;
    
    u_int    hashKey_[8];
    u_int    scanPos_;

private:
    u_int matchHashLevel_;
    u_int matchHashSize_;

    uint32_t* matchTable_;
};

} //namespaces AZO

#include "HashMapTable.cpp"

#endif /*AZO_HASHMAPTABLE_H*/
