#include "StdAfx.h"
#include "LEACrypto.h"

#include "../lib/aes/fileenc.h"

CLEADecryptor::CLEADecryptor(int mode, const char* password, const unsigned char* salt)
{
    const unsigned char* pwd = reinterpret_cast<const unsigned char*>(password);

    unsigned int keySize = KEY_LENGTH(mode);
    unsigned int saltSize = SALT_LENGTH(mode);
    unsigned char keyBuffer[66];
    derive_key(pwd, strlen(password), salt, saltSize, 1000, keyBuffer, keySize * 2 + PWD_VER_LENGTH);

    unsigned char iv[16] = { 0, };
    if ((lea_online_init(&ctx_, LEA_CTR_DEC, iv, keyBuffer, keySize) < 0)
        || (memcmp(salt + saltSize, &keyBuffer[keySize * 2], PWD_VER_LENGTH) != 0))
    {
        throw std::runtime_error("Wrong Password");
    }
}

CLEADecryptor::~CLEADecryptor()
{
}

HRESULT CLEADecryptor::Decrypt(LPBYTE buffer, UINT32 size)
{
    BYTE* tmpBuffer = GetTmpBuffer(size);
    int processedSize = lea_online_update(&ctx_, tmpBuffer, buffer, size);
    if (processedSize < 0)
    {
        return E_FAIL;
    }

    memcpy(buffer, tmpBuffer, processedSize);
    if ((UINT32)processedSize != size)
    {
        lea_online_final(&ctx_, buffer + processedSize);
    }
    return S_OK;
}

LPBYTE CLEADecryptor::GetTmpBuffer(UINT32 minSize)
{
    if (tmpBuffer_.size() < minSize)
    {
        tmpBuffer_ = std::vector<BYTE>(minSize, 0);
    }
    return tmpBuffer_.data();
}
