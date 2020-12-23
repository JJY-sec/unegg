#ifndef __ALEGG_ICRYPTO_H__
#define __ALEGG_ICRYPTO_H__

// {058A6C88-FB13-48BF-BEF0-169CC2E3A52F}
DEFINE_GUID(IID_IEncryptor, 0x58a6c88, 0xfb13, 0x48bf, 0xbe, 0xf0, 0x16, 0x9c, 0xc2, 0xe3, 0xa5, 0x2f);
struct DECLSPEC_UUID("058A6C88-FB13-48BF-BEF0-169CC2E3A52F") IEncryptor : public IUnknown
{
    virtual HRESULT Encrypt(LPBYTE buffer, UINT32 size) = 0;
};

// {45C037D8-991C-49D2-BC39-34F8618269AA}
DEFINE_GUID(IID_IDecryptor, 0x45c037d8, 0x991c, 0x49d2, 0xbc, 0x39, 0x34, 0xf8, 0x61, 0x82, 0x69, 0xaa);
struct DECLSPEC_UUID("45C037D8-991C-49D2-BC39-34F8618269AA") IDecryptor : public IUnknown
{
    virtual HRESULT Decrypt(LPBYTE buffer, UINT32 size) = 0;
};

#endif
