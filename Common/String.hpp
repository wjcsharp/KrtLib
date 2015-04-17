#pragma once
#include "utilis.hpp"

static AtomnicInt64 _gAllocCount;
static AtomnicInt64 _gReleaseCount;

class StringOperator
{
public:
    /*
     *	RtlCopyUnicodeString and RltCopyAnsiString
     */
    void static CopyString(
        _Out_ PUNICODE_STRING DestinationString,
        _In_opt_ PCUNICODE_STRING SourceString)
    {
        UNALIGNED WCHAR *src, *dst;
        ULONG n;

        if (ARGUMENT_PRESENT(SourceString))
        {
            dst = DestinationString->Buffer;
            src = SourceString->Buffer;
            n = SourceString->Length;
            if ((USHORT)n > DestinationString->MaximumLength)
            {
                n = DestinationString->MaximumLength;
            }

            DestinationString->Length = (USHORT)n;
            RtlCopyMemory(dst, src, n);
            if( (DestinationString->Length + sizeof (WCHAR)) <= DestinationString->MaximumLength)
            {
                dst[n / sizeof(WCHAR)] = UNICODE_NULL;
            }
        }
        else
        {
            DestinationString->Length = 0;
        }

        return;
    }

    static NTSTATUS AppendStringToString(
        _Inout_ PUNICODE_STRING Destination,
        _In_ PUNICODE_STRING Source)
    {
        USHORT n = Source->Length;
        UNALIGNED WCHAR *dst;

        if (n)
        {
            if ((n + Destination->Length) > Destination->MaximumLength) {
                return( STATUS_BUFFER_TOO_SMALL );
            }

            dst = &Destination->Buffer[ (Destination->Length / sizeof( WCHAR )) ];
            RtlMoveMemory( dst, Source->Buffer, n );

            Destination->Length += n;

            if( (Destination->Length + 1) < Destination->MaximumLength) {
                dst[ n / sizeof( WCHAR ) ] = UNICODE_NULL;
            }
        }

        return( STATUS_SUCCESS );
    }
};

class StringAllocater
{
public:

public:
    static PVOID Allocate(ULONG length)
    {	
        PVOID buf = new char[length];
        ASSERT(buf != NULL);
        if (buf != NULL)
        {
            ++_gAllocCount;
        }
        return buf;
    }

    static void Release(PVOID buffer)
    {
        ++_gReleaseCount;
        delete[] buffer;
    }

    static bool IsLeaked()
    {
        return(_gReleaseCount != _gAllocCount);
    }
};


class UnicodeString : public UNICODE_STRING
{

public:
    UnicodeString(void) :
        _isManaged(true),
        _refCount(NULL)
    {
        Buffer = NULL;
        Length = 0;
        MaximumLength = 0;
    }

    UnicodeString(_In_ wchar_t* rhs,_In_ bool bManaged = true) :
        _isManaged(bManaged),
        _refCount(NULL)
    {
        if (!_isManaged)
        {
            // Just use given string. Sometime it's useful in kernel mode.
            RtlInitUnicodeString(this, rhs);
        }
        else
        {
            // Allocate buffer to store given string.
            InitRef();

            UNICODE_STRING ustrTemp;
            RtlInitUnicodeString(&ustrTemp, rhs);
            AllocAndCopy(ustrTemp.MaximumLength, &ustrTemp);
        }
    }

