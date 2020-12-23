#include "StdAfx.h"
#include "EggArchiveInfo.h"

#include "../../common/IArchive.h"
#include "../../common/IPassword.h"
#include "../../common/PropID.h"
#include "../PropVariant.h"

#include "../../common/TimeService.h"

#include "../../common/StoreCoder.h"
#include "../../common/DeflateCoder.h"
#include "../../common/Bzip2Coder.h"
#include "../../common/AZOCoder.h"
#include "../../common/LZMACoder.h"
#include "../../common/CoderImpl.h"

#include "../../common/ZipCrypto.h"
#include "../../common/AESCrypto.h"
#include "../../common/LEACrypto.h"

#include "../../common/MultipleStream.h"
#include "../../common/AZFileStreams.h"

#include "PropService.h"

#include "eggoper.h"

#ifndef _WIN32

#include <dirent.h>

#if UNICODE
#define _tcsrchr        wcsrchr
#else
#define _tcsrchr        strrchr
#endif

#endif

namespace NArchive
{
    namespace NEgg
    {
        class CVolumeInStream : public IAddRefReleaseImpl<IInStream>
        {
        public:
            CVolumeInStream(IInStream* realInStream)
                : realInStream_(realInStream)
            {
                UInt32 signature;
                seven_istream sis(realInStream_);
                sis >> signature;
                if (signature != fhsEnd)
                {
                    throw operation_exception(HRESULT_FROM_WIN32(ERROR_BAD_FORMAT));
                }
                realInStream_->Seek(0, SEEK_CUR, &dataStartOffset_);
            }

            ~CVolumeInStream()
            {
            }

            // IUnknown
            virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject)
            {
                HRESULT ret = S_OK;
                if (riid == IID_IUnknown)
                {
                    *ppvObject = (void*)(IUnknown*)this;
                    AddRef();
                }
                else if (riid == IID_ISequentialInStream)
                {
                    *ppvObject = (void*)(ISequentialInStream*)this;
                    AddRef();
                }
                else if (riid == IID_IInStream)
                {
                    *ppvObject = (void*)(IInStream*)this;
                    AddRef();
                }
                else
                {
                    ret = E_NOINTERFACE;
                    *ppvObject = NULL;
                }
                return ret;
            }

            // ISequentialInStream
            STDMETHOD(Read)(void *data, UInt32 size, UInt32 *processedSize)
            {
                return realInStream_->Read(data, size, processedSize);
            }

            // IInStream
            STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition)
            {
                HRESULT ret = S_OK;
                UInt64 realNewPosition = 0;
                switch (seekOrigin)
                {
                case SEEK_SET:
                    ret = realInStream_->Seek(dataStartOffset_ + (UInt64)abs(offset), seekOrigin, &realNewPosition);
                    break;
                case SEEK_CUR:
                case SEEK_END:
                    ret = realInStream_->Seek(offset, seekOrigin, &realNewPosition);
                    break;
                default:
                    ret = E_INVALIDARG;
                    break;
                }

                if (ret == S_OK)
                {
                    if (realNewPosition < dataStartOffset_)
                    {
                        ret = realInStream_->Seek(dataStartOffset_, SEEK_SET, &realNewPosition);
                    }

                    if ((ret == S_OK) && newPosition)
                    {
                        *newPosition = realNewPosition - dataStartOffset_;
                    }
                }
                return ret;
            }

