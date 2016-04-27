/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Wed Apr 27 16:43:10 2016
 */
/* Compiler settings for D:\github\jmc\jmc-msvs6\sources\ttcoreex\ttcoreex.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __ttcoreex_h__
#define __ttcoreex_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IJmcSite_FWD_DEFINED__
#define __IJmcSite_FWD_DEFINED__
typedef interface IJmcSite IJmcSite;
#endif 	/* __IJmcSite_FWD_DEFINED__ */


#ifndef __IJmcObj_FWD_DEFINED__
#define __IJmcObj_FWD_DEFINED__
typedef interface IJmcObj IJmcObj;
#endif 	/* __IJmcObj_FWD_DEFINED__ */


#ifndef __JmcSite_FWD_DEFINED__
#define __JmcSite_FWD_DEFINED__

#ifdef __cplusplus
typedef class JmcSite JmcSite;
#else
typedef struct JmcSite JmcSite;
#endif /* __cplusplus */

#endif 	/* __JmcSite_FWD_DEFINED__ */


#ifndef ___IJmcObjEvents_FWD_DEFINED__
#define ___IJmcObjEvents_FWD_DEFINED__
typedef interface _IJmcObjEvents _IJmcObjEvents;
#endif 	/* ___IJmcObjEvents_FWD_DEFINED__ */


#ifndef __JmcObj_FWD_DEFINED__
#define __JmcObj_FWD_DEFINED__

#ifdef __cplusplus
typedef class JmcObj JmcObj;
#else
typedef struct JmcObj JmcObj;
#endif /* __cplusplus */

#endif 	/* __JmcObj_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IJmcSite_INTERFACE_DEFINED__
#define __IJmcSite_INTERFACE_DEFINED__

