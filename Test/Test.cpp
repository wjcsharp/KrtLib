// Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../Common/Object.h"
#include "../Common/StrongPtr.hpp"

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

int _tmain(int argc, _TCHAR* argv[])
{

    DriverPtr driverPtr = new TestDriver();

    {
        printf("driverPtr's Data is %d \n", driverPtr->_driverId);
        DriverPtr driverPtr2 = driverPtr;
    }

    printf("driverPtr's Data is %d \n", driverPtr->_driverId);

	return 0;
}

