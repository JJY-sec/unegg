#ifndef __EGG_IARCHIVE_H__
#define __EGG_IARCHIVE_H__

#include "../lib/MyGuidDef.h"
#include "IStream.h"

namespace NFileTimeType
{
  enum EEnum
  {
    kWindows,
    kUnix,
    kDOS
  };
}

namespace NArcInfoFlags
{
  const UInt32 kKeepName        = 1 << 0;  // keep name of file in archive name
  const UInt32 kAltStreams      = 1 << 1;  // the handler supports alt streams
  const UInt32 kNtSecure        = 1 << 2;  // the handler supports NT security
  const UInt32 kFindSignature   = 1 << 3;  // the handler can find start of archive
  const UInt32 kMultiSignature  = 1 << 4;  // there are several signatures
  const UInt32 kUseGlobalOffset = 1 << 5;  // the seek position of stream must be set as global offset
  const UInt32 kStartOpen       = 1 << 6;  // call handler for each start position
  const UInt32 kPureStartOpen   = 1 << 7;  // call handler only for start of file
  const UInt32 kBackwardOpen    = 1 << 8;  // archive can be open backward
  const UInt32 kPreArc          = 1 << 9;  // such archive can be stored before real archive (like SFX stub)
  const UInt32 kSymLinks        = 1 << 10; // the handler supports symbolic links
  const UInt32 kHardLinks       = 1 << 11; // the handler supports hard links
}

namespace NArchive
{
  namespace NHandlerPropID
  {
    enum
    {
      kName = 0,        // VT_BSTR
      kClassID,         // binary GUID in VT_BSTR
      kExtension,       // VT_BSTR
      kAddExtension,    // VT_BSTR
      kUpdate,          // VT_BOOL
      kKeepName,        // VT_BOOL
      kSignature,       // binary in VT_BSTR
      kMultiSignature,  // binary in VT_BSTR
      kSignatureOffset, // VT_UI4
      kAltStreams,      // VT_BOOL
      kNtSecure,        // VT_BOOL
      kFlags            // VT_UI4
      // kVersion          // VT_UI4 ((VER_MAJOR << 8) | VER_MINOR)
    };
  }

  namespace NExtract
  {
    namespace NAskMode
    {
      enum
      {
        kExtract = 0,
        kTest,
        kSkip
      };
    }
  
    namespace NOperationResult
    {
      enum
      {
        kOK = 0,
        kUnsupportedMethod,
        kDataError,
        kCRCError,
        kUnavailable,
        kUnexpectedEnd,
        kDataAfterEnd,
        kIsNotArc,
        kHeadersError,
        kWrongPassword
      };
    }
  }

  namespace NEventIndexType
  {
    enum
    {
      kNoIndex = 0,
      kInArcIndex,
      kBlockIndex,
      kOutArcIndex
    };
  }
  
  namespace NUpdate
  {
    namespace NOperationResult
    {
      enum
      {
        kOK = 0
        , // kError
      };
    }
  }
}

// {FE363F6C-5996-4776-8A09-3C439071969E}
DEFINE_GUID(IID_IArchiveOpenCallback, 0xfe363f6c, 0x5996, 0x4776, 0x8a, 0x9, 0x3c, 0x43, 0x90, 0x71, 0x96, 0x9e);
class IArchiveOpenCallback : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE SetTotal(const UInt64 *files, const UInt64 *bytes) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetCompleted(const UInt64 *files, const UInt64 *bytes) = 0;
};

// {AF0B72A3-9597-479A-8510-4DA826C3FD80}
DEFINE_GUID(IID_IProgress, 0xaf0b72a3, 0x9597, 0x479a, 0x85, 0x10, 0x4d, 0xa8, 0x26, 0xc3, 0xfd, 0x80);
class IProgress : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE SetTotal(UInt64 total) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetCompleted(const UInt64 *completeValue) = 0;
};

