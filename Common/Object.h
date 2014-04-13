#pragma once
#ifndef __cplusplus
#pragma message("################### The class only support cpp ###################")
#else

#pragma warning(push)
#pragma warning(disable:4100)

class DualRefCounter
{

protected:

public:
    
    DualRefCounter(BOOLEAN strong):
        strongCount_( strong ? 1 : 0 ),
        weakCount_( strong ? 0 : 1)
    {

    }

    ~DualRefCounter()
    {
        ASSERT( 0 == strongCount_ );
        ASSERT( 0 == weakCount_ );
    }

    LONGLONG IncStrongCount( )
    {
        return InterlockedIncrement64(&strongCount_);
    }

    LONGLONG DecStrongCount()
    {
        return InterlockedDecrement64(&strongCount_);
    }

    LONGLONG IncWeakCount()
    {
        return InterlockedIncrement64(&weakCount_);
    }

    LONGLONG DecWeakCount()
    {
        return InterlockedDecrement64(&weakCount_);
    }

    LONGLONG GetStrongCount()
    {
        return strongCount_;
    }

    LONGLONG GetWeakCount()
    {
        return weakCount_;
    }
    
protected:
    volatile LONGLONG strongCount_;
    volatile LONGLONG weakCount_;
};

class KBaseRefCounter
{

    KBaseRefCounter( const KBaseRefCounter& );
    KBaseRefCounter& operator = (const KBaseRefCounter&);

protected:
    KBaseRefCounter():
        refCount_(0)
    {

    }

public:
        
    LONGLONG IncrementReference()
    {
        return InterlockedIncrement64(&refCount_);
    }

    LONGLONG DecrementReference()
    {
        return InterlockedDecrement64(&refCount_);
    }

private:
    volatile LONGLONG refCount_;

};


class IObject
{
public:
    
    virtual LONGLONG Addref() = 0;
    virtual LONGLONG Release() = 0;

protected:
    
    // Define the virtual destructor to prevent memory leaking.
    virtual ~IObject(){}

};

    template
    <
    class KTInterface,
    class RefCountedPolicy = KBaseRefCounter
    >
    class KObjectImpl : 
        public virtual RefCountedPolicy,
        public virtual KTInterface
    {
        typedef RefCountedPolicy RP;


    protected:
        // Implement constructor
        KObjectImpl() : RP()
        {
        }

    public:

        virtual LONGLONG Addref()
        {
            return RP::IncrementReference();
        }

        virtual LONGLONG Release()
        {
            LONGLONG lRet = 0;
            if ( 0 == (lRet = RP::DecrementReference()))
            {
                delete this;
            }

            return lRet;
        }

    };


#pragma warning(pop)
    
#endif
