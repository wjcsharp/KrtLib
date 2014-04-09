#include "stdafx.h"


#define DEFAULT_MEM_TAG 'mtkW'

void * __cdecl malloc( size_t size , POOL_TYPE poolType = NonPagedPool, ULONG tag = DEFAULT_MEM_TAG);
void * __cdecl calloc(ULONG count, size_t size, POOL_TYPE poolType = NonPagedPool, ULONG tag = DEFAULT_MEM_TAG);
void __cdecl free(PVOID buffer);
void * __cdecl realloc(void *mem, size_t size, POOL_TYPE poolType = NonPagedPool, ULONG tag = DEFAULT_MEM_TAG);


void * __cdecl malloc( size_t size , POOL_TYPE poolType, ULONG tag)
{
    return ExAllocatePoolWithTag( poolType, size, tag );
}

void * __cdecl calloc(ULONG count, size_t size, POOL_TYPE poolType, ULONG tag)
{
    void *mem = malloc(count * size, poolType, tag);

    if (!mem) return NULL;

    RtlZeroMemory(mem, count * size);

    return mem;
}


void __cdecl free(PVOID buffer)
{
    ExFreePool(buffer);
}

void * __cdecl realloc(void *mem, size_t size, POOL_TYPE poolType, ULONG tag)
{
    free(mem);
    return malloc(size, poolType, tag);
}

inline void * __cdecl operator new(size_t nSize)
{
    return nSize ? malloc(nSize) : NULL;
}

inline void * __cdecl operator new(size_t nSize, POOL_TYPE iType)
{ 
    return nSize ? malloc(nSize, iType) : NULL;
}

inline void * __cdecl operator new(size_t nSize, POOL_TYPE iType, ULONG tag)
{ 
    return nSize ? malloc(nSize, iType, tag) : NULL;
}

inline void __cdecl operator delete(void* p, POOL_TYPE iType) 
{
    UNREFERENCED_PARAMETER(iType);
    if (p)
    {
        ExFreePool(p);
    }
}

inline void __cdecl operator delete(void* p)
{ 
    if (p)
    {
        ExFreePool(p);
    }
}

inline void __cdecl operator delete [] (void* p)
{ 
    if (p)
    {
        ExFreePool(p);
    }
}

// 
// typedef void (__cdecl *PVFV)(void);
// static void __cdecl onexitinit( void );
// static PVFV __cdecl onexit( PVFV func );
// static void __cdecl callPVFVArray( PVFV * pfbegin, PVFV * pfend );
// static void __cdecl doexit( int code, int quick, int retcaller );
// // #pragma section(".CRT",long,read,nopage)
// // #pragma section(".CRT$XCA",long,read,nopage)
// // #pragma section(".CRT$XCZ",long,read,nopage)
// 
// extern "C"
// {
//     PVFV __crtXia[];
//     PVFV __crtXiz[];
// 
//     PVFV __crtXca[]; // c++
//     PVFV __crtXcz[];
// 
//     PVFV __crtXpa[];
//     PVFV __crtXpz[];
// 
//     PVFV __crtXta[];
//     PVFV __crtXtz[];
// }
// 
// #pragma data_seg(".CRT$XCA")
// PVFV __crtXca[] = { NULL }; // c++
// #pragma data_seg(".CRT$XCZ")
// PVFV __crtXcz[] = { NULL };
// #pragma data_seg()
// 
// #pragma comment(linker, "/merge:.CRT=.rdata")
// 
// int __cdecl atexit( PVFV func )
// {
//     return (onexit(func) == NULL) ? -1 : 0;
// }
// 
// void __cdecl callPVFVArray( PVFV * pfbegin, PVFV * pfend )
// {// TerryM CODEREVIEW: Check and verify params
//     while( pfbegin < pfend )
//     {
//         if ( *pfbegin != NULL )
//             (**pfbegin)();
//         ++pfbegin;
//     }
// }
// 
// typedef struct
// {
//     LIST_ENTRY  link;
//     PVFV        exitFunc;
// } EXIT_FUNC_LIST, *PEXIT_FUNC_LIST;
// 
// LIST_ENTRY exitList;
// 
// 
// void __cdecl onexitinit( void )
// {
//     InitializeListHead(&exitList);
// }
// 
// PVFV __cdecl onexit( PVFV func )
// {
//     // TerryM CODEREVIEW check params do this for the rest of the routines that pass in pointers.  We need to verify that they are okay before we use them
//     PEXIT_FUNC_LIST pFuncListEntry = 
//         (PEXIT_FUNC_LIST)malloc(sizeof(EXIT_FUNC_LIST));
// 
//     if( !pFuncListEntry )
//     {
//         return NULL;
//     }
//     pFuncListEntry->exitFunc = func;
//     InsertHeadList(&exitList, &pFuncListEntry->link);
//     return func;
// }
// 
// void drainExit()
// {
//     PEXIT_FUNC_LIST pFuncListEntry;
//     while(!IsListEmpty(&exitList))
//     {
//         pFuncListEntry = (PEXIT_FUNC_LIST)RemoveHeadList(&exitList);
//         if((PVOID)pFuncListEntry != (PVOID)&pFuncListEntry->link )
//         {
//             KdPrint(( "SYPT> Error: drainExit() list corrupted?\n" ));
//         }
//         if(pFuncListEntry->exitFunc)
//         {
//             pFuncListEntry->exitFunc();
//         }
//         free(pFuncListEntry);
//     }
// }
// 
// void __cdecl doexit( int code, int quick, int retcaller )
// {
//     UNREFERENCED_PARAMETER(code);
//     UNREFERENCED_PARAMETER(retcaller);
//     if( NULL == quick )
//     {
//         drainExit();
//     }
//     return;
// }
// 
// 
// #ifdef __cplusplus
// extern "C" {
// #endif
//     extern NTSTATUS C_DriverEntry( PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath );
//     NTSTATUS DriverEntry( PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath );
// 
// #ifdef __cplusplus
// }
// #endif
// 
// #ifdef ALLOC_PRAGMA
// #pragma alloc_text(INIT, DriverEntry)
// #endif
// 
// static VOID Cpp_DriverUnload( PDRIVER_OBJECT DriverObject );
// static PDRIVER_UNLOAD clientUnload = NULL;
// 
// 
// NTSTATUS DriverEntry(	IN PDRIVER_OBJECT DriverObject, 
//                      IN PUNICODE_STRING RegistryPath )
// {
//     NTSTATUS Status = STATUS_SUCCESS;
// 
//     DbgBreakPoint();
// 
//     DbgPrint("[xc %p-%p]\n", __crtXca, __crtXcz);
// 
//     onexitinit();
//     callPVFVArray( __crtXca, __crtXcz );
// 
//     Status = C_DriverEntry( DriverObject, RegistryPath );
//     if( !NT_SUCCESS( Status ) )
//     {
//         doexit( 0, 0, 1 );
//         return Status;
//     }
// 
//     if( DriverObject->DriverUnload )
//     {
//         clientUnload = DriverObject->DriverUnload;
//         DriverObject->DriverUnload = Cpp_DriverUnload;
//     }
// 
//     return STATUS_SUCCESS;
// }
// 
// static VOID Cpp_DriverUnload( PDRIVER_OBJECT DriverObject )
// {	
//     // We just do logging here, the unloading actions will still be performed.
//     if( NULL == DriverObject )
//     {
//         KdPrint(("Driver unload received NULL driver object!"));
//     }
// 
//     if( NULL != clientUnload )
//     {
//         clientUnload( DriverObject );
//     }
//     doexit( 0, 0, 1 );
//     return;
// }
// 
