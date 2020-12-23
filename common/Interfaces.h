#ifndef __ALEGG_INTERFACES_H__
#define __ALEGG_INTERFACES_H__

// {435F1D29-A21D-4A30-8D01-3EE4E73296EE}
DEFINE_GUID(IID_ISetLastResult, 0x435f1d29, 0xa21d, 0x4a30, 0x8d, 0x1, 0x3e, 0xe4, 0xe7, 0x32, 0x96, 0xee);
struct DECLSPEC_UUID("435F1D29-A21D-4A30-8D01-3EE4E73296EE") ISetLastResult : public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE SetLastResult(HRESULT result) = 0;
};

// {51FCB9D8-B692-4600-BF15-37A03F95AC78}
DEFINE_GUID(IID_ISetModifiedTime, 0x51fcb9d8, 0xb692, 0x4600, 0xbf, 0x15, 0x37, 0xa0, 0x3f, 0x95, 0xac, 0x78);
struct DECLSPEC_UUID("51FCB9D8-B692-4600-BF15-37A03F95AC78") ISetModifiedTime : public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE SetModifiedTime(const LPFILETIME modifiedTime) = 0;
};

// {CE1CFDDD-47CE-48C3-B90B-FDDFEC8A20FE}
DEFINE_GUID(IID_ISetItemText, 0xce1cfddd, 0x47ce, 0x48c3, 0xb9, 0xb, 0xfd, 0xdf, 0xec, 0x8a, 0x20, 0xfe);
class DECLSPEC_UUID("CE1CFDDD-47CE-48C3-B90B-FDDFEC8A20FE") ISetItemText : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE SetItemText(LPCTSTR text) = 0;
};

// {7E239124-A5F7-4B4A-896D-7C4FAD3422E5}
DEFINE_GUID(IID_IMemoryMappedBlock, 0x7e239124, 0xa5f7, 0x4b4a, 0x89, 0x6d, 0x7c, 0x4f, 0xad, 0x34, 0x22, 0xe5);
struct DECLSPEC_UUID("7E239124-A5F7-4B4A-896D-7C4FAD3422E5") IMemoryMappedBlock : public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE GetBlockSize(UINT32* blockSize) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetBlockData(LPBYTE* blockData) = 0;
};

// {D764C837-18D9-4837-B43E-11EF01C4193D}
DEFINE_GUID(IID_IMemoryMappedFile, 0xd764c837, 0x18d9, 0x4837, 0xb4, 0x3e, 0x11, 0xef, 0x1, 0xc4, 0x19, 0x3d);
struct DECLSPEC_UUID("D764C837-18D9-4837-B43E-11EF01C4193D") IMemoryMappedFile : public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE GetBlockCount(UINT32* blockCount) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetMemoryBlock(UINT32 blockNumber, IMemoryMappedBlock** memoryMappedBlock) = 0;
};

#endif