    UnicodeString(_In_ UNICODE_STRING& rhs,_In_ bool bManaged = true) :
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
            // Allocate buffer to store given string.
            InitRef();
            AllocAndCopy(rhs.MaximumLength, &rhs);
        }
    }


    /*
    *  Copy and assignment constructor
    */
    UnicodeString(_In_ const UnicodeString& rhs) :
        _isManaged(true),
        _refCount(NULL)
    {
        if (!rhs._isManaged)
        {
            // _isManaged can only be set to false when construct with tchar*, unicode_string*
            // if rhs cannot manage it's buffer, current string must allocate its buffer.
            InitRef();
            AllocAndCopy(rhs.MaximumLength, const_cast<UnicodeString*>(&rhs));
        }
        else
        {
            rhs.AddRefBuf();
            Buffer = rhs.Buffer;
            Length = rhs.Length;
            MaximumLength = rhs.MaximumLength;
            _refCount = rhs._refCount;
        }
    }

    /*
    *  operator =
    */
    UnicodeString& operator=(_In_ const UnicodeString& rhs)
    {
        if (_refCount == rhs._refCount)
        {
            ASSERT(Buffer == rhs.Buffer);
            // if current string already set to the same with rhs,
            // just return
            return *this;
        }

        ReleaseRef();
        if (!rhs._isManaged)
        {
            // _isManaged can only be set to false when construct with tchar*, unicode_string*
            // if rhs cannot manage it's buffer, current string must allocate its buffer.			
            AllocAndCopy(rhs.MaximumLength, const_cast<UnicodeString*>(&rhs));
            _isManaged = true;
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
        return *this;
    }

    UnicodeString& operator=(_In_opt_z_ wchar_t* rhs)
    {
        ReleaseRef();

        // Allocate buffer to store given string.
        {
            _isManaged = true;
            InitRef();

            UNICODE_STRING ustrTemp;
            RtlInitUnicodeString(&ustrTemp, rhs);
            AllocAndCopy(ustrTemp.MaximumLength, &ustrTemp);
        }
        return *this;
    }

    UnicodeString& operator=(_In_ UNICODE_STRING& rhs)
    {
        ReleaseRef();

        // Allocate buffer to store given string.
        {
            _isManaged = true;
            InitRef();
            AllocAndCopy(rhs.MaximumLength, &rhs);
        }
        return *this;
    }

    /*
    *  operator +
    */
    UnicodeString operator+(_In_ const UnicodeString& rhs)
    {
        UnicodeString strRet(*this);
        strRet.AppendString(const_cast<UnicodeString*>(&rhs));
        return strRet;
    }

//     friend UnicodeString operator+(_In_ const UnicodeString& lhs, _In_ const UnicodeString& rhs)
//     {
//         UnicodeString strRet;
//         strRet.Reallocate((lhs.Length + rhs.Length + sizeof(wchar_t)));
//         strRet += lhs;
//         strRet += rhs;
//         return strRet;
//     }


    UnicodeString operator+(_In_ wchar_t* rhs)
    {
        UnicodeString strRet(*this);
        strRet.AppendString(rhs);
        return strRet;
    }

    UnicodeString operator+(_In_ UNICODE_STRING& rhs)
    {
        UnicodeString strRet(*this);
        strRet.AppendString(&rhs);
        return strRet;
    }

    UnicodeString operator+(_In_ PUNICODE_STRING rhs)
    {
        UnicodeString strRet(*this);
        strRet.AppendString(rhs);
        return strRet;
    }
    /*
    *  operator +=
    */
    UnicodeString& operator+=(_In_ const UnicodeString& rhs)
    {
        AppendString(dynamic_cast<PUNICODE_STRING>(const_cast<UnicodeString*>(&rhs)));
        return (*this);
    }

    UnicodeString& operator+=(_In_ wchar_t* rhs)
    {
        AppendString(const_cast<wchar_t*>(rhs));
        return (*this);
    }

    UnicodeString& operator+=(_In_ UNICODE_STRING& rhs)
    {
        AppendString(&rhs);
        return (*this);
    }
    
    UnicodeString& operator+=(_In_ PUNICODE_STRING rhs)
    {
        AppendString(rhs);
        return (*this);
    }
    
    /*
    *
    */
    bool operator ==(_In_ const wchar_t* rhs)
    {
        return (0 == wcscmp(Buffer, rhs));
    }

    /*
    *  Release
    */
    ~UnicodeString()
    {
        ReleaseRef();
    }

    /*
     *	Reallocate,
     *  @param: nLength is the memory length, but not the string length
     */
    void Reallocate(_In_ int nLength)
    {
        ReleaseRef();
        InitRef();
        
        Buffer = (wchar_t*)StringAllocater::Allocate(nLength);
        if (NULL == Buffer)
        {
            ASSERT(Buffer);
            return;
        }
        this->Length = 0;
        this->MaximumLength = (USHORT)nLength;
        this->_isManaged = true;
    }


private:

    /*
    *	Allocate and initialize Buffer
    */
    NTSTATUS AllocAndCopy(int length, PUNICODE_STRING strSrc)
    {
        Buffer = (wchar_t*)StringAllocater::Allocate(length);
        ASSERT(Buffer != NULL);
        if (Buffer == NULL)
        {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        this->_isManaged = true;
        this->MaximumLength = (USHORT)length;

        StringOperator::CopyString(this, strSrc);
        return STATUS_SUCCESS;
    }

    ULONG AppendString(_In_ PUNICODE_STRING rhs)
    {
        NTSTATUS status = STATUS_UNSUCCESSFUL;
        // check current allocated max length
        if (this->Length + rhs->Length + sizeof(wchar_t)> this->MaximumLength)
        {
            UnicodeString strTemp;
            strTemp.InitRef();
            status = strTemp.AllocAndCopy(this->Length + rhs->Length + sizeof(wchar_t), this);            
            if (!NT_SUCCESS(status))
            {
                return status;
            }

            Common::Swap<UnicodeString>(*this, strTemp);
        }

        status = StringOperator::AppendStringToString(this, rhs);
        return status;
    }

    ULONG AppendString(_In_ wchar_t* strSrc)
    {
        UNICODE_STRING strTemp = {0};
        RtlInitUnicodeString(&strTemp, strSrc);
        return AppendString(&strTemp);
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

public:

    // whether the Buffer is can be managed
    bool _isManaged;
    AtomnicInt32* _refCount;

};