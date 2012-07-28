// JmcSite.h : Declaration of the CJmcSite

#ifndef __JMCSITE_H_
#define __JMCSITE_H_

#include "resource.h"       // main symbols
#include <ActivScp.h>
#include "../include/activdbg.h"

/////////////////////////////////////////////////////////////////////////////
// CJmcSite
class ATL_NO_VTABLE CJmcSite : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CJmcSite, &CLSID_JmcSite>,
	public ISupportErrorInfo,
	public IActiveScriptSite,
	public IActiveScriptSiteWindow,
    public IActiveScriptSiteDebug, 
	public IDispatchImpl<IJmcSite, &IID_IJmcSite, &LIBID_TTCOREEXLib>
{
public:
	CJmcSite()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_JMCSITE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CJmcSite)
	COM_INTERFACE_ENTRY(IJmcSite)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IActiveScriptSite)
	COM_INTERFACE_ENTRY(IActiveScriptSiteWindow)
    COM_INTERFACE_ENTRY_IID(IID_IActiveScriptSiteDebug, IActiveScriptSiteDebug)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

// IActiveScriptSite
		virtual HRESULT STDMETHODCALLTYPE GetLCID( 
            /* [out] */ LCID __RPC_FAR *plcid);
        
        virtual HRESULT STDMETHODCALLTYPE GetItemInfo( 
            /* [in] */ LPCOLESTR pstrName,
            /* [in] */ DWORD dwReturnMask,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppiunkItem,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppti);
        
        virtual HRESULT STDMETHODCALLTYPE GetDocVersionString( 
            /* [out] */ BSTR __RPC_FAR *pbstrVersion);
        
        virtual HRESULT STDMETHODCALLTYPE OnScriptTerminate( 
            /* [in] */ const VARIANT __RPC_FAR *pvarResult,
            /* [in] */ const EXCEPINFO __RPC_FAR *pexcepinfo);
        
        virtual HRESULT STDMETHODCALLTYPE OnStateChange( 
            /* [in] */ SCRIPTSTATE ssScriptState);
        
        virtual HRESULT STDMETHODCALLTYPE OnScriptError( 
            /* [in] */ IActiveScriptError __RPC_FAR *pscripterror);
        
        virtual HRESULT STDMETHODCALLTYPE OnEnterScript( void);
        
        virtual HRESULT STDMETHODCALLTYPE OnLeaveScript( void);

// IActiveScriptSiteWindow

		virtual HRESULT STDMETHODCALLTYPE GetWindow( 
            /* [out] */ HWND __RPC_FAR *phwnd);
        
        virtual HRESULT STDMETHODCALLTYPE EnableModeless( 
            /* [in] */ BOOL fEnable);

// IActiveScriptSiteDebug
        virtual HRESULT STDMETHODCALLTYPE GetDocumentContextFromPosition( 
            /* [in] */ DWORD dwSourceContext,
            /* [in] */ ULONG uCharacterOffset,
            /* [in] */ ULONG uNumChars,
            /* [out] */ IDebugDocumentContext __RPC_FAR *__RPC_FAR *ppsc) {
            
           ULONG ulStartPos = 0;
           HRESULT hr;

           if (m_pDebugDocHelper)
           {
              hr = m_pDebugDocHelper->GetScriptBlockInfo(dwSourceContext, NULL, &ulStartPos, NULL);
              hr = m_pDebugDocHelper->CreateDebugDocumentContext(ulStartPos + uCharacterOffset, uNumChars, ppsc);
           }
           else
           {
              hr = E_NOTIMPL;
           }

	        return hr;
        };

        
        virtual HRESULT STDMETHODCALLTYPE GetApplication( 
            /* [out] */ IDebugApplication __RPC_FAR *__RPC_FAR *ppda) {
            
/*            if ( !bAllowDebug || !ppda ) {
                *ppda = NULL;
                return E_NOTIMPL;
            }
*/
            if (!ppda)
               {
                  return E_INVALIDARG;
               }

               // bugbug - should addref to this ?
            if ( m_pDebugApp ) {
                m_pDebugApp.CopyTo(ppda );
	            return S_OK;
            } else {
                *ppda = NULL;
                return E_NOTIMPL;
            }
        };

        
        virtual HRESULT STDMETHODCALLTYPE GetRootApplicationNode( 
            /* [out] */ IDebugApplicationNode __RPC_FAR *__RPC_FAR *ppdanRoot) {

           if (!ppdanRoot)
           {
              return E_INVALIDARG;
           }

           if (m_pDebugDocHelper)
           {
              return m_pDebugDocHelper->GetDebugApplicationNode(ppdanRoot);
           }

           return E_NOTIMPL;

        };
        
        virtual HRESULT STDMETHODCALLTYPE OnScriptErrorDebug( 
            /* [in] */ IActiveScriptErrorDebug __RPC_FAR *pErrorDebug,
            /* [out] */ BOOL __RPC_FAR *pfEnterDebugger,
            /* [out] */ BOOL __RPC_FAR *pfCallOnScriptErrorWhenContinuing) {

           if (pfEnterDebugger)
           {
              *pfEnterDebugger = bAllowDebug;
           }
           if (pfCallOnScriptErrorWhenContinuing)
           {
              *pfCallOnScriptErrorWhenContinuing = FALSE;
              if ( !bAllowDebug ) 
                OnScriptError(pErrorDebug);
           }
           return S_OK;

        };




// IJmcSite
public:
		CComPtr<IActiveScriptParse>	    m_ScriptParser;
		CComPtr<IActiveScript>		    m_ScriptEngine;

        CComPtr<IProcessDebugManager>   m_pdm;
        CComPtr<IDebugApplication>      m_pDebugApp  ;
        DWORD                           m_dwAppCookie;
        CComPtr<IDebugDocumentHelper>   m_pDebugDocHelper;


        HWND m_hwndSiteWindow;

        BOOL InitSite(HWND hwndParentWindow, LPCSTR strScript, GUID guidEngine);
    
};
extern CComObject<CJmcSite>* pSite;

#endif //__JMCSITE_H_
