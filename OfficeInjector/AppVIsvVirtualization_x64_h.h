

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0626 */
/* at Tue Jan 19 05:14:07 2038
 */
/* Compiler settings for AppVIsvVirtualization.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.01.0626 
    protocol : all , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */



/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 500
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */


#ifndef __AppVIsvVirtualization_x64_h_h__
#define __AppVIsvVirtualization_x64_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef DECLSPEC_XFGVIRT
#if _CONTROL_FLOW_GUARD_XFG
#define DECLSPEC_XFGVIRT(base, func) __declspec(xfg_virtual(base, func))
#else
#define DECLSPEC_XFGVIRT(base, func)
#endif
#endif

/* Forward Declarations */ 

/* header files for imported files */
#include "wtypesbase.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __AppVIsvVirtualization_INTERFACE_DEFINED__
#define __AppVIsvVirtualization_INTERFACE_DEFINED__

/* interface AppVIsvVirtualization */
/* [version][uuid] */ 

typedef struct _PATH_STRUCT
    {
    DWORD PathLength;
    /* [unique][size_is] */ const wchar_t *Path;
    } 	PATH_STRUCT;

int virtman_notification_server_notify_new_process( 
    /* [in] */ handle_t IDL_handle,
    /* [in] */ int Pid);

int virtman_notification_server_notify_new_child_process( 
    /* [in] */ handle_t IDL_handle,
    /* [in] */ int Pid,
    /* [in] */ PATH_STRUCT *PathStruct);



extern RPC_IF_HANDLE AppVIsvVirtualization_v1_0_c_ifspec;
extern RPC_IF_HANDLE AppVIsvVirtualization_v1_0_s_ifspec;
#endif /* __AppVIsvVirtualization_INTERFACE_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


