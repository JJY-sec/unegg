#ifndef AZO_AZO_H
#define AZO_AZO_H

const int AZO_VERSION       (0x100);

// return error
const int AZO_OK                    (0);
const int AZO_STREAM_END            (1);
const int AZO_PARAM_ERROR           (-1);
const int AZO_MEM_ERROR             (-2);
const int AZO_OUTBUFF_FULL          (-3);
const int AZO_DATA_ERROR            (-4);
const int AZO_DATA_ERROR_VERSON     (-5);
const int AZO_DATA_ERROR_BLOCKSIZE  (-6);


typedef void* (*AZO_ALLOC_FUNCTON)(void*, unsigned int);
typedef void (*AZO_FREE_FUNCTON)(void*, void*);

// struct
struct AZO_CompressOption
{    
    unsigned int dictionarySize;
    unsigned int matchHashLevel;
    unsigned int matchHashSize;
    unsigned int minProbCalcSize;
};

const AZO_CompressOption AZO_COMPRESS_LEVEL[] = 
{
    // dictionary size , hash level, hash size, prob size
    { 19, 2, 16, 2 }, //0
    { 19, 3, 18, 2 }, //1
    { 19, 3, 19, 2 }, //2
    { 20, 4, 20, 3 }, //3
    { 21, 4, 20, 4 }, //4
    { 21, 5, 21, 4 }, //5
    { 21, 5, 21, 5 }, //6
    { 21, 5, 22, 5 }, //7
    { 22, 6, 22, 5 }, //8
    { 22, 6, 23, 5 }, //9

    // compress memory size = 2^(dictionarySize+1) + (2^(matchHashSize+2)) * matchHashLevel
    // decompress memory size = 2^(dictionarySize+1)
};


struct AZO_Stream 
{
    const char* next_in;
    unsigned int avail_in;

    char* next_out;
    unsigned int avail_out;
};

// handle
typedef void*   AZO_HCOMPRESS;
typedef void*   AZO_HDECOMPRESS;


// init
int AZO_Version();

void AZO_Allocator(AZO_ALLOC_FUNCTON allocFunc, 
                   AZO_FREE_FUNCTON freeFunc, 
                   void* opaque);

//Compress
int AZO_CompressInit(AZO_HCOMPRESS* pHandle /*OUT*/, 
                     const AZO_CompressOption* option);

int AZO_Compress(AZO_HCOMPRESS handle, 
                 AZO_Stream* stream /*INOUT*/,
                 int end);

int AZO_CompressEnd(AZO_HCOMPRESS handle);


//Decompress
int AZO_DecompressInit(AZO_HDECOMPRESS* pHandle /*OUT*/);

int AZO_Decompress(AZO_HDECOMPRESS handle,
                   AZO_Stream* stream /*INOUT*/);

int AZO_DecompressEnd(AZO_HDECOMPRESS handle);


//Utility
int AZO_BufferCompress(char* dest,
                       unsigned int* destLen,
                       const char* source,
                       unsigned int sourceLen,
                       const AZO_CompressOption* option);


int AZO_BufferDecompress(char* dest,
                        unsigned int* destLen,
                        const char* source,
                        unsigned int sourceLen);

#endif /*AZO_AZO_H*/
