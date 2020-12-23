#include "StdAfx.h"
#include <jni.h>

#include <map>
#include <random>

#include "exports.h"
#include "common/PropID.h"
#include "lib/PropVariant.h"

HRESULT g_LastResult = S_OK;

std::map<jint, CComPtr<IInArchive>> g_ArchiveMap;
std::map<jint, string> g_PasswordMap;

jint JNI_GetLastResult(JNIEnv* env, jobject)
{
    jint ret = g_LastResult;
    switch (ret)
    {
    case S_FALSE:
    case HRESULT_FROM_WIN32(ERROR_BAD_FORMAT):          ret = ercBadFormat;         break;
    case E_NOINTERFACE:                                 ret = ercNoInterface;       break;
    case HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND):      ret = ercFileNotFound;      break;
    case E_OUTOFMEMORY:                                 ret = ercOutOfMemory;       break;
    case HRESULT_FROM_WIN32(ERROR_HANDLE_EOF):          ret = ercEndOfFile;         break;
    case HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED):       ret = ercUnsupported;       break;
    case HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER):   ret = ercInvalidParameters; break;
    case HRESULT_FROM_WIN32(ERROR_INVALID_FLAGS):       ret = ercInvalidFlags;      break;
    }
    if ((ret != 0) && ((ret < START_ERROR_CODE) || (ercLast < ret)))
    {
        ret = ercUndefined;
    }
    return ret;
}

jint JNI_OpenArchive(JNIEnv* env, jobject, jbyteArray path)
{
    jint ret = 0;
    jsize size = env->GetArrayLength(path);
    jbyte* bytes = env->GetByteArrayElements(path, NULL);

    char* pszPath = new char[size + 1];
    strncpy(pszPath, reinterpret_cast<const char*>(bytes), size);
    pszPath[size] = 0;

    CComPtr<IInArchive> archive;
    HRESULT result = OpenArchive(pszPath, &archive);
    if (result == S_OK)
    {
        std::random_device rd;
        std::mt19937 generator(rd());
        ret = std::uniform_int_distribution<int>(1)(generator);

        g_ArchiveMap[ret] = archive;
    }
    else
    {
        g_LastResult = result;
    }

    delete[] pszPath;

    env->ReleaseByteArrayElements(path, bytes, 0);
    return ret;
}

void JNI_CloseArchive(JNIEnv* env, jobject, jint handle)
{
    auto it = g_ArchiveMap.find(handle);
    if ((it != g_ArchiveMap.end()) && it->second)
    {
        it->second.Release();
    }
}

jint JNI_GetNumberOfItems(JNIEnv* env, jobject, jint handle)
{
    jint ret = 0;
    HRESULT result = S_OK;
    auto it = g_ArchiveMap.find(handle);
    if ((it != g_ArchiveMap.end()) && it->second)
    {
        UInt32 count;
        result = it->second->GetNumberOfItems(&count);
        if (result == S_OK)
        {
            ret = count;
        }
    }
    else
    {
        result = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }
    g_LastResult = result;
    return ret;
}

jstring JNI_GetItemName(JNIEnv* env, jobject, jint handle, jint index)
{
    jstring ret = NULL;
    HRESULT result = S_OK;
    auto it = g_ArchiveMap.find(handle);
    if ((it != g_ArchiveMap.end()) && it->second)
    {
        NWindows::NCOM::CPropVariant prop;
        result = it->second->GetProperty(index, kpidPath, &prop);
        if (result == S_OK)
        {
            ret = env->NewString(
                reinterpret_cast<const jchar*>(prop.bstrVal),
                SysStringLen(prop.bstrVal) * sizeof(jchar)
            );
        }
    }
    else
    {
        result = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }
    g_LastResult = result;

    if (ret == NULL)
    {
        ret = env->NewString(
            reinterpret_cast<const jchar*>(L""), 0
        );
    }

    return ret;
}

jboolean JNI_Extract(JNIEnv* env, jobject, jint handle, jbyteArray destination, jbyteArray password)
{
    HRESULT result = S_OK;

    auto it = g_ArchiveMap.find(handle);
    if ((it != g_ArchiveMap.end()) && it->second)
    {
        char* pszDestination = NULL;
        {
            jsize size = env->GetArrayLength(destination);
            jbyte* bytes = env->GetByteArrayElements(destination, NULL);

            pszDestination = new char[size + 1];
            strncpy(pszDestination, reinterpret_cast<const char*>(bytes), size);
            pszDestination[size] = 0;

            env->ReleaseByteArrayElements(destination, bytes, 0);
        }

        char* pszPassword = NULL;
        {
            jsize size = env->GetArrayLength(password);
            if (size)
            {
                jbyte* bytes = env->GetByteArrayElements(password, NULL);

                pszPassword = new char[size + 1];
                strncpy(pszPassword, reinterpret_cast<const char*>(bytes), size);
                pszPassword[size] = 0;

                env->ReleaseByteArrayElements(password, bytes, 0);
            }
        }

        result = ExtractArchive(it->second, pszDestination, pszPassword);

        if (pszPassword)
        {
            delete[] pszPassword;
        }

        if (pszDestination)
        {
            delete[] pszDestination;
        }
    }
    else
    {
        result = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }
    g_LastResult = result;
    return ((g_LastResult == S_OK) ? JNI_TRUE : JNI_FALSE);
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK)
    {
        return JNI_ERR;
    }

    // Find your class. JNI_OnLoad is called from the correct class loader context for this to work.
    jclass c = env->FindClass("UnEGG");
    if (c == nullptr)
    {
        return JNI_ERR;
    }

    // Register your class' native methods.
    static const JNINativeMethod methods[] =
    {
        { "_GetLastResult", "()I", reinterpret_cast<void*>(JNI_GetLastResult) },
        { "_OpenArchive", "([B)I", reinterpret_cast<void*>(JNI_OpenArchive) },
        { "_CloseArchive", "(I)V", reinterpret_cast<void*>(JNI_CloseArchive) },
        { "_GetNumberOfItems", "(I)I", reinterpret_cast<void*>(JNI_GetNumberOfItems) },
        { "_GetItemName", "(II)Ljava/lang/String;", reinterpret_cast<void*>(JNI_GetItemName) },
        { "_Extract", "(I[B[B)Z", reinterpret_cast<void*>(JNI_Extract) },
    };
    int rc = env->RegisterNatives(c, methods, sizeof(methods)/sizeof(JNINativeMethod));
    if (rc != JNI_OK)
    {
        return rc;
    }

    return JNI_VERSION_1_6;
}
