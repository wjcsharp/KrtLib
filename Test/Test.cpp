// Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../Common/Object.h"
#include "../Common/StrongPtr.hpp"
#include "../Common/WeakPtr.hpp"


class IDriver : public virtual IObject
{
public:
    virtual LONG GetDriverId() const = 0;
    virtual VOID SetDriverId(LONG& driverId) = 0;

};

typedef StrongPtr<IDriver> IDriverPtrS;
typedef WeakPtr<IDriver> IDriverPtrW;

//////////////////////////////////////////////////////////////////////////

class IProcessNotify : public virtual IObject
{
public:
    virtual VOID OnProcessNotify(
        IN HANDLE   parentId,
        IN HANDLE   processId,
        IN BOOLEAN  create) = 0;
};

typedef StrongPtr<IProcessNotify> IProcessNotifyPtrS;
typedef WeakPtr<IProcessNotify> IProcessNotifyPtrW;

//////////////////////////////////////////////////////////////////////////

class KDriver : 
    public KObjectImpl<IDriver>,
    public KObjectImpl<IProcessNotify>
{
public:
    KDriver():
        _driverId(NULL)
    {
        printf("Test Driver constructor.\n");
    }

    ~KDriver()
    {
        printf("Test Driver destructor.\n");
    }

    /*
     *	Implement of IDriver
     */
    
    LONG GetDriverId() const
    {
        return _driverId;
    }
    
    VOID SetDriverId(LONG& driverId)
    {
        _driverId = driverId;
    }

    /*
     *	Implement of IProcessNotify
     */

    VOID OnProcessNotify(
        IN HANDLE   parentId,
        IN HANDLE   processId,
        IN BOOLEAN  create
        )
    {
        if (create)
        {
            printf(" Process[%d] create process [%d].\n", parentId, processId);
        }
        else
        {
            printf(" Process[%d] exit.\n", processId);
        }

    }

protected:
private:

public:

    int _driverId;

};

typedef StrongPtr<KDriver> DriverPtrS;
typedef WeakPtr<KDriver> DriverPtrW;

//////////////////////////////////////////////////////////////////////////


void TestStrong(IDriverPtrS& driverPtr)
{
    printf("driverPtr's Data is %d \n", driverPtr->GetDriverId());
   // DriverPtr driverPtr2 = driverPtr;
}

void TestProcessNotify(DriverPtrS& driverPtrW)
{
    HANDLE hParent = reinterpret_cast<HANDLE>(0x04);
    HANDLE hProcess = reinterpret_cast<HANDLE>(0x20);

    driverPtrW->OnProcessNotify( hParent, hProcess, TRUE);
}

void TestWeak(DriverPtrW& driverPtrW)
{
    //DriverPtr driverS = driverPtrW;
    driverPtrW;

}

int _tmain(int argc, _TCHAR* argv[])
{


    /** IDriver* driver = new Driver();
     *  
    -*/

    DriverPtrS driverPtr = new KDriver();

    TestProcessNotify(driverPtr);

    //DriverWeakPtr driverWeakPtr = driverPtr;
    
    //TestStrong(driverPtr);
    //TestWeak(driverWeakPtr);

	return 0;
}

