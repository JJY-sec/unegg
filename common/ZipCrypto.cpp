#include "StdAfx.h"
#include "ZipCrypto.h"
#include "crc32.h"

CZipDecryptor::CZipDecryptor(const char* password)
{
    const LPBYTE data = (const LPBYTE)password;
    UINT32 size = strlen(password);

    keys_[0] = 0x12345678;
    keys_[1] = 0x23456789;
    keys_[2] = 0x34567890;

    for (UINT32 i = 0; i < size; i++)
    {
        UpdateKeys(data[i]);
    }

    memcpy(initKeys_, keys_, sizeof(keys_));
}

CZipDecryptor::~CZipDecryptor()
{
}

HRESULT CZipDecryptor::Decrypt(LPBYTE buffer, UINT32 size)
{
    for (UINT32 i = 0; i < size; i++)
    {
        buffer[i] ^= GetDecryptByte();
        UpdateKeys(buffer[i]);
    }
    return S_OK;
}

void CZipDecryptor::UpdateKeys(unsigned char b)
{
    CRC32UpdateByte(&keys_[0], b);
    keys_[1] = (keys_[1] + (keys_[0] & 0xFF)) * 0x8088405 + 1;
    CRC32UpdateByte(&keys_[2], keys_[1] >> 24);
}

unsigned char CZipDecryptor::GetDecryptByte()
{
    UINT32 tmp = keys_[2] | 2;
    return (unsigned char)((tmp * (tmp ^ 1)) >> 8);
}

void CZipDecryptor::RestoreKeys()
{
    memcpy(keys_, initKeys_, sizeof(keys_));
}
