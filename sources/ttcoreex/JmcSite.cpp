// JmcSite.cpp : Implementation of CJmcSite
#include "stdafx.h"
#include "Ttcoreex.h"
#include "tintin.h"
#include "JmcSite.h"
#include "JmcObj.h"

/////////////////////////////////////////////////////////////////////////////
// CJmcSite

const IID IID_IProcessDebugManager = {0x51973C2F,0xCB0C,0x11D0,{0xB5,0xC9,0x00,0xA0,0x24,0x4A,0x0E,0x7A}};
const IID IID_IActiveScriptSiteDebug = {0x51973C11,0xCB0C,0x11D0,{0xB5,0xC9,0x00,0xA0,0x24,0x4A,0x0E,0x7A}};


STDMETHODIMP CJmcSite::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IJmcSite
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))    
			return S_OK;
	}
	return S_FALSE;
}

// IActiveScriptSite

HRESULT STDMETHODCALLTYPE CJmcSite::GetLCID(/* [out] */ LCID __RPC_FAR *plcid)
{
	return E_NOTIMPL;
}
        
HRESULT STDMETHODCALLTYPE CJmcSite::GetItemInfo(/* [in] */ LPCOLESTR pstrName,
									  /* [in] */ DWORD dwReturnMask,
								      /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppiunkItem,
									  /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppti)
{
	

	if (dwReturnMask & SCRIPTINFO_ITYPEINFO)
	{
		if (!ppti)
			return E_INVALIDARG;
		*ppti = NULL;
	}

	if (dwReturnMask & SCRIPTINFO_IUNKNOWN)
	{
		if (!ppiunkItem)
			return E_INVALIDARG;
		*ppiunkItem = NULL;
	}

	// Global object
	if (!_wcsicmp(L"JMC", pstrName))
	{
		if (dwReturnMask & SCRIPTINFO_ITYPEINFO)
		{


            ITypeLib *ptLib = 0;
            HRESULT hr = LoadTypeLib(L"ttcoreex.dll", &ptLib);
            // ASSERT(SUCCEEDED(hr));
           // Initialize your IActiveScriptSite implementation with your
           // object's ITypeInfo...
            hr = ptLib->GetTypeInfoOfGuid(CLSID_JmcObj, ppti);
            ptLib->Release();
            
            
        }
		
		if (dwReturnMask & SCRIPTINFO_IUNKNOWN)
		{
			*ppiunkItem = (IDispatch*)pJmcObj;//pThis->GetIDispatch(TRUE);
			(*ppiunkItem)->AddRef();    // because returning
		}
		return S_OK;
	}
	
    
    return E_INVALIDARG;
}
        
HRESULT STDMETHODCALLTYPE CJmcSite::GetDocVersionString(/* [out] */ BSTR __RPC_FAR *pbstrVersion)
{
	return E_NOTIMPL;
}

        
HRESULT STDMETHODCALLTYPE CJmcSite::OnScriptTerminate(/* [in] */ const VARIANT __RPC_FAR *pvarResult,
											/* [in] */ const EXCEPINFO __RPC_FAR *pexcepinfo)
{
	return S_OK;
}
        
HRESULT STDMETHODCALLTYPE CJmcSite::OnStateChange(/* [in] */ SCRIPTSTATE ssScriptState)
{
	return S_OK;
}
        
HRESULT STDMETHODCALLTYPE CJmcSite::OnScriptError(/* [in] */ IActiveScriptError __RPC_FAR *pscripterror)
{
	EXCEPINFO ei;
	DWORD     dwSrcContext;
	ULONG     ulLine;
	LONG      ichError;
	BSTR      bstrLine = NULL;
	wchar_t* strError;

	pscripterror->GetExceptionInfo(&ei);
	pscripterror->GetSourcePosition(&dwSrcContext, &ulLine, &ichError);
	pscripterror->GetSourceLineText(&bstrLine);
	


    if ( bstrLine ) {
        strError = new wchar_t[SysStringLen(ei.bstrDescription) + SysStringLen(bstrLine)+100];
        swprintf(strError, rs::rs(1088),  ei.bstrSource, ei.bstrDescription, ulLine, (int)ei.wCode, ei.scode, bstrLine);
    }
    else {
        strError = new wchar_t[SysStringLen(ei.bstrDescription) +100];
        swprintf(strError, rs::rs(1089),  ei.bstrSource, ei.bstrDescription, ulLine, (int)ei.wCode, ei.scode);
    }
    switch (nScripterrorOutput ) {
    case 1: 
        {
            wchar_t err [BUFFER_SIZE];
            wcscpy ( err, DEFAULT_BEGIN_COLOR);
            wcscat ( err, L";31m");
            wcscat ( err, strError);
            wcscat ( err, DEFAULT_END_COLOR);
            tintin_puts2(err);
        }
        break;
    case 2:
        {
            wchar_t err [BUFFER_SIZE];
            wcscpy ( err, DEFAULT_BEGIN_COLOR);
            wcscat ( err, L";31m");
            wcscat ( err, strError);
            wcscat ( err, DEFAULT_END_COLOR);
//vls-begin// multiple output
//            tintin_puts3(err);
            tintin_puts3(err, 0);
//vls-end//
        }
        break;
    default:
        ::MessageBox(m_hwndSiteWindow, strError, rs::rs(1090), MB_OK | MB_ICONSTOP );
    }
    
    delete[] strError;

	return S_OK;
	
}
        
