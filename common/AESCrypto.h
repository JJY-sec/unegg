#ifndef __ALEGG_AES_CRYPTO_H__
#define __ALEGG_AES_CRYPTO_H__

#include "ICrypto.h"
#include "UnknownImpl.h"

#include "../lib/aes/fileenc.h"

class CAESDecryptor : public TUnknown<IDecryptor>
{
public:
    enum AESMode
    {
        aes128 = 1,
        aes196 = 2,
        aes256 = 3
    };

    CAESDecryptor(int mode, const char* password, const unsigned char* salt);
    virtual ~CAESDecryptor();

#ifndef _WIN32
START_QUERYINTERFACE
    QUERY_UNKNOWN(IDecryptor)
    QUERY_INTERFACE(IDecryptor)
END_QUERYINTERFACE
#endif

    // IDecryptor
    virtual HRESULT Decrypt(LPBYTE buffer, UINT32 size);

private:
    int mode_;
    fcrypt_ctx ctx_;
};

#endif
