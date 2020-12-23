#ifndef __ALEGG_LEA_CRYPTO_H__
#define __ALEGG_LEA_CRYPTO_H__

#include "ICrypto.h"
#include "UnknownImpl.h"

#pragma warning(push)
#pragma warning(disable:4819)
#include "../lib/lea/lea.h"
#pragma warning(pop)

class CLEADecryptor : public TUnknown<IDecryptor>
{
public:
    enum LEAMode
    {
        lea128 = 1,
        lea196 = 2,
        lea256 = 3
    };

    CLEADecryptor(int mode, const char* password, const unsigned char* salt);
    virtual ~CLEADecryptor();

#ifndef _WIN32
START_QUERYINTERFACE
    QUERY_UNKNOWN(IDecryptor)
    QUERY_INTERFACE(IDecryptor)
END_QUERYINTERFACE
#endif

    // IDecryptor
    virtual HRESULT Decrypt(LPBYTE buffer, UINT32 size);

private:
    LPBYTE GetTmpBuffer(UINT32 minSize);

    lea_online_ctx ctx_;
    std::vector<BYTE> tmpBuffer_;
};

#endif
