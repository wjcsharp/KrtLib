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


template<typename T>
struct NativeString
{

};

template<>
struct NativeString<char>: public ANSI_STRING
{
    typedef UNICODE_STRING  Type;
    typedef Type*           PType;
};

template<>
struct NativeString<wchar_t>: public UNICODE_STRING
{
    typedef ANSI_STRING     Type;
    typedef Type*           PType;
};

template<typename T>
class KString
{
    typedef NativeString<T> NS;
    typedef KString<T>      ThisType;

private:

    LONG            _length;
    LONG            _maxlength;
    T*              _buffer;
    AtomnicInt32*   _refCount;
    bool            _isManaged;

private:
    static ULONG GetStringLength(
        _In_ const T* cs
        )
    {
        const T *eos = cs;
        while( *eos++ ) ;
        return( (ULONG)(eos - cs - 1) );
    }


    static LONG Compare(
        _In_ const ThisType& lhs,
        _In_ const T* rhs
        )
    {
        const T* buflhs=lhs._buffer;
        const T* bufrhs=rhs;

        LONG length=lhs._length;
        while(length-- && *bufrhs)
        {
            LONG diff=*buflhs++-*bufrhs++;
            if(diff!=0)
            {
                return diff;
            }
        };

        return (GetStringLength(rhs)<<(sizeof(T)-1))-lhs._length;
    }

    static LONG Compare(
        _In_ const ThisType& lhs,
        _In_ const ThisType& rhs
        )
    {
        const T* buflhs=lhs._buffer;
        const T* bufrhs=rhs._buffer;

        LONG length = lhs._length < rhs._length? lhs._length: rhs._length;
        while(length--)
        {
            LONG diff=*buflhs++-*buflhs++;
            if(diff!=0)
            {
                return diff;
            }
        };

        return lhs._length - rhs._length;
    }

public:
    KString(void) :
        _isManaged(false),
        _refCount(NULL),
    {
    }

    explicit KString(
        _In_bytecount_(initilizeLength)const ULONG initLength,
        _In_opt_ const T* source = NULL,
        _In_reads_bytes_(source to copy) const ULONG copyLength = 0
        ) :
    _isManaged(true),
        _refCount(NULL)
    {
        // Allocate Buffer to store given string.
        if( NT_SUCCESS(InitRef()))
        {
            if( NT_SUCCESS(Alloc(initLength)))
            {
                if (ARGUMENT_PRESENT(source))
                {
                    Copy(source, copyLength);
                }
            }
        }
    }


    KString(
        _In_ const T* rhs,
        _In_ bool bManaged = true
        ) :
    _isManaged(bManaged),
        _refCount(NULL)
    {
        if (NULL != rhs)
        {
            if (!_isManaged)
            {
                // Just use given string. Sometime it's useful in kernel mode.
                this->_buffer = const_cast<T*>(rhs);
                this->_length = (GetStringLength(rhs))*sizeof(T);
                this->_maxlength = this->_length+sizeof(T);
            }
            else
            {
                // Allocate Buffer to store given string.
                InitRef();
                LONG rhsLen = GetStringLength(rhs)*sizeof(T);
                if (NT_SUCCESS(Alloc(rhsLen + sizeof(T))))
                {
                    if(!NT_SUCCESS(Copy(rhs, rhsLen)))
                    {
                        ASSERT(0);
                    }
                }
            }
        }

    }

     KString(
         _In_ const NS& rhs,
         _In_ bool bManaged = true
         ) :
     _isManaged(bManaged),
         _refCount(NULL)
     {
         if (!_isManaged)
         {
             // Just use given string. Sometime it's useful in kernel mode.
             this->_buffer = rhs.Buffer;
             this->_length = (ULONG)rhs.Length;
             this->_maxlength = (ULONG)rhs.MaximumLength;
         }
         else
         {
             // Allocate Buffer to store given string.
             InitRef();
             if (NT_SUCCESS(Alloc(rhs.MaximumLength)))
             {
                 if(!NT_SUCCESS(Copy(rhs.Buffer, rhs.Length)))
                 {
                     ASSERT(0);
                 }
             }
         }
     }


