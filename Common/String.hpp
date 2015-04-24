#pragma once
#include "utilis.hpp"
#include "String.back.hpp"


static LONGLONG _gAllocCount;
static LONGLONG _gReleaseCount;


class StringAllocater
{
public:

public:
    static PVOID Allocate(_In_ ULONG Length)
    {	
        PVOID buf = new char[Length];
        ASSERT(buf != NULL);
        if (buf != NULL)
        {
            ++_gAllocCount;
        }
        return buf;
    }

    static void Release(_In_ PVOID Buffer)
    {
        ++_gReleaseCount;
        delete[] Buffer;
    }

    static bool IsLeaked()
    {
        return(_gReleaseCount != _gAllocCount);
    }
};


template<typename BaseType>
struct NativeString
{
};

template<>
struct NativeString<char>: public ANSI_STRING
{
    NativeString()
    {
        Length = 0;
        MaximumLength = 0;
        Buffer = 0;
    }
};

template<>
struct NativeString<wchar_t>: public UNICODE_STRING
{
    NativeString(USHORT length = 0, USHORT maxLength = 0, wchar_t* buffer = NULL)
    {
        Length = length;
        MaximumLength = maxLength;
        Buffer = buffer;
    }
};


template<typename BaseType>
class StringOperator
{
};

template<>
class StringOperator<wchar_t>
{
    typedef NativeString<wchar_t> NS;
public:

    static void InitString(_Out_ NS* destinationString, wchar_t* sourceString)
    {
        return RtlInitUnicodeString(destinationString, sourceString);
    }

    static void CopyString(
        _Out_ NS* destinationString,
        _In_opt_ const NS* sourceString)
    {
        return RtlCopyUnicodeString(destinationString, sourceString);
    }

    static NTSTATUS AppendString(
        _Inout_ NS* destination,
        _In_ NS* source
        )
    {
        return RtlAppendUnicodeStringToString(destination, source);
    }


    static LONG Compare(
        _In_ NS* string1,
        _In_ const NS* string2,
        _In_ bool caseInSensitive
        )
    {
        return RtlCompareUnicodeString(string1, string2, caseInSensitive);       
    }
};

template<>
class StringOperator<char>
{
    typedef NativeString<char> NS;
public:

    static void InitString(
        _Out_ NS* destinationString,
        _In_ char* sourceString)
    {
        return RtlInitString(destinationString, sourceString);
    }

    static void CopyString(
        _Out_ NS* destinationString,
        _In_opt_ const NS* sourceString)
    {
        return RtlCopyString(destinationString, sourceString);
    }

    static NTSTATUS AppendString(
        _Inout_ NS* destination,
        _In_ NS* source
        )
    {
        return RtlAppendStringToString(destination, source);
    }


    static LONG Compare(
        _In_ NS* string1,
        _In_ const NS* string2,
        _In_ bool caseInSensitive
        )
    {
        return RtlCompareString(string1, string2, caseInSensitive);       
    }
};



template<typename BaseType>
class KString : public NativeString<BaseType>
{
    typedef BaseType CharType;
    typedef NativeString<BaseType> NS;
    typedef StringOperator<BaseType> SO;

public:
    KString(void) :
        NS(),
        _isManaged(false),
        _refCount(NULL),
    {
    }


    KString(
        _In_bytecount_(initilizeLength) USHORT initLength,
        _In_opt_ CharType* source,
        _In_reads_bytes_(source) USHORT Length
        ) :
    _isManaged(true),
        _refCount(NULL)
    {
        // Allocate Buffer to store given string.
        InitRef();
        NS ustrTemp(Length, Length + sizeof(CharType), source);
        AllocAndCopy(initLength, &ustrTemp);
    }

    KString(
        _In_ CharType* rhs,
        _In_ bool bManaged = true
        ) :
    NS(),
        _isManaged(bManaged),
        _refCount(NULL)
    {
        if (NULL != rhs)
        {
            if (!_isManaged)
            {
                // Just use given string. Sometime it's useful in kernel mode.
                SO::InitString(this, rhs);
            }
            else
            {
                // Allocate Buffer to store given string.

                InitRef();
                NS ustrTemp;
                SO::InitString(&ustrTemp, rhs);
                AllocAndCopy(ustrTemp.MaximumLength, &ustrTemp);

            }
        }

    }

    KString(
        _In_ NS& rhs,
        _In_ bool bManaged = true
        ) :
    NS(),
        _isManaged(bManaged),
        _refCount(NULL)
    {
        if (!_isManaged)
        {
            // Just use given string. Sometime it's useful in kernel mode.
            this->Buffer = rhs.Buffer;
            this->Length = rhs.Length;
            this->MaximumLength = rhs.MaximumLength;
        }
        else
        {
            // Allocate Buffer to store given string.
            InitRef();
            AllocAndCopy(rhs.MaximumLength, &rhs);
        }
    }


