#ifndef __EGG_UNKNOWNIMPL_H__
#define __EGG_UNKNOWNIMPL_H__

/*
    IAddRefReleaseImpl 템플릿을 둘 이상의 템플릿 인수와 함께 상속받으면 C4584 경고가 발생합니다.
    이는 IUnknown 인터페이스를 모두 상속받는 COM 개체의 상속구조와 관련되어 있습니다.

    본 템플릿 사용시에는 문제될 소지가 없으나 QueryInterface를 구현할 때에는
    반드시 TUnknown 템플릿의 구현을 참고하세요.
*/
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4584)
#endif
template<typename... Unknowns>
class IAddRefReleaseImpl : virtual public Unknowns...
{
#ifdef _WIN32
#pragma warning(pop)
#endif
public:
    IAddRefReleaseImpl() : __referenceCount_(0) {}
    virtual ~IAddRefReleaseImpl() {}

    virtual ULONG STDMETHODCALLTYPE AddRef()
    {
        return InterlockedIncrement(&__referenceCount_);
    }

    virtual ULONG STDMETHODCALLTYPE Release()
    {
        ULONG ret = InterlockedDecrement(&__referenceCount_);
        if (ret == 0)
        {
            delete this;
        }
        return ret;
    }

private:
    ULONG __referenceCount_;
};

#ifdef _WIN32

/*
    TUnknown 템플릿을 사용하기 위해서는 템플릿 인수에 전달된 모든 인터페이스에 GUID가 지정되어 있어야 합니다.
    지정할 수 없는 상황이라면 직접 IAddRefReleaseImpl 템플릿을 상속받고 QueryInterface 함수를 구현해야 합니다.
*/
template<typename... Unknowns>
class TUnknown : public IAddRefReleaseImpl<Unknowns...>
{
public:
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject)
    {
        if (riid == IID_IUnknown)
        {
            return QueryIUnknown<Unknowns...>(ppvObject);
        }
        return _QueryInterface<Unknowns...>(riid, ppvObject);
    }

private:
    template<typename... Args>
    inline HRESULT _QueryInterface(REFIID riid, void** ppvObject)
    {
        return __QueryInterface<Args...>(riid, ppvObject);
    }

    template<typename _First, typename... _Others>
    inline HRESULT __QueryInterface(REFIID riid, void** ppvObject)
    {
        static_assert(std::is_base_of<IUnknown, _First>::value, "Unknowns MUST be extended from IUnknown.");
        if (__uuidof(_First) == riid)
        {
            *ppvObject = (LPVOID)(_First*)this;
            ((_First*)(*ppvObject))->AddRef();
            return S_OK;
        }
        return _QueryInterface<_Others...>(riid, ppvObject);
    }

    template<>
    inline HRESULT _QueryInterface(REFIID, void** ppvObject)
    {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }

    template<typename _First, typename... _Others>
    inline HRESULT QueryIUnknown(void **ppvObject)
    {
        *ppvObject = (LPVOID)(IUnknown*)(_First*)this;
        ((IUnknown*)(*ppvObject))->AddRef();
        return S_OK;
    }
};

template<typename... Interfaces>
class TQueryInterface : public Interfaces...
{
public:
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject)
    {
        return _QueryInterface<Interfaces...>(riid, ppvObject);
    }

private:
    template<typename... Args>
    inline HRESULT _QueryInterface(REFIID riid, void** ppvObject)
    {
        return __QueryInterface<Args...>(riid, ppvObject);
    }

    template<typename _First, typename... _Others>
    inline HRESULT __QueryInterface(REFIID riid, void** ppvObject)
    {
        if (__uuidof(_First) == riid)
        {
            *ppvObject = (LPVOID)(_First*)this;
            return S_OK;
        }
        return _QueryInterface<_Others...>(riid, ppvObject);
    }

    template<>
    inline HRESULT _QueryInterface(REFIID, void** ppvObject)
    {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }
};

#else

#define TUnknown IAddRefReleaseImpl

#endif

#define START_QUERYINTERFACE virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) { HRESULT ret = S_OK;
#define QUERY_UNKNOWN(first) if (riid == __uuidof(IUnknown)) { *ppvObject = (void*)(IUnknown*)(first*)this; AddRef(); }
#define QUERY_INTERFACE(inter) else if (riid == __uuidof(inter)) { *ppvObject = (void*)(inter*)this; AddRef(); }
#define QUERY_INTERFACE_IID(inter) else if (riid == IID_ ## inter) { *ppvObject = (void*)(inter*)this; AddRef(); }
#define END_QUERYINTERFACE else { ret = E_NOINTERFACE; } return ret; }

#endif