    KString(
        _In_ const KString& rhs
        ) :
        _isManaged(false),
        _refCount(NULL)
    {
        if (!rhs._isManaged)
        {
            // _isManaged can only be set to false when construct with tchar*, unicode_string*
            // if rhs cannot manage it's Buffer, current string must allocate its Buffer.
            InitRef();
            if (NT_SUCCESS(Alloc(rhs._maxlength)))
            {
                if(!NT_SUCCESS(Copy(rhs._buffer, rhs._length)))
                {
                    ASSERT(0);
                }
            }
        }
        else
        {
            rhs.AddRefBuf();
            _buffer = rhs._buffer;
            _length = rhs._length;
            _maxlength = rhs._maxlength;
            _refCount = rhs._refCount;
            _isManaged = rhs._isManaged;
        }
    }

    /*
    *  operators
    */
    KString& operator=(_In_ const KString& rhs)
    {
        if (_refCount == rhs._refCount)
        {
            ASSERT(_buffer == rhs._buffer);
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
        Append(rhs._buffer, rhs._length);
        return strRet;
    }

    KString& operator+=(_In_ const KString& rhs)
    {
        Append(rhs._buffer, rhs._length);
        return (*this);
    }

    bool operator ==(_In_ const KString& rhs)
    {
        return (Compare(*this, rhs) == 0);
    }

    bool operator ==(_In_ const T* rhs)
    {
        return (Compare(*this, rhs) == 0);
    }

    bool operator !=(_In_ const KString& rhs)
    {
        return (Compare(*this, rhs) != 0);
    }

    operator NS () const
    {
        NS nsting;
        nsting.Buffer = this->_buffer;
        nsting.Length = (USHORT)this->_length;
        nsting.MaximumLength = (USHORT)this->_maxlength;

        return nsting;
    }

    /*
    *  Release
    */
    ~KString()
    {
        ReleaseRef();
    }

    const T* GetBuffer()
    {
        return _buffer;
    }

private:


    NTSTATUS Alloc(
        _In_bytecount_(allocate) LONG length)
    {
        this->_length = 0;
        this->_maxlength = 0;
        _buffer = (T*)StringAllocater::Allocate(length);
        ASSERT(_buffer != NULL);
        if (_buffer == NULL)
        {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        this->_isManaged = true;
        this->_maxlength = length;

        return STATUS_SUCCESS;
    }

    NTSTATUS Copy(
        _In_opt_ const T* source,
        _In_opt_bytecount_(source) LONG copyLength
        )
    {
        if (!ARGUMENT_PRESENT(source))
        {
            return STATUS_INVALID_PARAMETER;
        }

        LONG n = copyLength;
        if (n > this->_maxlength)
        {
            n = this->_maxlength-sizeof(T);
        }

        this->_length = n;
        RtlCopyMemory(this->_buffer, source, n);

        return STATUS_SUCCESS;
    }

    NTSTATUS Append(
        _In_ const T* source,
        _In_reads_bytes_(source) ULONG appendLength
        )
    {
        NTSTATUS status = STATUS_UNSUCCESSFUL;
        bool bNeedAlloc = false;

        if (!ARGUMENT_PRESENT(source))
        {
            return STATUS_INVALID_PARAMETER;
        }

        //ASSERT(appendLength <= (GetStringLength(source))*sizeof(T));
        /*if (appendLength >GetStringLength(strSrc))
        {
            appendLength = GetStringLength(strSrc);
        }*/

        // check current allocated max Length
        
        LONG needLength = this->_length + appendLength + sizeof(T);
        if (IsShared() ||
            ( needLength > this->_maxlength)
            )
        {
            KString strTemp(needLength, this->_buffer, this->_length);
            Common::Swap<KString>(*this, strTemp);
        }

        
        T* dst = &this->_buffer[ (this->_length / sizeof(T)) ];
        RtlMoveMemory( dst,
            source,
            appendLength
            );
        this->_length += appendLength;
        return( STATUS_SUCCESS );
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

        return STATUS_SUCCESS;
    }

    ULONG AddRefBuf() const
    {
        if (_refCount)
        {
            return ++(*_refCount);
        }
        return 0;
    }

    ULONG ReleaseRef()
    {
        if( _refCount && 
            (--(*_refCount) == 0))
        {
            if (_isManaged && _buffer)
            {
                StringAllocater::Release(_buffer);
            }

            delete _refCount;
        }

        _length = 0;
        _maxlength = 0;
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

};

typedef NativeString<char>      ANTString;
typedef KString<char>           AString;

typedef NativeString<wchar_t>   WNTString;
typedef KString<wchar_t>        WString;