    KString(
        _In_ const KString& rhs
        ) :
    NS(),
        _isManaged(false),
        _refCount(NULL)
    {
        if (!rhs._isManaged)
        {
            // _isManaged can only be set to false when construct with tchar*, unicode_string*
            // if rhs cannot manage it's Buffer, current string must allocate its Buffer.
            InitRef();
            AllocAndCopy(rhs.MaximumLength, const_cast<KString*>(&rhs));
        }
        else
        {
            rhs.AddRefBuf();
            Buffer = rhs.Buffer;
            Length = rhs.Length;
            MaximumLength = rhs.MaximumLength;
            _refCount = rhs._refCount;
            _isManaged = rhs._isManaged;
        }
    }

    /*
    *  operators
    */
    KString& operator=(_In_ const KString& rhs)
    {
        if (_refCount != rhs._refCount)
        {
            ASSERT(Buffer == rhs.Buffer);
            // if current string already set to the same with rhs,
            // just return
            return *this;
        }
        else
        {
            KString strTemp(rhs);
            Common::Swap<KString>(*this, strTemp);
        }

        return *this;
    }

    KString operator+(_In_ const KString& rhs)
    {
        KString strRet(*this);
        Append(rhs.Buffer, rhs.Length);
        return strRet;
    }

    KString& operator+=(_In_ const KString& rhs)
    {
        Append(rhs.Buffer, rhs.Length);
        return (*this);
    }

    bool operator ==(_In_ const KString& rhs)
    {
        return (SO::Compare(this, &rhs, false) == 0);
    }

    bool operator !=(_In_ const KString& rhs)
    {
        return (SO::Compare(this, &rhs, false) != 0);
    }

    /*
    *  Release
    */
    ~KString()
    {
        ReleaseRef();
    }

private:

    /*
    *	Allocate and initialize Buffer
    */
    NTSTATUS AllocAndCopy(_In_bytecount_(allocate) ULONG Length,_In_opt_ NS* strSrc)
    {
        Buffer = (CharType*)StringAllocater::Allocate(Length);
        ASSERT(Buffer != NULL);
        if (Buffer == NULL)
        {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        this->_isManaged = true;
        this->MaximumLength = (USHORT)Length;

        if (NULL != strSrc)
        {
            SO::CopyString(this, strSrc);
        }
        return STATUS_SUCCESS;
    }

    NTSTATUS Alloc(_In_bytecount_(allocate) ULONG Length)
    {
        Buffer = (CharType*)StringAllocater::Allocate(Length);
        ASSERT(Buffer != NULL);
        if (Buffer == NULL)
        {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        this->_isManaged = true;
        this->MaximumLength = (USHORT)Length;

        return STATUS_SUCCESS;
    }

    NTSTATUS Append(_In_ CharType* strSrc,_In_reads_bytes_(strSrc) USHORT Length)
    {
        NTSTATUS status = STATUS_UNSUCCESSFUL;
        bool bNeedAlloc = false;

        // check current allocated max Length
        USHORT needLength = this->Length + Length + sizeof(CharType);
        if (IsShared() ||
            ( needLength > this->MaximumLength)
            )
        {
            KString strTemp(needLength, Buffer, this->Length);
            Common::Swap<KString>(*this, strTemp);
        }

        NS sourth(Length, Length+sizeof(CharType), strSrc);
        status = SO::AppendString(this, &sourth);
        return status;
    }

    /*
    *	reference count related
    */
    NTSTATUS InitRef()
    {        
        if (NULL != _refCount)
        {
            ASSERT(_refCount == NULL);
            return STATUS_UNSUCCESSFUL;
        }
        _refCount = new AtomnicInt32;
        if (NULL == _refCount)
        {
            ASSERT(_refCount != NULL);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        (*_refCount)++;

        Buffer = NULL;
        Length = 0;
        MaximumLength = 0;
        return STATUS_SUCCESS;
    }

    ULONG AddRefBuf() const
    {
        if (_refCount)
        {
            (*_refCount)++;
        }
        return 0;
    }

    ULONG ReleaseRef()
    {
        if( _refCount && 
            (--(*_refCount) == 0))
        {
            if (_isManaged && Buffer)
            {
                StringAllocater::Release(Buffer);
            }

            delete _refCount;
        }

        Buffer = NULL;
        Length = 0;
        MaximumLength = 0;
        _refCount = NULL;
        return 0;
    }

    bool IsShared()
    {
        if (_refCount)
        {
            return((*_refCount)-1 > 0);
        }
        return true;
    }

public:

    // whether the Buffer is can be managed
    bool _isManaged;
    AtomnicInt32* _refCount;

};

typedef NativeString<char>      NStringA;
typedef KString<char>           StringA;

typedef NativeString<wchar_t>   NStringW;
typedef KString<wchar_t>        StringW;