#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#pragma warning(push)
#pragma warning(disable:4510 4512 4610)
#include <fltKernel.h>
#pragma warning(pop)

    NTSTATUS RunTest();

#ifdef __cplusplus
}
#endif