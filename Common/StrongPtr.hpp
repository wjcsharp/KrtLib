#pragma once

template
    <

    /**
    * Template parameter must support IObject interface.
    */
        typename KObjectT
    >
class StrongPtr
{

    /**
     * Constructors and Destructors.
     */

public:

    StrongPtr():
        _p(NULL)
    {

    }

    StrongPtr(KObjectT* p):
        _p(NULL)
    {
        /**
		 * Enables 'Strong<T> ptr = NULL;' syntax. Always initializes internal pointer to NULL
		 * no matter what argument is passed. This prevents user from mistakes like:
		 *  StrongPtr<T> ptr((T*)123);
		 *  StrongPtr<T> ptr(new T()); 
		 */        
        Init(p);
    }

    /**
    * Copy constructor implementation.
    */
    StrongPtr( const StrongPtr & rhs )
        : _p(NULL)
    {
        Init(rhs._p);
    }

    /**
    * Destructor provided to maintain(decrement) references to object
    */
    ~StrongPtr()
    {
        KObjectT* p = _p;
        _p = NULL;

        if ( NULL != p)
        {
            p->Release(TRUE);
        }
    }


    KObjectT& operator*()
    {
        return *_p;
    }

    const KObjectT& operator*() const
    {
        return *_p;
    }

    KObjectT* operator ->()
    {
        return _p;
    }

    const KObjectT* operator ->() const
    {
        return _p;
    }


private:
    void Init(KObjectT * p)
    {
        // Increment internal object's reference counter 
        if ( NULL != p )
        {
            _p = p;

            // Increment external reference counter
            _p->Addref(TRUE);
        }
    }

private:
    KObjectT* _p;
};