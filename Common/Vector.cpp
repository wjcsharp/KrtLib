
#include "stdafx.h"



typedef void* PVOID;

template<class TClass>
class KAllocator
{
public:
    static PVOID Alloc()
    {
        return NULL;
    }

    static void Release(PVOID buffer)
    {
        return NULL;
    }
protected:
private:
};




template<class TClass,
         class TAllocator = KAllocator<TClass>>
class KArrary
{
public:
	KArrary()
    {
        TAllocator::Alloc();
    }
	~KArrary()
    {
        TAllocator::Release();
    }
protected:
	

private:
};

typedef KArrary<char> char_array;


//char_array.initial(100);