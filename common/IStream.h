#ifndef __ISTREAM_H
#define __ISTREAM_H

// {43D95098-D89D-4655-983F-6DEE645457AF}
DEFINE_GUID(IID_ISequentialInStream, 0x43d95098, 0xd89d, 0x4655, 0x98, 0x3f, 0x6d, 0xee, 0x64, 0x54, 0x57, 0xaf);
class ISequentialInStream : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE Read(void *data, UInt32 size, UInt32 *processedSize) = 0;
};

// {E3413C75-4272-4BEB-9533-65A4FBA4C49B}
DEFINE_GUID(IID_ISequentialOutStream, 0xe3413c75, 0x4272, 0x4beb, 0x95, 0x33, 0x65, 0xa4, 0xfb, 0xa4, 0xc4, 0x9b);
class ISequentialOutStream : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE Write(const void *data, UInt32 size, UInt32 *processedSize) = 0;
};

// {8C63CD05-ECBF-43EA-A825-D504FC9FDFAC}
DEFINE_GUID(IID_IInStream, 0x8c63cd05, 0xecbf, 0x43ea, 0xa8, 0x25, 0xd5, 0x4, 0xfc, 0x9f, 0xdf, 0xac);
class IInStream : public ISequentialInStream
{
public:
    virtual HRESULT STDMETHODCALLTYPE Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition) = 0;
};

// {6B9D8251-268B-4FB4-8D5C-DB36D80C6687}
DEFINE_GUID(IID_IOutStream, 0x6b9d8251, 0x268b, 0x4fb4, 0x8d, 0x5c, 0xdb, 0x36, 0xd8, 0xc, 0x66, 0x87);
class IOutStream : public ISequentialOutStream
{
public:
    virtual HRESULT STDMETHODCALLTYPE Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetSize(UInt64 newSize) = 0;
};

#endif