// {C53E71E6-A4D9-4EC9-9788-B060CA44AE06}
DEFINE_GUID(IID_IArchiveExtractCallback, 0xc53e71e6, 0xa4d9, 0x4ec9, 0x97, 0x88, 0xb0, 0x60, 0xca, 0x44, 0xae, 0x6);
class IArchiveExtractCallback : public IProgress
{
public:
    virtual HRESULT STDMETHODCALLTYPE GetStream(UInt32 index, ISequentialOutStream **outStream, Int32 askExtractMode) = 0;
    virtual HRESULT STDMETHODCALLTYPE PrepareOperation(Int32 askExtractMode) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetOperationResult(Int32 opRes) = 0;
};

// {AC5076DD-F46A-4A72-86E4-322096958092}
DEFINE_GUID(IID_IArchiveOpenVolumeCallback, 0xac5076dd, 0xf46a, 0x4a72, 0x86, 0xe4, 0x32, 0x20, 0x96, 0x95, 0x80, 0x92);
class IArchiveOpenVolumeCallback : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE GetProperty(PROPID propID, PROPVARIANT *value) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetStream(const wchar_t *name, IInStream **inStream) = 0;
};

// {FE200983-6260-4E44-95C8-7BBB2FF545E8}
DEFINE_GUID(IID_IInArchive, 0xfe200983, 0x6260, 0x4e44, 0x95, 0xc8, 0x7b, 0xbb, 0x2f, 0xf5, 0x45, 0xe8);
class IInArchive : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE Open(IInStream *stream, const UInt64 *maxCheckStartPosition, IArchiveOpenCallback *openCallback) = 0;
    virtual HRESULT STDMETHODCALLTYPE Close() = 0;
    virtual HRESULT STDMETHODCALLTYPE GetNumberOfItems(UInt32 *numItems) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetProperty(UInt32 index, PROPID propID, PROPVARIANT *value) = 0;
    virtual HRESULT STDMETHODCALLTYPE Extract(const UInt32* indices, UInt32 numItems, Int32 testMode, IArchiveExtractCallback *extractCallback) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetArchiveProperty(PROPID propID, PROPVARIANT *value) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetNumberOfProperties(UInt32 *numProps) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetPropertyInfo(UInt32 index, BSTR *name, PROPID *propID, VARTYPE *varType) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetNumberOfArchiveProperties(UInt32 *numProps) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetArchivePropertyInfo(UInt32 index, BSTR *name, PROPID *propID, VARTYPE *varType) = 0;
};

// {F5D462BD-015D-4722-87D2-4EC5539E2A07}
DEFINE_GUID(IID_IFileArray, 0xf5d462bd, 0x15d, 0x4722, 0x87, 0xd2, 0x4e, 0xc5, 0x53, 0x9e, 0x2a, 0x7);
class DECLSPEC_UUID("F5D462BD-015D-4722-87D2-4EC5539E2A07") IFileArray : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE GetFileCount(UINT64* fileCount) = 0;
};

// {4BC25728-C6FE-4E3A-8194-709BEB53A541}
DEFINE_GUID(IID_IFileArrayOwner, 0x4bc25728, 0xc6fe, 0x4e3a, 0x81, 0x94, 0x70, 0x9b, 0xeb, 0x53, 0xa5, 0x41);
class DECLSPEC_UUID("4BC25728-C6FE-4E3A-8194-709BEB53A541") IFileArrayOwner : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE DetachFileArray(IFileArray** fileArray) = 0;
    virtual HRESULT STDMETHODCALLTYPE AttachFileArray(IFileArray* fileArray) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetFileList(IFileArray** fileArray) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetFileList(IFileArray* fileArray) = 0;
};

// {D3EE728D-F5A8-4AF3-9B5B-3B1C8C5787A6}
DEFINE_GUID(IID_IThreadSpecifier, 0xd3ee728d, 0xf5a8, 0x4af3, 0x9b, 0x5b, 0x3b, 0x1c, 0x8c, 0x57, 0x87, 0xa6);
class DECLSPEC_UUID("D3EE728D-F5A8-4AF3-9B5B-3B1C8C5787A6") IThreadSpecifier : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE IsMultiThreadModel(VARIANT_BOOL* b) = 0;
};

#endif
