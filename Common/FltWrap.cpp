#include "stdafx.h"

#include "FltWrap.h"

NTSTATUS KFltFilter::Start()
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    
    if ( NULL != m_hFilter )
    {
        return STATUS_ALREADY_REGISTERED;
    }
    
    return status;
}
