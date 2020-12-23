#include "StdAfx.h"
#include "AESCrypto.h"

CAESDecryptor::CAESDecryptor(int mode, const char* password, const unsigned char* salt)
    : mode_(mode)
{
    unsigned char pwdVerifier[PWD_VER_LENGTH];
    const unsigned char* pwd = reinterpret_cast<const unsigned char*>(password);

    if ((fcrypt_init(mode_, pwd, strlen(password), salt, pwdVerifier, &ctx_) != GOOD_RETURN)
        || (memcmp(salt + SALT_LENGTH(mode_), pwdVerifier, PWD_VER_LENGTH) != 0))
    {
        throw std::runtime_error("Wrong Password");
    }
}

CAESDecryptor::~CAESDecryptor()
{
    if ((0 < mode_) && (mode_ < 4))
    {
        unsigned char mac[MAC_LENGTH(mode_)];
        fcrypt_end(mac, &ctx_);
    }
}

HRESULT CAESDecryptor::Decrypt(LPBYTE buffer, UINT32 size)
{
    fcrypt_decrypt(buffer, size, &ctx_);
    return S_OK;
}
