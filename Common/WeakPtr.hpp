#pragma once


// May be have an exception if incorrectly use weak.
// WeakPtr cannot be used after StrongPtr's reference dec to 0.
// So, when design to use weak ptr, must keep this in mind.


typename
    <
    typename KObjectT
    >
    class WeakPtr
    {

    public:
        WeakPtr():
            _p(NULL)
        {

        }

        WeakPtr( KObjectT* p):
            _p(NULL)
        {
            Init(p);
        }

        WeakPtr( const WeakPtr& rhs)
        {
            Init(rhs._p);
        }

        ~WeakPtr()
        {
                       
            KObjectT* p = _p;
            _p = NULL;

            if ( NULL != p)
            {
                // May be have an exception if incorrectly use weak.
                // WeakPtr cannot be used after StrongPtr's reference dec to 0.
                // So, when design to use weak ptr, must keep this in mind.

                __try
                {
                    p->Release(FALSE);
                }
                __except
                {
                    assert();
                    _p = NULL;
                }
            }
        }

    private:
        void Init( KObjectT* p )
        {
            // Increment internal object's reference counter 
            if ( NULL != p )
            {
                _p = p;

                // Increment external reference counter
                _p->Addref(FALSE);
            }
        }


    private:
        KObjectT* _p;
    };