HRESULT STDMETHODCALLTYPE CJmcSite::OnEnterScript( void)
{
	return S_OK;
}
        
HRESULT STDMETHODCALLTYPE CJmcSite::OnLeaveScript( void)
{
	return S_OK;
}

// IActiveScriptSiteWindow

HRESULT STDMETHODCALLTYPE CJmcSite::GetWindow(/*[out]*/HWND __RPC_FAR *phwnd)
{
	*phwnd = m_hwndSiteWindow;
	return S_OK;
}
        
HRESULT STDMETHODCALLTYPE CJmcSite::EnableModeless(/*[in]*/BOOL fEnable)
{
	return S_OK;
}


// const GUID CLSID_VBScript = { 0xb54f3741, 0x5b07, 0x11cf, { 0xa4, 0xb0,  0x0,  0xaa,  0x0,  0x4a,  0x55,  0xe8 } };


BOOL CJmcSite::InitSite(HWND hwndParentWindow, LPWSTR strScript, GUID guidEngine)
{
    m_hwndSiteWindow  = hwndParentWindow;


    if ( m_ScriptEngine != NULL ) {
        pJmcObj->Fire_Unload();
        m_ScriptEngine->SetScriptState (SCRIPTSTATE_CLOSED);
        m_ScriptEngine.Release ();
    }

    if ( m_ScriptParser != NULL ) 
        m_ScriptParser.Release ();
    
    if ( m_pDebugDocHelper != NULL ) {
        m_pDebugDocHelper->Detach();
        m_pDebugDocHelper.Release();
        m_pDebugDocHelper = NULL;
    }

    // here  we create and init engine

	// XX ActiveX Scripting XX 
	HRESULT hr = CoCreateInstance(
		guidEngine, NULL, CLSCTX_INPROC_SERVER, IID_IActiveScript, (void **)&m_ScriptEngine);
    if (FAILED(hr)) {
		// If this happens, the scripting engine is probably not properly registered
        tintin_puts(rs::rs(1091) );
		return FALSE;
    }
	// Script Engine must support IActiveScriptParse for us to use it
	hr = m_ScriptEngine->QueryInterface(IID_IActiveScriptParse, (void **)&m_ScriptParser);
	
    if (FAILED(hr)) {
		// If this happens, the scripting engine is probably not properly registered
        tintin_puts(rs::rs(1092) );
		return FALSE;
    }
	
	hr = m_ScriptEngine->SetScriptSite((IActiveScriptSite*)this); 

    if (FAILED(hr)) {
		// If this happens, the scripting engine is probably not properly registered
        tintin_puts(rs::rs(1093) );
		return FALSE;
    }

    // debug specific 
    if ( m_pdm ) {
        hr = m_pdm->CreateDebugDocumentHelper(NULL, &m_pDebugDocHelper);
        hr = m_pDebugDocHelper->Init(m_pDebugApp, L"JMC scriptlet", L"JMC scriptlet code", TEXT_DOC_ATTR_READONLY);
        hr = m_pDebugDocHelper->Attach(NULL);
    }
	
    hr = m_ScriptParser->InitNew();


	hr = m_ScriptEngine->AddNamedItem(L"jmc", SCRIPTITEM_ISVISIBLE | SCRIPTITEM_ISSOURCE | SCRIPTITEM_GLOBALMEMBERS);
    hr = m_ScriptEngine->SetScriptState(SCRIPTSTATE_INITIALIZED);

    
	EXCEPINFO    ei;

    wchar_t dir[MAX_PATH+2];
    GetCurrentDirectory(MAX_PATH, dir);
    SetCurrentDirectory(szBASE_DIR);
	hr = m_ScriptParser->ParseScriptText(strScript, NULL, NULL, NULL, 0, 0, 0, NULL, &ei);
    SetCurrentDirectory(dir);

    hr = m_ScriptEngine->SetScriptState(SCRIPTSTATE_STARTED);

    hr = m_ScriptEngine->SetScriptState(SCRIPTSTATE_CONNECTED);



    pJmcObj->Fire_Load();
    
    return TRUE;

}


void script_command(wchar_t *arg)
{
    ParseScript(arg);
}
