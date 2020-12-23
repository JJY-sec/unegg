#ifndef __EGG_ARCHIVE_INFO_H__
#define __EGG_ARCHIVE_INFO_H__

#include "../../common/IArchive.h"
#include "../../common/ICrypto.h"
#include "../../common/ICoder.h"

#include "../../common/seven_stream.h"
#include "../../common/UnknownImpl.h"

#include "eggstruct.h"

namespace NArchive
{
    namespace NEgg
    {
        enum Result
        {
            SelectFirstFile = 0xA0450000
        };

        typedef struct _OPEN_CALLBACK
        {
            CComPtr<IArchiveOpenCallback> callback;
        } OPEN_CALLBACK, *POPEN_CALLBACK;

        typedef struct _EXTRACT_CALLBACK
        {
            CComPtr<IArchiveExtractCallback> extractCallback;
            CComPtr<ICoderCallback> coderCallback;
            CComPtr<ICodeBuffer> buffer;
            CComPtr<IDecryptor> decryptor;
        } EXTRACT_CALLBACK, *PEXTRACT_CALLBACK;

        class CBlockInfo
        {
        public:
            CBlockInfo();
            explicit CBlockInfo(seven_istream& sis);

            HRESULT LoadBlockInfo(seven_istream& sis);

            int GetMethod() const { return method_; }
            DWORD GetUnpackedSize() const { return unpackedSize_; }
            DWORD GetPackedSize() const { return packedSize_; }
            DWORD GetCRC() const { return crc_; }
            ULONGLONG GetOffset() const { return offset_; }

            // return is NArchive::NExtract::NOperationResult or HRESULT
            int Decompress(IInStream* inStream, ISequentialOutStream* outStream, EXTRACT_CALLBACK& callback);

        private:
            int method_;
            DWORD unpackedSize_;
            DWORD packedSize_;
            DWORD crc_;
            ULONGLONG offset_;
        };

        class CPackedFileInfo
        {
        public:
            CPackedFileInfo();
            explicit CPackedFileInfo(seven_istream& sis);

            HRESULT LoadPackedFileInfo(seven_istream& sis);

            bool IsDirectory() const;
            ULONGLONG GetPackedSize() const;
            bool IsEncrypted() const;

            ULONGLONG GetSize() const { return size_; }
            LPCWSTR GetPath() const { return path_.c_str(); }
            LPCWSTR GetComment() const { return comment_.c_str(); }
            const FILETIME& GetLastModified() const { return lastModified_; }
            DWORD GetAttributes() const { return attributes_; }

            // return is NArchive::NExtract::NOperationResult or HRESULT
            int ExtractTo(IInStream* inStream, ISequentialOutStream* outStream, EXTRACT_CALLBACK& callback);

        private:
            HRESULT LoadFileNameInformation(seven_istream& sis);
            HRESULT LoadCommentInformation(seven_istream& sis);
            HRESULT LoadWindowsFileInformation(seven_istream& sis);
            HRESULT LoadEncryptInformation(seven_istream& sis);

            int BuildZipCrypto(LPCTSTR password, EXTRACT_CALLBACK& callback);
            int BuildAESCrypto(LPCTSTR password, EXTRACT_CALLBACK& callback);
            int BuildLEACrypto(LPCTSTR password, EXTRACT_CALLBACK& callback);

            DWORD index_;
            ULONGLONG size_;
            std::basic_string<WCHAR> path_;
            std::basic_string<WCHAR> comment_;
            FILETIME lastModified_;
            DWORD attributes_;
            std::shared_ptr<ENCRYPT_INFO> encryption_;
            std::basic_string<TCHAR> password_;
            std::list<CBlockInfo> blocks_;
        };

        class CInArchive : public IAddRefReleaseImpl<IInArchive, IFileArrayOwner>
        {
        public:
            CInArchive();
            virtual ~CInArchive();

            // IUnknown
            virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);

            // IInArchive
            virtual HRESULT STDMETHODCALLTYPE Open(IInStream *stream, const UInt64 *maxCheckStartPosition, IArchiveOpenCallback *openCallback);
            virtual HRESULT STDMETHODCALLTYPE Close();

            virtual HRESULT STDMETHODCALLTYPE GetNumberOfItems(UInt32 *numItems);
            virtual HRESULT STDMETHODCALLTYPE Extract(const UInt32* indices, UInt32 numItems, Int32 testMode, IArchiveExtractCallback *extractCallback);

            virtual HRESULT STDMETHODCALLTYPE GetProperty(UInt32 index, PROPID propID, PROPVARIANT *value);
            virtual HRESULT STDMETHODCALLTYPE GetNumberOfProperties(UInt32 *numProps);
            virtual HRESULT STDMETHODCALLTYPE GetPropertyInfo(UInt32 index, BSTR *name, PROPID *propID, VARTYPE *varType);

            virtual HRESULT STDMETHODCALLTYPE GetArchiveProperty(PROPID propID, PROPVARIANT *value);
            virtual HRESULT STDMETHODCALLTYPE GetNumberOfArchiveProperties(UInt32 *numProps);
            virtual HRESULT STDMETHODCALLTYPE GetArchivePropertyInfo(UInt32 index, BSTR *name, PROPID *propID, VARTYPE *varType);

            // IFileArrayOwner
            virtual HRESULT STDMETHODCALLTYPE DetachFileArray(IFileArray** fileArray);
            virtual HRESULT STDMETHODCALLTYPE AttachFileArray(IFileArray* fileArray);
            virtual HRESULT STDMETHODCALLTYPE GetFileList(IFileArray** fileArray);
            virtual HRESULT STDMETHODCALLTYPE SetFileList(IFileArray* fileArray);

        private:
            HRESULT LoadPrefixInformation(seven_istream& sis, OPEN_CALLBACK& callback);
            HRESULT LoadSplitInformation(seven_istream& sis, OPEN_CALLBACK& callback, bool bSkip = false);
            HRESULT LoadSolidInformation(seven_istream& sis);
            HRESULT LoadCommentInformation(seven_istream& sis);

            HRESULT GetVolumeStream(IInStream** inStream, unsigned long id, OPEN_CALLBACK& callback);

            HRESULT FindArchiveName(unsigned long id, std::basic_string<TCHAR>& fileName, LPCTSTR folderPath);
            HRESULT IsArchiveForId(LPCTSTR filePath, unsigned long id, bool* isMatched);
            HRESULT GetNextSplitId(IInStream* inStream, unsigned long* nextId);

            std::vector<CPackedFileInfo> files_;
            bool isSolid_;
            bool isSplit_;
            std::basic_string<WCHAR> comment_;

            CComPtr<IInStream> inStream_;
            std::basic_string<TCHAR> archiveFolderPath_;
        };
    }
}

#endif
