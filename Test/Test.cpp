// Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../Common/Object.h"
#include "../Common/StrongPtr.hpp"
#include "../Common/WeakPtr.hpp"

class TestDriver : 
    public KObjectImpl<>
{
public:
    TestDriver():
        _driverId(NULL)
    {
        printf("Test Driver constructor.\n");
    }

    ~TestDriver()
    {
        printf("Test Driver destructor.\n");
    }

protected:
private:

public:

    int _driverId;

};

typedef StrongPtr<TestDriver> DriverPtr;
typedef WeakPtr<TestDriver> DriverWeakPtr;


void TestStrong(DriverPtr& driverPtr)
{
    printf("driverPtr's Data is %d \n", driverPtr->_driverId);
    DriverPtr driverPtr2 = driverPtr;
}

void TestWeak(DriverWeakPtr& driverPtrW)
{
    DriverPtr driverS = driverPtrW;
}

int _tmain(int argc, _TCHAR* argv[])
{

    DriverPtr driverPtr = new TestDriver();

    DriverWeakPtr driverWeakPtr = driverPtr;
    
    TestWeak(driverWeakPtr);

	return 0;
}

