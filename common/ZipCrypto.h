#ifndef __ALEGG_ZIP_CRYPTO_H__
#define __ALEGG_ZIP_CRYPTO_H__

#include "ICrypto.h"
#include "UnknownImpl.h"

class CZipDecryptor : public TUnknown<IDecryptor>
{
public:
    CZipDecryptor(const char* password);
    virtual ~CZipDecryptor();

#ifndef _WIN32
START_QUERYINTERFACE
    QUERY_UNKNOWN(IDecryptor)
    QUERY_INTERFACE(IDecryptor)
END_QUERYINTERFACE
#endif

    // IDecryptor
    virtual HRESULT Decrypt(LPBYTE buffer, UINT32 size);

    void RestoreKeys();

private:
    UINT32 keys_[3];
    UINT32 initKeys_[3];

    void UpdateKeys(unsigned char b);
    unsigned char GetDecryptByte();
};

#endif
