#ifndef __EGG_SEVEN_STREAM_H__
#define __EGG_SEVEN_STREAM_H__

#include "IStream.h"

class operation_exception : public std::runtime_error
{
    typedef std::runtime_error base_exception;
public:
    operation_exception(HRESULT res)
        : base_exception("Operation is failed")
        , result(res)
    {}

    HRESULT result;
};

// Throw If Not OK
inline void TINOK(HRESULT res)
{ if (res != S_OK) throw operation_exception(res); }

class seven_stream_base
{
public:
    inline bool good() const
    {
        return state_ == std::ios_base::goodbit;
    }
    inline bool eof() const
    {
        return state_ & std::ios_base::eofbit;
    }
    inline bool fail() const
    {
        return state_ & std::ios_base::failbit;
    }
    inline bool bad() const
    {
        return state_ & std::ios_base::badbit;
    }

    inline HRESULT get_result() const
    {
        return result_;
    }

protected:
    seven_stream_base() : state_(0) {}

    inline void clear_state()
    {
        state_ = 0;
    }
    inline void set_state(int state)
    {
        state_ = state;
    }
    inline void add_state(int state)
    {
        state_ |= state;
    }
    inline void remove_state(int state)
    {
        state_ &= ~state;
    }

    inline void set_result(HRESULT result)
    {
        result_ = result;
    }

private:
    int state_;
    HRESULT result_;
};

class seven_istream : public seven_stream_base
{
public:
    seven_istream(IInStream* inStream);
    ~seven_istream();

    operator IInStream* () { return inStream_; }
    seven_istream& operator = (IInStream* inStream)
    {
        inStream_ = inStream;
        return *this;
    }

    template<typename value_type>
    HRESULT read(value_type& v, UINT32* processedSize)
    {
        return read(&v, sizeof(value_type), processedSize);
    }

    HRESULT read(void* data, UINT32 size, UINT32* processedSize);
    HRESULT seek(LONGLONG offset, UINT32 origin, UINT64* newPosition);

    seven_istream& operator >> (bool& v)                { TINOK(read(v, NULL)); return *this; }
    seven_istream& operator >> (char& v)                { TINOK(read(v, NULL)); return *this; }
    seven_istream& operator >> (unsigned char& v)       { TINOK(read(v, NULL)); return *this; }
    seven_istream& operator >> (wchar_t& v)             { TINOK(read((short&)v, NULL)); return *this; }
    seven_istream& operator >> (short& v)               { TINOK(read(v, NULL)); return *this; }
    seven_istream& operator >> (unsigned short& v)      { TINOK(read(v, NULL)); return *this; }
    seven_istream& operator >> (int& v)                 { TINOK(read(v, NULL)); return *this; }
    seven_istream& operator >> (unsigned int& v)        { TINOK(read(v, NULL)); return *this; }
    seven_istream& operator >> (long& v)                { TINOK(read(v, NULL)); return *this; }
    seven_istream& operator >> (unsigned long& v)       { TINOK(read(v, NULL)); return *this; }
    seven_istream& operator >> (long long& v)           { TINOK(read(v, NULL)); return *this; }
    seven_istream& operator >> (unsigned long long& v)  { TINOK(read(v, NULL)); return *this; }
    seven_istream& operator >> (float& v)               { TINOK(read(v, NULL)); return *this; }
    seven_istream& operator >> (double& v)              { TINOK(read(v, NULL)); return *this; }
    seven_istream& operator >> (long double& v)         { TINOK(read(v, NULL)); return *this; }

private:
    CComPtr<IInStream> inStream_;
};

class seven_ostream : public seven_stream_base
{
public:
    seven_ostream(ISequentialOutStream* outStream);
    ~seven_ostream();

    operator ISequentialOutStream* () { return outStream_; }
    seven_ostream& operator = (ISequentialOutStream* outStream)
    {
        outStream_ = outStream;
        return *this;
    }

    template<typename value_type>
    HRESULT write(const value_type& v, UINT32* processedSize)
    {
        return write(&v, sizeof(value_type), processedSize);
    }

    HRESULT write(const void* data, UINT32 size, UINT32* processedSize);
    HRESULT pad(UINT32 size, UINT32* processedSize);

    seven_ostream& operator << (const bool& v)                  { TINOK(write(v, NULL)); return *this; }
    seven_ostream& operator << (const char& v)                  { TINOK(write(v, NULL)); return *this; }
    seven_ostream& operator << (const unsigned char& v)         { TINOK(write(v, NULL)); return *this; }
    seven_ostream& operator << (const wchar_t& v)               { TINOK(write((const short&)v, NULL)); return *this; }
    seven_ostream& operator << (const short& v)                 { TINOK(write(v, NULL)); return *this; }
    seven_ostream& operator << (const unsigned short& v)        { TINOK(write(v, NULL)); return *this; }
    seven_ostream& operator << (const int& v)                   { TINOK(write(v, NULL)); return *this; }
    seven_ostream& operator << (const unsigned int& v)          { TINOK(write(v, NULL)); return *this; }
    seven_ostream& operator << (const long& v)                  { TINOK(write(v, NULL)); return *this; }
    seven_ostream& operator << (const unsigned long& v)         { TINOK(write(v, NULL)); return *this; }
    seven_ostream& operator << (const long long& v)             { TINOK(write(v, NULL)); return *this; }
    seven_ostream& operator << (const unsigned long long& v)    { TINOK(write(v, NULL)); return *this; }
    seven_ostream& operator << (const float& v)                 { TINOK(write(v, NULL)); return *this; }
    seven_ostream& operator << (const double& v)                { TINOK(write(v, NULL)); return *this; }
    seven_ostream& operator << (const long double& v)           { TINOK(write(v, NULL)); return *this; }

private:
    CComPtr<ISequentialOutStream> outStream_;
};

#endif
