#pragma once

#ifndef __cplusplus

#pragma message("################### The class only support cpp ###################")

#else


class KDriver : KObjectImpl<KBaseRefCounted>
{
public:
    KDriver(
        _In_ PDRIVER_OBJECT DriverObject,
        _In_ PUNICODE_STRING RegistryPath)
    {
        FLT_ASSERT(NULL != DriverObject);
        FLT_ASSERT(NULL != RegistryPath);
    }

    ~KDriver()
    {

    }

protected:
private:

    PDRIVER_OBJECT m_pDriverObject;
};

class KDevice
{
public:
    KDevice(){}
    ~KDevice(){}
private:
    KDriver* m_pDriver;
};

class KFltFilter
{
public:

    KFltFilter(_In_ KDriver* driver);
    ~KFltFilter();

    NTSTATUS Start();
    NTSTATUS Stop();

private:

    PFLT_FILTER m_hFilter;
};



class KFltStreamContext
{
public:
protected:
private:
};

class KFltStreamHandleContext
{
public:
};


#endif