/* interface IJmcSite */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IJmcSite;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B41BA78F-8ED5-11D4-92AE-0001027ED904")
    IJmcSite : public IDispatch
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct IJmcSiteVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IJmcSite __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IJmcSite __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IJmcSite __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IJmcSite __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IJmcSite __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IJmcSite __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IJmcSite __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } IJmcSiteVtbl;

    interface IJmcSite
    {
        CONST_VTBL struct IJmcSiteVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IJmcSite_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IJmcSite_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IJmcSite_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IJmcSite_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IJmcSite_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IJmcSite_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IJmcSite_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IJmcSite_INTERFACE_DEFINED__ */


#ifndef __IJmcObj_INTERFACE_DEFINED__
#define __IJmcObj_INTERFACE_DEFINED__

/* interface IJmcObj */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IJmcObj;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DDF78224-8F80-11D4-92AE-0001027ED904")
    IJmcObj : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ShowMe( 
            /* [in] */ BSTR varText,
            /* [optional][in] */ BSTR varColor) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Send( 
            /* [in] */ BSTR bstrLine) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Beep( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Parse( 
            /* [in] */ BSTR bstrCommand) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Output( 
            /* [in] */ BSTR bstrText,
            /* [optional][in] */ BSTR bstrColor) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetTimer( 
            /* [in] */ LONG ID,
            /* [in] */ LONG Interval,
            /* [optional][in] */ LONG preInterval) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE KillTimer( 
            /* [in] */ LONG ID) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_IsConnected( 
            /* [retval][out] */ BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Eval( 
            /* [in] */ BSTR bstrCommand) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE RegisterHandler( 
            /* [in] */ BSTR bstrEvent,
            /* [in] */ BSTR bstrCode) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Event( 
            /* [optional][in] */ LONG nIndex,
            /* [retval][out] */ VARIANT __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Event( 
            /* [optional][in] */ LONG nIndex,
            /* [in] */ VARIANT newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Connect( 
            /* [in] */ BSTR bstrAddress,
            /* [in] */ BSTR bstrPort) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE DropEvent( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Disconnect( void) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Profile( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetHotkey( 
            /* [in] */ BSTR bstrKey,
            /* [in] */ BSTR bstrCommand) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CommandChar( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetStatus( 
            /* [in] */ LONG StatusNum,
            /* [in] */ BSTR bstrText,
            /* [optional][in] */ BSTR bstrColor) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetVar( 
            /* [in] */ BSTR bstrVarName,
            /* [in] */ BSTR bstrValue,
            /* [optional][in] */ BOOL bGlobal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetVar( 
            /* [in] */ BSTR bstrVarName,
            /* [retval][out] */ BSTR __RPC_FAR *bstrRet) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Quit( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Run( 
            /* [in] */ BSTR bstrCmdLine,
            /* [optional][in] */ BSTR bstrParams) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Play( 
            /* [in] */ BSTR bstrFileName) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE wOutput( 
            /* [in] */ LONG wndNum,
            /* [in] */ BSTR bstrText,
            /* [optional][in] */ BSTR bstrColor) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IJmcObjVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IJmcObj __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IJmcObj __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IJmcObj __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IJmcObj __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IJmcObj __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IJmcObj __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IJmcObj __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ShowMe )( 
            IJmcObj __RPC_FAR * This,
            /* [in] */ BSTR varText,
            /* [optional][in] */ BSTR varColor);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Send )( 
            IJmcObj __RPC_FAR * This,
            /* [in] */ BSTR bstrLine);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Beep )( 
            IJmcObj __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Parse )( 
            IJmcObj __RPC_FAR * This,
            /* [in] */ BSTR bstrCommand);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Output )( 
            IJmcObj __RPC_FAR * This,
            /* [in] */ BSTR bstrText,
            /* [optional][in] */ BSTR bstrColor);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetTimer )( 
            IJmcObj __RPC_FAR * This,
            /* [in] */ LONG ID,
            /* [in] */ LONG Interval,
            /* [optional][in] */ LONG preInterval);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *KillTimer )( 
            IJmcObj __RPC_FAR * This,
            /* [in] */ LONG ID);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IsConnected )( 
            IJmcObj __RPC_FAR * This,
            /* [retval][out] */ BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Eval )( 
            IJmcObj __RPC_FAR * This,
            /* [in] */ BSTR bstrCommand);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RegisterHandler )( 
            IJmcObj __RPC_FAR * This,
            /* [in] */ BSTR bstrEvent,
            /* [in] */ BSTR bstrCode);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Event )( 
            IJmcObj __RPC_FAR * This,
            /* [optional][in] */ LONG nIndex,
            /* [retval][out] */ VARIANT __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Event )( 
            IJmcObj __RPC_FAR * This,
            /* [optional][in] */ LONG nIndex,
            /* [in] */ VARIANT newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Connect )( 
            IJmcObj __RPC_FAR * This,
            /* [in] */ BSTR bstrAddress,
            /* [in] */ BSTR bstrPort);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DropEvent )( 
            IJmcObj __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Disconnect )( 
            IJmcObj __RPC_FAR * This);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Profile )( 
            IJmcObj __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetHotkey )( 
            IJmcObj __RPC_FAR * This,
            /* [in] */ BSTR bstrKey,
            /* [in] */ BSTR bstrCommand);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_CommandChar )( 
            IJmcObj __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetStatus )( 
            IJmcObj __RPC_FAR * This,
            /* [in] */ LONG StatusNum,
            /* [in] */ BSTR bstrText,
            /* [optional][in] */ BSTR bstrColor);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetVar )( 
            IJmcObj __RPC_FAR * This,
            /* [in] */ BSTR bstrVarName,
            /* [in] */ BSTR bstrValue,
            /* [optional][in] */ BOOL bGlobal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetVar )( 
            IJmcObj __RPC_FAR * This,
            /* [in] */ BSTR bstrVarName,
            /* [retval][out] */ BSTR __RPC_FAR *bstrRet);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Quit )( 
            IJmcObj __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Run )( 
            IJmcObj __RPC_FAR * This,
            /* [in] */ BSTR bstrCmdLine,
            /* [optional][in] */ BSTR bstrParams);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Play )( 
            IJmcObj __RPC_FAR * This,
            /* [in] */ BSTR bstrFileName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *wOutput )( 
            IJmcObj __RPC_FAR * This,
            /* [in] */ LONG wndNum,
            /* [in] */ BSTR bstrText,
            /* [optional][in] */ BSTR bstrColor);
        
        END_INTERFACE
    } IJmcObjVtbl;

    interface IJmcObj
    {
        CONST_VTBL struct IJmcObjVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IJmcObj_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IJmcObj_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IJmcObj_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IJmcObj_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IJmcObj_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IJmcObj_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IJmcObj_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IJmcObj_ShowMe(This,varText,varColor)	\
    (This)->lpVtbl -> ShowMe(This,varText,varColor)

#define IJmcObj_Send(This,bstrLine)	\
    (This)->lpVtbl -> Send(This,bstrLine)

#define IJmcObj_Beep(This)	\
    (This)->lpVtbl -> Beep(This)

#define IJmcObj_Parse(This,bstrCommand)	\
    (This)->lpVtbl -> Parse(This,bstrCommand)

#define IJmcObj_Output(This,bstrText,bstrColor)	\
    (This)->lpVtbl -> Output(This,bstrText,bstrColor)

#define IJmcObj_SetTimer(This,ID,Interval,preInterval)	\
    (This)->lpVtbl -> SetTimer(This,ID,Interval,preInterval)

#define IJmcObj_KillTimer(This,ID)	\
    (This)->lpVtbl -> KillTimer(This,ID)

#define IJmcObj_get_IsConnected(This,pVal)	\
    (This)->lpVtbl -> get_IsConnected(This,pVal)

#define IJmcObj_Eval(This,bstrCommand)	\
    (This)->lpVtbl -> Eval(This,bstrCommand)

#define IJmcObj_RegisterHandler(This,bstrEvent,bstrCode)	\
    (This)->lpVtbl -> RegisterHandler(This,bstrEvent,bstrCode)

#define IJmcObj_get_Event(This,nIndex,pVal)	\
    (This)->lpVtbl -> get_Event(This,nIndex,pVal)

#define IJmcObj_put_Event(This,nIndex,newVal)	\
    (This)->lpVtbl -> put_Event(This,nIndex,newVal)

#define IJmcObj_Connect(This,bstrAddress,bstrPort)	\
    (This)->lpVtbl -> Connect(This,bstrAddress,bstrPort)

#define IJmcObj_DropEvent(This)	\
    (This)->lpVtbl -> DropEvent(This)

#define IJmcObj_Disconnect(This)	\
    (This)->lpVtbl -> Disconnect(This)

#define IJmcObj_get_Profile(This,pVal)	\
    (This)->lpVtbl -> get_Profile(This,pVal)

#define IJmcObj_SetHotkey(This,bstrKey,bstrCommand)	\
    (This)->lpVtbl -> SetHotkey(This,bstrKey,bstrCommand)

#define IJmcObj_get_CommandChar(This,pVal)	\
    (This)->lpVtbl -> get_CommandChar(This,pVal)

#define IJmcObj_SetStatus(This,StatusNum,bstrText,bstrColor)	\
    (This)->lpVtbl -> SetStatus(This,StatusNum,bstrText,bstrColor)

#define IJmcObj_SetVar(This,bstrVarName,bstrValue,bGlobal)	\
    (This)->lpVtbl -> SetVar(This,bstrVarName,bstrValue,bGlobal)

#define IJmcObj_GetVar(This,bstrVarName,bstrRet)	\
    (This)->lpVtbl -> GetVar(This,bstrVarName,bstrRet)

#define IJmcObj_Quit(This)	\
    (This)->lpVtbl -> Quit(This)

#define IJmcObj_Run(This,bstrCmdLine,bstrParams)	\
    (This)->lpVtbl -> Run(This,bstrCmdLine,bstrParams)

#define IJmcObj_Play(This,bstrFileName)	\
    (This)->lpVtbl -> Play(This,bstrFileName)

#define IJmcObj_wOutput(This,wndNum,bstrText,bstrColor)	\
    (This)->lpVtbl -> wOutput(This,wndNum,bstrText,bstrColor)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IJmcObj_ShowMe_Proxy( 
    IJmcObj __RPC_FAR * This,
    /* [in] */ BSTR varText,
    /* [optional][in] */ BSTR varColor);


void __RPC_STUB IJmcObj_ShowMe_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IJmcObj_Send_Proxy( 
    IJmcObj __RPC_FAR * This,
    /* [in] */ BSTR bstrLine);


void __RPC_STUB IJmcObj_Send_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IJmcObj_Beep_Proxy( 
    IJmcObj __RPC_FAR * This);


void __RPC_STUB IJmcObj_Beep_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IJmcObj_Parse_Proxy( 
    IJmcObj __RPC_FAR * This,
    /* [in] */ BSTR bstrCommand);


void __RPC_STUB IJmcObj_Parse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IJmcObj_Output_Proxy( 
    IJmcObj __RPC_FAR * This,
    /* [in] */ BSTR bstrText,
    /* [optional][in] */ BSTR bstrColor);


void __RPC_STUB IJmcObj_Output_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IJmcObj_SetTimer_Proxy( 
    IJmcObj __RPC_FAR * This,
    /* [in] */ LONG ID,
    /* [in] */ LONG Interval,
    /* [optional][in] */ LONG preInterval);


void __RPC_STUB IJmcObj_SetTimer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IJmcObj_KillTimer_Proxy( 
    IJmcObj __RPC_FAR * This,
    /* [in] */ LONG ID);


void __RPC_STUB IJmcObj_KillTimer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IJmcObj_get_IsConnected_Proxy( 
    IJmcObj __RPC_FAR * This,
    /* [retval][out] */ BOOL __RPC_FAR *pVal);


void __RPC_STUB IJmcObj_get_IsConnected_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IJmcObj_Eval_Proxy( 
    IJmcObj __RPC_FAR * This,
    /* [in] */ BSTR bstrCommand);


void __RPC_STUB IJmcObj_Eval_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IJmcObj_RegisterHandler_Proxy( 
    IJmcObj __RPC_FAR * This,
    /* [in] */ BSTR bstrEvent,
    /* [in] */ BSTR bstrCode);


void __RPC_STUB IJmcObj_RegisterHandler_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IJmcObj_get_Event_Proxy( 
    IJmcObj __RPC_FAR * This,
    /* [optional][in] */ LONG nIndex,
    /* [retval][out] */ VARIANT __RPC_FAR *pVal);


void __RPC_STUB IJmcObj_get_Event_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IJmcObj_put_Event_Proxy( 
    IJmcObj __RPC_FAR * This,
    /* [optional][in] */ LONG nIndex,
    /* [in] */ VARIANT newVal);


void __RPC_STUB IJmcObj_put_Event_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IJmcObj_Connect_Proxy( 
    IJmcObj __RPC_FAR * This,
    /* [in] */ BSTR bstrAddress,
    /* [in] */ BSTR bstrPort);


void __RPC_STUB IJmcObj_Connect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IJmcObj_DropEvent_Proxy( 
    IJmcObj __RPC_FAR * This);


void __RPC_STUB IJmcObj_DropEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IJmcObj_Disconnect_Proxy( 
    IJmcObj __RPC_FAR * This);


void __RPC_STUB IJmcObj_Disconnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IJmcObj_get_Profile_Proxy( 
    IJmcObj __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IJmcObj_get_Profile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IJmcObj_SetHotkey_Proxy( 
    IJmcObj __RPC_FAR * This,
    /* [in] */ BSTR bstrKey,
    /* [in] */ BSTR bstrCommand);


void __RPC_STUB IJmcObj_SetHotkey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IJmcObj_get_CommandChar_Proxy( 
    IJmcObj __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IJmcObj_get_CommandChar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IJmcObj_SetStatus_Proxy( 
    IJmcObj __RPC_FAR * This,
    /* [in] */ LONG StatusNum,
    /* [in] */ BSTR bstrText,
    /* [optional][in] */ BSTR bstrColor);


void __RPC_STUB IJmcObj_SetStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IJmcObj_SetVar_Proxy( 
    IJmcObj __RPC_FAR * This,
    /* [in] */ BSTR bstrVarName,
    /* [in] */ BSTR bstrValue,
    /* [optional][in] */ BOOL bGlobal);


void __RPC_STUB IJmcObj_SetVar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IJmcObj_GetVar_Proxy( 
    IJmcObj __RPC_FAR * This,
    /* [in] */ BSTR bstrVarName,
    /* [retval][out] */ BSTR __RPC_FAR *bstrRet);


void __RPC_STUB IJmcObj_GetVar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IJmcObj_Quit_Proxy( 
    IJmcObj __RPC_FAR * This);


void __RPC_STUB IJmcObj_Quit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IJmcObj_Run_Proxy( 
    IJmcObj __RPC_FAR * This,
    /* [in] */ BSTR bstrCmdLine,
    /* [optional][in] */ BSTR bstrParams);


void __RPC_STUB IJmcObj_Run_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IJmcObj_Play_Proxy( 
    IJmcObj __RPC_FAR * This,
    /* [in] */ BSTR bstrFileName);


void __RPC_STUB IJmcObj_Play_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IJmcObj_wOutput_Proxy( 
    IJmcObj __RPC_FAR * This,
    /* [in] */ LONG wndNum,
    /* [in] */ BSTR bstrText,
    /* [optional][in] */ BSTR bstrColor);


void __RPC_STUB IJmcObj_wOutput_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IJmcObj_INTERFACE_DEFINED__ */



#ifndef __TTCOREEXLib_LIBRARY_DEFINED__
#define __TTCOREEXLib_LIBRARY_DEFINED__

/* library TTCOREEXLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_TTCOREEXLib;

EXTERN_C const CLSID CLSID_JmcSite;

#ifdef __cplusplus

class DECLSPEC_UUID("B41BA790-8ED5-11D4-92AE-0001027ED904")
JmcSite;
#endif

#ifndef ___IJmcObjEvents_DISPINTERFACE_DEFINED__
#define ___IJmcObjEvents_DISPINTERFACE_DEFINED__

/* dispinterface _IJmcObjEvents */
/* [helpstring][uuid] */ 


EXTERN_C const IID DIID__IJmcObjEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("DDF78226-8F80-11D4-92AE-0001027ED904")
    _IJmcObjEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _IJmcObjEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            _IJmcObjEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            _IJmcObjEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            _IJmcObjEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            _IJmcObjEvents __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            _IJmcObjEvents __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            _IJmcObjEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            _IJmcObjEvents __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } _IJmcObjEventsVtbl;

    interface _IJmcObjEvents
    {
        CONST_VTBL struct _IJmcObjEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _IJmcObjEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _IJmcObjEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _IJmcObjEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _IJmcObjEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _IJmcObjEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _IJmcObjEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _IJmcObjEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___IJmcObjEvents_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_JmcObj;

#ifdef __cplusplus

class DECLSPEC_UUID("DDF78225-8F80-11D4-92AE-0001027ED904")
JmcObj;
#endif
#endif /* __TTCOREEXLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long __RPC_FAR *, unsigned long            , VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long __RPC_FAR *, VARIANT __RPC_FAR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