        private:
            CComPtr<IInStream> realInStream_;
            UInt64 dataStartOffset_;
        };


        CBlockInfo::CBlockInfo()
            : method_(0)
            , unpackedSize_(0), packedSize_(0)
            , crc_(0), offset_(0)
        {
        }

        CBlockInfo::CBlockInfo(seven_istream& sis)
            : method_(0)
            , unpackedSize_(0), packedSize_(0)
            , crc_(0), offset_(0)
        {
            TINOK(LoadBlockInfo(sis));
        }

        HRESULT CBlockInfo::LoadBlockInfo(seven_istream& sis)
        {
            HRESULT ret = S_OK;
            BLOCK_INFO bh;
            sis >> bh;
            method_ = bh.compressMethod;
            unpackedSize_ = bh.unpackSize;
            packedSize_ = bh.packSize;
            crc_ = bh.crc;

            UInt32 signature;
            sis >> signature;
            if (signature == fhsEnd)
            {
                ret = sis.seek(0, SEEK_CUR, &offset_);
                if (ret == S_OK)
                {
                    ret = sis.seek(packedSize_, SEEK_CUR, NULL);
                }
            }
            else
            {
                ret = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
            }
            return ret;
        }

        int CBlockInfo::Decompress(IInStream* inStream, ISequentialOutStream* outStream, EXTRACT_CALLBACK& callback)
        {
            CComPtr<IInStream> _inStream(inStream);
            CComPtr<ISequentialOutStream> stream(outStream);

            int ret = NArchive::NExtract::NOperationResult::kOK;
            CComPtr<IDecoder> decoder;
            switch (method_)
            {
            case BLOCK_INFO::cmStore:       decoder = new CStoreCoder(callback.buffer, callback.coderCallback);     break;
            case BLOCK_INFO::cmDeflate:     decoder = new CDeflateCoder(callback.buffer, callback.coderCallback);   break;
            case BLOCK_INFO::cmBzip2:       decoder = new CBzip2Coder(callback.buffer, callback.coderCallback);     break;
            case BLOCK_INFO::cmAZO:         decoder = new CAZOCoder(callback.buffer, callback.coderCallback);       break;
            case BLOCK_INFO::cmLZMA:        decoder = new CLZMACoder(callback.buffer, callback.coderCallback);      break;
            }

            if (decoder)
            {
                UInt64 pos;
                if ((inStream->Seek((Int64)offset_, SEEK_SET, &pos) == S_OK) && (offset_ == pos))
                {
                    HRESULT result = decoder->Decode(inStream, outStream, packedSize_, unpackedSize_, callback.decryptor);
                    if (result == S_OK)
                    {
                        ret = decoder->GetResult(crc_);
                    }
                    else
                    {
                        ret = (int)result;
                    }
                }
                else
                {
                    ret = NArchive::NExtract::NOperationResult::kUnexpectedEnd;
                }
            }
            else
            {
                ret = NArchive::NExtract::NOperationResult::kUnsupportedMethod;
            }

            return ret;
        }


        CPackedFileInfo::CPackedFileInfo()
            : index_(0), size_(0), attributes_(0)
        {
            memset(&lastModified_, 0, sizeof(lastModified_));
        }

        CPackedFileInfo::CPackedFileInfo(seven_istream& sis)
            : index_(0), size_(0), attributes_(0)
        {
            memset(&lastModified_, 0, sizeof(lastModified_));
            TINOK(LoadPackedFileInfo(sis));
        }

        HRESULT CPackedFileInfo::LoadPackedFileInfo(seven_istream& sis)
        {
            HRESULT ret = S_OK;
            FILE_INFO fh;
            sis >> fh;
            index_ = fh.index;
            size_ = fh.size;

            UInt32 signature;
            do
            {
                sis >> signature;
                switch (signature)
                {
                case fhsFilename:           ret = LoadFileNameInformation(sis);             break;
                case fhsComment:            ret = LoadCommentInformation(sis);              break;
                case fhsWindowsFileInfo:    ret = LoadWindowsFileInformation(sis);          break;
                case fhsEncrypt:            ret = LoadEncryptInformation(sis);              break;
                case fhsEnd:                                                                break;
                default:                    ret = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);     break;
                }
            } while ((ret == S_OK) && (signature != fhsEnd));

            if (ret == S_OK)
            {
                bool bContinue = true;
                do
                {
                    sis >> signature;
                    switch (signature)
                    {
                    case fhsBlock:      blocks_.push_back(CBlockInfo(sis));                 break;
                    case fhsComment:
                    case fhsFile:
                    case fhsEnd:        bContinue = false;  sis.seek(-4, SEEK_CUR, NULL);   break;
                    default:            ret = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);         break;
                    }
                } while ((ret == S_OK) && bContinue);
            }

            return ret;
        }

        bool CPackedFileInfo::IsDirectory() const
        {
            return (attributes_ & FILE_ATTRIBUTE_DIRECTORY);
        }

        ULONGLONG CPackedFileInfo::GetPackedSize() const
        {
            ULONGLONG ret = 0;
            for (auto it = blocks_.begin(); it != blocks_.end(); ++it)
            {
                ret += it->GetPackedSize();
            }
            return ret;
        }

        bool CPackedFileInfo::IsEncrypted() const
        {
            return (bool)encryption_;
        }

        int CPackedFileInfo::ExtractTo(IInStream* inStream, ISequentialOutStream* outStream, EXTRACT_CALLBACK& callback)
        {
            CComPtr<IInStream> _inStream(inStream);
            CComPtr<ISequentialOutStream> stream(outStream);
            int ret = NArchive::NExtract::NOperationResult::kOK;

            if (encryption_)
            {
                if (password_.size() == 0)
                {

                    CComPtr<ICryptoGetTextPassword> cryptoGetTextPassword;
                    if (callback.extractCallback->QueryInterface(IID_ICryptoGetTextPassword, (void**)&cryptoGetTextPassword) == S_OK)
                    {
                        BSTR password;
                        if ((ret = cryptoGetTextPassword->CryptoGetTextPassword(&password)) == S_OK)
                        {
                            UINT cbPassword = SysStringByteLen(password);
                            LPWSTR pszPassword = (LPWSTR)malloc(cbPassword + sizeof(WCHAR));
                            memset(pszPassword, 0, cbPassword + sizeof(WCHAR));
                            memcpy(pszPassword, password, cbPassword);
                            password_ = tstring(pszPassword);
                            free(pszPassword);
                            SysFreeString(password);
                        }
                    }
                }

                if (password_.size() != 0)
                {
                    switch (encryption_->encryptionMethod)
                    {
                    case ENCRYPT_INFO::emZipCrypto:
                        ret = BuildZipCrypto(password_.c_str(), callback);
                        break;
                    case ENCRYPT_INFO::emAES128:
                    case ENCRYPT_INFO::emAES256:
                        ret = BuildAESCrypto(password_.c_str(), callback);
                        break;
                    case ENCRYPT_INFO::emLEA128:
                    case ENCRYPT_INFO::emLEA256:
                        ret = BuildLEACrypto(password_.c_str(), callback);
                        break;
                    default:
                        ret = NArchive::NExtract::NOperationResult::kUnsupportedMethod;
                        break;
                    }

                    if (ret == NArchive::NExtract::NOperationResult::kWrongPassword)
                    {
                        password_.clear();
                    }
                }
                else if (ret != HRESULT_FROM_WIN32(ERROR_CANCELLED))
                {
                    ret = NArchive::NExtract::NOperationResult::kWrongPassword;
                }
            }

            for (auto it = blocks_.begin(); (ret == NArchive::NExtract::NOperationResult::kOK) && (it != blocks_.end()); ++it)
            {
                ret = it->Decompress(inStream, stream, callback);
            }
            return ret;
        }

        HRESULT CPackedFileInfo::LoadFileNameInformation(seven_istream& sis)
        {
            FILENAME_INFO fnh;
            sis >> fnh;
            path_ = fnh.path;
            return S_OK;
        }

        HRESULT CPackedFileInfo::LoadCommentInformation(seven_istream& sis)
        {
            COMMENT_INFO ch;
            sis >> ch;
            comment_ = ch.comment;
            return S_OK;
        }

        HRESULT CPackedFileInfo::LoadWindowsFileInformation(seven_istream& sis)
        {
            WINDOWS_FILE_INFO wfi;
            sis >> wfi;
            memcpy(&lastModified_, &wfi.lastModified, sizeof(FILETIME));
            attributes_ = wfi.attributes;
            return S_OK;
        }

        HRESULT CPackedFileInfo::LoadEncryptInformation(seven_istream& sis)
        {
            encryption_.reset(new ENCRYPT_INFO());
            sis >> *encryption_;
            return S_OK;
        }

        int CPackedFileInfo::BuildZipCrypto(LPCTSTR password, EXTRACT_CALLBACK& callback)
        {
            int ret = NArchive::NExtract::NOperationResult::kOK;
            if (callback.decryptor)
            {
                CZipDecryptor* zipDecryptor = dynamic_cast<CZipDecryptor*>((IDecryptor*)(callback.decryptor));
                if (zipDecryptor)
                {
                    zipDecryptor->RestoreKeys();
                }
            }
            else
            {
                callback.decryptor = new CZipDecryptor(string(password));
            }

            if (callback.decryptor)
            {
                unsigned char data[12];
                memcpy(data, encryption_->data.zipCrypto.verifyData, 12);
                if ((callback.decryptor->Decrypt(data, 12) != S_OK) || (data[11] != (encryption_->data.zipCrypto.crc >> 24)))
                {
                    ret = NArchive::NExtract::NOperationResult::kWrongPassword;
                }
            }
            else
            {
                ret = NArchive::NExtract::NOperationResult::kWrongPassword;
            }
            return ret;
        }

        int CPackedFileInfo::BuildAESCrypto(LPCTSTR password, EXTRACT_CALLBACK& callback)
        {
            int ret = NArchive::NExtract::NOperationResult::kOK;
            try
            {
                switch (encryption_->encryptionMethod)
                {
                case ENCRYPT_INFO::emAES128:
                    callback.decryptor = new CAESDecryptor(CAESDecryptor::aes128, string(password), encryption_->data.aes_lea_128.header);
                    break;
                case ENCRYPT_INFO::emAES256:
                    callback.decryptor = new CAESDecryptor(CAESDecryptor::aes256, string(password), encryption_->data.aes_lea_256.header);
                    break;
                }
            }
            catch (std::exception&)
            {
                ret = NArchive::NExtract::NOperationResult::kWrongPassword;
            }
            return ret;
        }

        int CPackedFileInfo::BuildLEACrypto(LPCTSTR password, EXTRACT_CALLBACK& callback)
        {
            int ret = NArchive::NExtract::NOperationResult::kOK;
            try
            {
                switch (encryption_->encryptionMethod)
                {
                case ENCRYPT_INFO::emLEA128:
                    callback.decryptor = new CLEADecryptor(CLEADecryptor::lea128, string(password), encryption_->data.aes_lea_128.header);
                    break;
                case ENCRYPT_INFO::emLEA256:
                    callback.decryptor = new CLEADecryptor(CLEADecryptor::lea256, string(password), encryption_->data.aes_lea_256.header);
                    break;
                }
            }
            catch (std::exception&)
            {
                ret = NArchive::NExtract::NOperationResult::kWrongPassword;
            }
            return ret;
        }


        class CPackedFileArray : public TUnknown<IFileArray>
        {
        public:
            CPackedFileArray() {}
            virtual ~CPackedFileArray() {}

#ifndef _WIN32
            START_QUERYINTERFACE
                QUERY_UNKNOWN(IFileArray)
                QUERY_INTERFACE(IFileArray)
            END_QUERYINTERFACE
#endif

            // IFileArray
            virtual HRESULT STDMETHODCALLTYPE GetFileCount(UINT64* fileCount)
            {
                *fileCount = fileArray_.size();
                return S_OK;
            }

            std::vector<CPackedFileInfo> fileArray_;
        };


        std::vector<BYTE> g_FileProps
        {
            kpidPath, kpidSize, kpidPackSize, kpidMTime,
            kpidComment, kpidAttrib, kpidEncrypted
        };

        std::vector<BYTE> g_ArchiveProps
        {
            kpidSolid, kpidComment
        };

        CInArchive::CInArchive()
            : isSolid_(false)
            , isSplit_(false)
        {
        }

        CInArchive::~CInArchive()
        {
        }

        HRESULT CInArchive::QueryInterface(REFIID riid, void **ppvObject)
        {
            HRESULT ret = S_OK;
            if (riid == IID_IUnknown)
            {
                *ppvObject = (void*)(IUnknown*)(IInArchive*)this;
                AddRef();
            }
            else if (riid == IID_IInArchive)
            {
                *ppvObject = (void*)(IInArchive*)this;
                AddRef();
            }
            else if (riid == __uuidof(IFileArrayOwner))
            {
                *ppvObject = (void*)(IFileArrayOwner*)this;
                AddRef();
            }
            else
            {
                ret = E_NOINTERFACE;
                *ppvObject = NULL;
            }
            return ret;
        }

        HRESULT CInArchive::Open(IInStream *stream, const UInt64*, IArchiveOpenCallback* openCallback)
        {
            HRESULT ret = S_OK;
            if (stream)
            {
                std::list<CPackedFileInfo> fileList;

                seven_istream sis(stream);
                UInt32 signature;

                OPEN_CALLBACK callback;
                callback.callback = openCallback;

                try
                {
                    sis >> signature;
                    if (signature == fhsEgg)
                    {
                        ret = LoadPrefixInformation(sis, callback);
                    }
                    else
                    {
                        ret = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
                    }

                    if (files_.size() == 0)
                    {
                        while ((ret == S_OK) && (signature != fhsEnd))
                        {
                            sis >> signature;
                            switch (signature)
                            {
                            case fhsFile:       fileList.push_back(CPackedFileInfo(sis));           break;
                            case fhsComment:    ret = LoadCommentInformation(sis);                  break;
                            case fhsEnd:                                                            break;
                            default:            ret = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);         break;
                            }
                        }
                    }
                }
                catch (operation_exception& ex)
                {
                    ret = ex.result;
                }
                catch (...)
                {
                    ret = E_FAIL;
                }

                if (ret == S_OK)
                {
                    if (!inStream_)
                    {
                        inStream_ = stream;
                    }

                    if (files_.size() == 0)
                    {
                        files_.reserve(fileList.size());
                        for (auto it = fileList.begin(); it != fileList.end(); ++it)
                        {
                            files_.push_back(std::move(*it));
                        }
                    }
                }
                else if (HRESULT_CODE(ret) == ERROR_BAD_FORMAT)
                {
                    ret = S_FALSE;
                }
            }
            else
            {
                ret = E_INVALIDARG;
            }
            return ret;
        }

        HRESULT CInArchive::Close()
        {
            inStream_.Release();
            return S_OK;
        }

        HRESULT CInArchive::GetNumberOfItems(UInt32 *numItems)
        {
            HRESULT ret = S_OK;
            if (numItems)
            {
                *numItems = files_.size();
            }
            else
            {
                ret = E_INVALIDARG;
            }
            return ret;
        }

        HRESULT CInArchive::Extract(const UInt32* indices, UInt32 numItems, Int32, IArchiveExtractCallback *extractCallback)
        {
            HRESULT ret = S_OK;
            CComPtr<IArchiveExtractCallback> callback(extractCallback);

            ULONGLONG totalSize = 0;

            bool allFiles = !(indices && (numItems > 0));
            UINT32 itemCount = allFiles ? files_.size() : numItems;
            {
                UINT32 itemIndex = 0;
                for (UINT32 i = 0; (ret == S_OK) && (i < itemCount); i++)
                {
                    itemIndex = allFiles ? i : indices[i];
                    if (itemIndex < files_.size())
                    {
                        totalSize += files_[itemIndex].GetSize();
                    }
                    else
                    {
                        ret = E_INVALIDARG;
                    }
                }
            }

            if (ret == S_OK)
            {
                ret = callback->SetTotal(totalSize);

                EXTRACT_CALLBACK callbackStruct;
                callbackStruct.extractCallback = callback;
                callbackStruct.coderCallback = new CCoderCallback(callback);
                callbackStruct.buffer = new CCodeBuffer();

                int extractResult = 0;

                UINT32 itemIndex = 0;
                UINT64 completed = 0;
                UINT64 fileSize;
                for (UINT32 i = 0; (ret == S_OK) && (i < itemCount); i++)
                {
                    itemIndex = allFiles ? i : indices[i];

                    CComPtr<ISequentialOutStream> outStream;
                    if ((ret = callback->GetStream(itemIndex, &outStream, 0)) == S_OK)
                    {
                        fileSize = files_[itemIndex].GetSize();
                        if (outStream)
                        {
                            ret = callback->PrepareOperation(0);
                            if ((ret == S_OK) && fileSize)
                            {
                                extractResult = files_[itemIndex].ExtractTo(inStream_, outStream, callbackStruct);
                            }
                            else
                            {
                                extractResult = (int)ret;
                            }

                            if (SUCCEEDED(extractResult))
                            {
                                ret = callback->SetOperationResult(extractResult);
                            }
                            else
                            {
                                ret = (HRESULT)extractResult;
                            }
                        }
                        else
                        {
                            completed += fileSize;
                            callbackStruct.extractCallback->SetCompleted(&completed);
                        }
                    }
                }
            }

            return ret;
        }

        HRESULT CInArchive::GetProperty(UInt32 index, PROPID propID, PROPVARIANT *value)
        {
            HRESULT ret = S_OK;
            if (index < files_.size())
            {
                CPackedFileInfo& info = files_[index];

                NWindows::NCOM::CPropVariant prop;
                switch (propID)
                {
                case kpidPath:          prop = info.GetPath();                              break;
                case kpidSize:          prop = info.GetSize();                              break;
                case kpidPackSize:      prop = info.GetPackedSize();                        break;
                case kpidMTime:         prop = info.GetLastModified();                      break;
                case kpidComment:       prop = info.GetComment();                           break;
                case kpidAttrib:        prop = (UINT32)info.GetAttributes();                break;
                case kpidEncrypted:     prop = info.IsEncrypted();                          break;
                case kpidIsDir:         prop = info.IsDirectory();                          break;
                default:                ret = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);      break;
                }
                if (ret == S_OK)
                {
                    prop.Detach(value);
                }
            }
            else
            {
                ret = E_INVALIDARG;
            }
            return ret;
        }

        HRESULT CInArchive::GetNumberOfProperties(UInt32 *numProps)
        {
            return PropService::GetPropertyCount(g_FileProps, numProps);
        }

        HRESULT CInArchive::GetPropertyInfo(UInt32 index, BSTR *name, PROPID *propID, VARTYPE *varType)
        {
            return PropService::GetPropertyInfo(g_FileProps, index, name, propID, varType);
        }

        HRESULT CInArchive::GetArchiveProperty(PROPID propID, PROPVARIANT *value)
        {
            HRESULT ret = S_OK;
            NWindows::NCOM::CPropVariant prop;
            switch (propID)
            {
            case kpidSolid:     prop = isSolid_;                                break;
            case kpidComment:   prop = comment_.c_str();                        break;
            default:            ret = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);  break;
            }
            if (ret == S_OK)
            {
                prop.Detach(value);
            }
            return ret;
        }

        HRESULT CInArchive::GetNumberOfArchiveProperties(UInt32 *numProps)
        {
            return PropService::GetPropertyCount(g_ArchiveProps, numProps);
        }

        HRESULT CInArchive::GetArchivePropertyInfo(UInt32 index, BSTR *name, PROPID *propID, VARTYPE *varType)
        {
            return PropService::GetPropertyInfo(g_ArchiveProps, index, name, propID, varType);
        }

        HRESULT CInArchive::DetachFileArray(IFileArray** fileArray)
        {
            CPackedFileArray* files = new CPackedFileArray();
            files->fileArray_.swap(files_);
            *fileArray = files;
            (*fileArray)->AddRef();
            return S_OK;
        }

        HRESULT CInArchive::AttachFileArray(IFileArray* fileArray)
        {
            HRESULT ret = S_OK;
            CPackedFileArray* files = dynamic_cast<CPackedFileArray*>(fileArray);
            if (files)
            {
                files_.swap(files->fileArray_);
            }
            else
            {
                ret = E_INVALIDARG;
            }
            return ret;
        }

        HRESULT CInArchive::GetFileList(IFileArray** fileArray)
        {
            CPackedFileArray* files = new CPackedFileArray();
            files->fileArray_ = files_;
            *fileArray = files;
            (*fileArray)->AddRef();
            return S_OK;
        }

        HRESULT CInArchive::SetFileList(IFileArray* fileArray)
        {
            HRESULT ret = S_OK;
            CPackedFileArray* files = dynamic_cast<CPackedFileArray*>(fileArray);
            if (files)
            {
                files_ = files->fileArray_;
            }
            else
            {
                ret = E_INVALIDARG;
            }
            return ret;
        }

        HRESULT CInArchive::LoadPrefixInformation(seven_istream& sis, OPEN_CALLBACK& callback)
        {
            HRESULT ret = S_OK;
            HEADER eh;
            sis >> eh;

            UInt32 signature;
            do
            {
                sis >> signature;
                switch (signature)
                {
                case fhsSplit:  ret = LoadSplitInformation(sis, callback);          break;
                case fhsSolid:  ret = LoadSolidInformation(sis);                    break;
                case fhsSkip:   ret = LoadSplitInformation(sis, callback, true);    break;
                case fhsEnd:                                                        break;
                default:        ret = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);         break;
                }
            } while ((ret == S_OK) && (signature != fhsEnd));

            return ret;
        }

        HRESULT CInArchive::LoadSplitInformation(seven_istream& sis, OPEN_CALLBACK& callback, bool bSkip)
        {
            HRESULT ret = S_OK;
            SPLIT_INFO sc;
            sis >> sc;
            if (bSkip)
            {
                return ret;
            }

            if (!isSplit_)
            {
                isSplit_ = true;

                CComPtr<IInStream> firstStream;
                unsigned long nextId = 0;
                if (sc.previous == 0)
                {
                    firstStream = sis;
                    nextId = sc.next;
                }
                else
                {
                    ret = SelectFirstFile;
                }

                if (ret == S_OK && firstStream)
                {
                    // inStreamSpec에 추가되는 모든 Stream은 SPLIT_INFO까지 읽은 상태여야 합니다.
                    CInMultipleStream* inStreamSpec = new CInMultipleStream(firstStream);
                    sis = inStream_ = inStreamSpec;

                    while ((ret == S_OK) && nextId)
                    {
                        CComPtr<IInStream> nextStream;
                        if ((ret = GetVolumeStream(&nextStream, nextId, callback)) == S_OK)
                        {
                            ret = GetNextSplitId(nextStream, &nextId);
                            if (ret == S_OK)
                            {
                                inStreamSpec->AddStream(new CVolumeInStream(nextStream));
                            }
                        }
                    }
                }
            }
            return ret;
        }

        HRESULT CInArchive::LoadSolidInformation(seven_istream& sis)
        {
            SOLID_INFO sc;
            sis >> sc;
            isSolid_ = true;
            return S_OK;
        }

        HRESULT CInArchive::LoadCommentInformation(seven_istream& sis)
        {
            COMMENT_INFO ch;
            sis >> ch;
            comment_ = ch.comment;
            return S_OK;
        }

        HRESULT CInArchive::GetVolumeStream(IInStream** inStream, unsigned long id, OPEN_CALLBACK& callback)
        {
            HRESULT ret = S_OK;
            if (archiveFolderPath_.size() == 0)
            {
                CComPtr<IArchiveOpenVolumeCallback> openVolumeCallback;
                ret = callback.callback->QueryInterface(IID_IArchiveOpenVolumeCallback, (void**)&openVolumeCallback);
                if (ret == S_OK)
                {
                    NWindows::NCOM::CPropVariant prop;
                    ret = openVolumeCallback->GetProperty(kpidPath, &prop);

                    if ((ret == S_OK) && (prop.vt == VT_BSTR))
                    {
                        UINT length = SysStringByteLen(prop.bstrVal);
                        TCHAR* path = (TCHAR*)malloc(length + sizeof(TCHAR));
                        memcpy(path, prop.bstrVal, length);
                        path[length / 2] = 0;

                        LPTSTR psz = NULL;
#ifdef _WIN32
                        psz = _tcsrchr(path, TEXT('\\'));
#else
                        psz = _tcsrchr(path, TEXT('/'));
#endif
                        if (psz)
                        {
                            *psz = 0;
                            archiveFolderPath_ = path;
                        }
                        free(path);
                    }
                }
            }

            if (ret == S_OK)
            {
                tstring fileName;
                ret = FindArchiveName(id, fileName, archiveFolderPath_.c_str());

                if (ret == S_OK)
                {
                    CComPtr<IArchiveOpenVolumeCallback> openVolumeCallback;
                    ret = callback.callback->QueryInterface(IID_IArchiveOpenVolumeCallback, (void**)&openVolumeCallback);
                    if (ret == S_OK)
                    {
                        ret = openVolumeCallback->GetStream(wstring(fileName).c_str(), inStream);
                    }
                }
            }

            return ret;
        }

        HRESULT CInArchive::FindArchiveName(unsigned long id, std::basic_string<TCHAR>& fileName, LPCTSTR folderPath)
        {
            HRESULT ret = S_OK;
#if _WIN32
            tstring strFind;
            strFind.format(TEXT("%s\\*.*"), folderPath);
            WIN32_FIND_DATA wfd;
            HANDLE hFileFind = FindFirstFile(strFind, &wfd);
            if (hFileFind != INVALID_HANDLE_VALUE)
            {
                fileName.clear();
                do
                {
                    if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
                    {
                        tstring filePath;
                        filePath.format(TEXT("%s\\%s"), folderPath, wfd.cFileName);

                        bool isMatched = false;
                        if ((IsArchiveForId(filePath, id, &isMatched) == S_OK) && isMatched)
                        {
                            fileName = wfd.cFileName;
                        }
                    }
                } while ((fileName.size() == 0) && FindNextFile(hFileFind, &wfd));
                FindClose(hFileFind);

                if (fileName.size() == 0)
                {
                    ret = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
                }
            }
            else
            {
                ret = HRESULT_FROM_WIN32(GetLastError());
            }
#else
            DIR* dir;
            dirent* entry;
            string _folderPath(folderPath);
            if ((dir = opendir(_folderPath.size() == 0 ? "." : _folderPath.c_str())) != NULL)
            {
                while ((entry = readdir(dir)) != NULL)
                {
                    if (entry->d_type == DT_REG)
                    {
                        tstring filePath(entry->d_name);
                        if (_folderPath.size())
                        {
                            filePath.format(TEXT("%s/%s"), _folderPath.c_str(), tstring(entry->d_name).c_str());
                        }

                        bool isMatched = false;
                        if ((IsArchiveForId(filePath, id, &isMatched) == S_OK) && isMatched)
                        {
                            fileName = entry->d_name;
                        }
                    }
                }
                closedir(dir);

                if (fileName.size() == 0)
                {
                    ret = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
                }
            }
            else
            {
                ret = E_FAIL;
            }
#endif
            return ret;
        }

        HRESULT CInArchive::IsArchiveForId(LPCTSTR filePath, unsigned long id, bool* isMatched)
        {
            *isMatched = false;
            CInGeneralFileStream* fileStream = new CInGeneralFileStream();
            CComPtr<IInStream> inStream(fileStream);

            HRESULT ret = S_OK;
            if (fileStream->Open(filePath))
            {
                seven_istream sis(inStream);

                try
                {
                    UInt32 signature;
                    sis >> signature;
                    if (signature == fhsEgg)
                    {
                        HEADER eh;
                        sis >> eh;
                        if (id == 0)
                        {
                            do
                            {
                                sis >> signature;
                                switch (signature)
                                {
                                case fhsSplit:                                                  break;
                                case fhsSolid:  ret = LoadSolidInformation(sis);                break;
                                case fhsEnd:
                                default:        ret = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);     break;
                                }
                            } while ((ret == S_OK) && (signature != fhsSplit));

                            if ((ret == S_OK) && (signature == fhsSplit))
                            {
                                SPLIT_INFO si;
                                sis >> si;
                                if (si.previous == 0)
                                {
                                    *isMatched = true;
                                }
                            }
                        }
                        else if (eh.program == id)
                        {
                            *isMatched = true;
                        }
                    }
                }
                catch (operation_exception& ex)
                {
                    ret = ex.result;
                }
                catch (...)
                {
                    ret = E_FAIL;
                }
            }
            else
            {
#ifdef _WIN32
                ret = HRESULT_FROM_WIN32(GetLastError());
#else
                ret = E_FAIL;
#endif
            }

            return ret;
        }

        HRESULT CInArchive::GetNextSplitId(IInStream* inStream, unsigned long* nextId)
        {
            HRESULT ret = S_OK;
            seven_istream sis(inStream);
            try
            {
                UInt32 signature;
                HEADER egg;
                do
                {
                    sis >> signature;
                    switch (signature)
                    {
                    case fhsEgg:               sis >> egg;                                     break;
                    case fhsSplit:
                    {
                        SPLIT_INFO split;
                        sis >> split;
                        *nextId = split.next;
                        break;
                    }
                    default:                    ret = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);     break;
                    }
                } while ((ret == S_OK) && (signature != fhsSplit));
            }
            catch (operation_exception& ex)
            {
                ret = ex.result;
            }
            catch (...)
            {
                ret = E_FAIL;
            }
            return ret;
        }
    }
}
