#pragma once


// May be have an exception if incorrectly use weak.
// WeakPtr cannot be used after StrongPtr's reference dec to 0.
// So, when design to use weak ptr, must keep this in mind.


template
    <
    typename KObjectT
    >
class WeakPtr
{

    // WeakPtr doesn't support convert from object pointer directly.
    // Because when convert from object pointer directly, when use the pointer later,
    // it may be already released. Look reference at the implement of boost::weak_ptr.
    // It just can be convert from strong ptr or weakptr.

    WeakPtr( KObjectT* p):
        p_(NULL),
        refCounter_( NULL )
    {
    }

public:
    WeakPtr():
        p_(NULL),
        refCounter_( NULL )
    {

    }

    WeakPtr( const WeakPtr& rhs):
        p_(NULL),
        refCounter_( NULL )
    {
        Init(rhs.p_, rhs.refCounter_);
    }

    WeakPtr( const StrongPtr<KObjectT>& rhs):
        p_(NULL),
        refCounter_( NULL )
    {
        Init( rhs.p_, rhs.refCounter_ );
    }

    ~WeakPtr()
    {
        p_ = NULL;

        if ( NULL != refCounter_ )
        {
            if ( ( 0 == refCounter_->DecWeakCount()) &&
                ( 0 == refCounter_->GetStrongCount()))
            {
                delete refCounter_;
                refCounter_ = NULL;
            }
        }
    }

private:

    void Init(KObjectT * p, DualRefCounter* refCounter)
    {
        p_ = p;

        assert( NULL == refCounter_ );
        if (NULL != refCounter)
        {
            refCounter->IncWeakCount();
            refCounter_ = refCounter;
        }
    }

private:

    // Establish friendship between all specializations of StrongPtr and WeakPtr
    template< typename KObjectT > friend class StrongPtr;


    KObjectT*       p_;
    DualRefCounter* refCounter_;
};