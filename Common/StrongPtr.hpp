#pragma once

template< typename KObjectT > class WeakPtr;


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
        p_( NULL ),
        refCounter_( NULL )
    {
    }

    StrongPtr(KObjectT* p):
        p_( NULL ),
        refCounter_( NULL )
    {
        /**
		 * Enables 'Strong<T> ptr = NULL;' syntax. Always initializes internal pointer to NULL
		 * no matter what argument is passed. This prevents user from mistakes like:
		 *  StrongPtr<T> ptr((T*)123);
		 *  StrongPtr<T> ptr(new T()); 
		 */
        Init(p, NULL);
    }

    /**
    * Copy constructor implementation.
    */
    StrongPtr( const StrongPtr & rhs ):
        p_( NULL ),
        refCounter_( NULL )
    {
        Init(rhs.p_, rhs.refCounter_);
    }


    /**
    * Copy constructor implementation from WeakPtr,
    * 
    */
    StrongPtr( const WeakPtr<KObjectT>& rhs ):
        p_( NULL ),
        refCounter_( NULL )
    {
        // ! This may be has exception in multi thread environment. 
        // ! Because, there is no lock to guard the p_ release.
        // ! User should use weak and strong correctly. 
        // ! Should correct this code in future.

        assert( 0 != rhs.refCounter_->GetStrongCount() );

        if ( 0 != rhs.refCounter_->GetStrongCount() )
        {
            Init(rhs.p_, rhs.refCounter_);
        }

    }

    /**
    * Destructor provided to maintain(decrement) references to object
    */
    ~StrongPtr()
    {

        if ( NULL != refCounter_ )
        {
            if ( ( 0 == refCounter_->DecStrongCount()) &&
                 ( 0 == refCounter_->GetWeakCount()))
            {
                delete refCounter_;
                refCounter_ = NULL;
            }
        }

        if ( NULL != p_)
        {
            KObjectT* p = p_;
            p_ = NULL;

            p->Release();
        }

    }


    KObjectT& operator*()
    {
        return *p_;
    }

    const KObjectT& operator*() const
    {
        return *p_;
    }

    KObjectT* operator ->()
    {
        return p_;
    }

    const KObjectT* operator ->() const
    {
        return p_;
    }


private:
    void InitCounter()
    {
        refCounter_ = new DualRefCounter( true );
    }

    void ReleaseCounter()
    {
        if ( NULL != refCounter_ )
        {
            // Delete counter if there are no weak references to the object.
            delete refCounter_;
            refCounter_ = NULL;
        }
    }

    void Init(KObjectT * p, DualRefCounter* refCounter)
    {
        if ( NULL != p )
        {
            // If object isn't null, we should inc object's ref and init daul refcount
            p->Addref();
            p_ = p;
        }

        if ( NULL != p_ )
        {
            assert( NULL == refCounter_ );
            if (NULL != refCounter)
            {
                refCounter->IncStrongCount();
                refCounter_ = refCounter;
            }
            else
            {
                refCounter_ = new DualRefCounter(TRUE);
                assert( NULL != refCounter_ );
            }
        }

    }

private:

    // Establish friendship between all specializations of StrongPtr and WeakPtr
    template< typename KObjectT > friend class WeakPtr;


    KObjectT*       p_;
    DualRefCounter*   refCounter_;
};