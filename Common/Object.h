#pragma once
#ifndef __cplusplus
#pragma message("################### The class only support cpp ###################")
#else

#pragma warning(push)
#pragma warning(disable:4100)

class KBaseRefCounted
{

public:
    KBaseRefCounted():
        m_strongCount(0),
        m_weakCount(0)
    {

    }
        
    LONGLONG IncStrongPointer()
    {
        return InterlockedIncrement64(&m_strongCount);
    }

    LONGLONG DecStrongPointer()
    {
        return InterlockedDecrement64(&m_strongCount);
    }

    LONGLONG IncWeakPointer()
    {
        return InterlockedIncrement64(&m_weakCount);
    }

    LONGLONG DecWeakPointer()
    {
        return InterlockedDecrement64(&m_weakCount);
    }

private:
    volatile LONGLONG m_strongCount;
    volatile LONGLONG m_weakCount;

};


class IObject
{
public:
    
    virtual LONGLONG Addref(BOOLEAN strong = TRUE) = 0;
    virtual LONGLONG Release(BOOLEAN strong = TRUE) = 0;

protected:
    
    // Define the virtual destructor to prevent memory leaking.
    virtual ~IObject(){}

};

    template
    <
        class RefCountedPolicy = KBaseRefCounted
    >
    class KObjectImpl : 
        public RefCountedPolicy,
        public IObject
    {
        typedef RefCountedPolicy RP;

    public:

        // Implement constructor
        KObjectImpl() : RP()
        {
        }

        virtual LONGLONG Addref(BOOLEAN strong)
        {
            if (strong)
            {
                return RP::IncStrongPointer();
            }
            else
            {
                return RP::IncWeakPointer();
            }
        }

        virtual LONGLONG Release(BOOLEAN strong)
        {
            LONGLONG lRet = 0;
            if (strong)
            {
                if ( 0 == (lRet = RP::DecStrongPointer()))
                {
                    delete this;
                }
                return lRet;
            }
            else
            {
                return RP::DecWeakPointer();
            }
        }


    };


#pragma warning(pop)
    
#endif
