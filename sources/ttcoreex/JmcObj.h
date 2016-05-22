// JmcObj.h : Declaration of the CJmcObj

#ifndef __JMCOBJ_H_
#define __JMCOBJ_H_

#include "resource.h"       // main symbols
#include "ttcoreexCP.h"

#define PARAMS_MAXCOUNT 2

    enum {
        ID_Connected = 0, 
		ID_ConnectLost, 
		ID_Incoming,
		ID_Input, 
		ID_Timer,
		ID_PreTimer, 
		ID_Disconnected, 
		ID_Load, 
		ID_Unload,
		ID_Prompt,
		ID_TelnetSE

    };

/////////////////////////////////////////////////////////////////////////////
// CJmcObj
class ATL_NO_VTABLE CJmcObj : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CJmcObj, &CLSID_JmcObj>,
	public ISupportErrorInfo,
	public IProvideClassInfo2Impl<&CLSID_JmcObj, NULL, &LIBID_TTCOREEXLib>,
	public IConnectionPointContainerImpl<CJmcObj>,
	public IDispatchImpl<IJmcObj, &IID_IJmcObj, &LIBID_TTCOREEXLib>,
	public CProxy_IJmcObjEvents< CJmcObj >
{
public:
	CJmcObj()
	{
	}

    CComVariant m_pvarEventParams[PARAMS_MAXCOUNT];
    CComBSTR m_bstrProfile;
    BOOL m_bDropped;
    CComBSTR m_bstrEventsHandlers[20];


DECLARE_REGISTRY_RESOURCEID(IDR_JMCOBJ)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CJmcObj)
	COM_INTERFACE_ENTRY(IJmcObj)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(IProvideClassInfo)
	COM_INTERFACE_ENTRY(IProvideClassInfo2)
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()
BEGIN_CONNECTION_POINT_MAP(CJmcObj)
CONNECTION_POINT_ENTRY(DIID__IJmcObjEvents)
END_CONNECTION_POINT_MAP()


// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

// IJmcObj
public:
	STDMETHOD(GetVar)(/*[in]*/ BSTR bstrVarName, /*[out,retval]*/ BSTR* bstrRet);
	STDMETHOD(SetVar)(/*[in]*/ BSTR bstrVarName, /*[in]*/ BSTR bstrValue, /*[in,optional]*/ BOOL bGlobal);
	STDMETHOD(SetStatus)(/*[in]*/ LONG StatusNum, /*[in]*/ BSTR bstrText, /*[in, optional]*/ BSTR bstrColor);
	STDMETHOD(get_CommandChar)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(SetHotkey)(/*[in]*/ BSTR bstrKey, /*[in]*/ BSTR bstrCommand);
	STDMETHOD(get_Profile)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(Disconnect)();
	STDMETHOD(Connect)(/*[in]*/ BSTR bstrAddredd, /*[in]*/ BSTR bstrPort);
	STDMETHOD(DropEvent)();
	STDMETHOD(get_Event)(/*[in, optional]*/ LONG nIndex, /*[out, retval]*/ VARIANT *pVal);
	STDMETHOD(put_Event)(/*[in, optional]*/ LONG nIndex, /*[in]*/ VARIANT newVal);
	STDMETHOD(RegisterHandler)(/*[in]*/ BSTR bstrEvent, /*[in]*/ BSTR bstrCode);
	STDMETHOD(Eval)(/*[in]*/ BSTR bstrCommand);
	STDMETHOD(get_IsConnected)(/*[out, retval]*/ BOOL *pVal);
	STDMETHOD(KillTimer)(/*[in]*/ LONG ID);
	STDMETHOD(SetTimer)(/*[in]*/ LONG ID, /*[in]*/ LONG Interval, /*[in, optional]*/ LONG preInterval);
	STDMETHOD(Output)(/*[in]*/ BSTR bstrText, /*[in, optional]*/ BSTR bstrColor);
	STDMETHOD(Parse)(/*[in]*/ BSTR bstrCommand);
	STDMETHOD(Beep)();
	STDMETHOD(Send)(/*[in]*/ BSTR bstrLine);
	STDMETHOD(ShowMe)(/*[in]*/ BSTR varText, /*[in, optional]*/ BSTR varColor);
//vls-begin// #quit
    STDMETHOD(Quit)();
//vls-end//
//vls-begin// #run
    STDMETHOD(Run)(/*[in*/ BSTR bstrCmdLine, /*[in, optional]*/ BSTR bstrParams);
//vls-end//
//vls-begin// #play
    STDMETHOD(Play)(/*[in]*/ BSTR bstrFileName);
//vls-end//
	STDMETHOD(wOutput)(/*[in]*/ LONG wndNum, /*[in]*/ BSTR bstrText, /*[in, optional]*/ BSTR bstrColor);
	STDMETHOD(wGetWidth)(/*[in]*/ LONG wndNum, /*[out, retval]*/ LONG *nWidth);
	STDMETHOD(wGetHeight)(/*[in]*/ LONG wndNum, /*[out, retval]*/ LONG *nHeight);
	STDMETHOD(TelnetSB)(/*[in]*/ LONG Option, /*[in]*/ BSTR bstrData);
	STDMETHOD(ToText)(/*[in]*/ BSTR bstrANSI, /*[out, retval]*/ BSTR *bstrText);
	STDMETHOD(ToColored)(/*[in]*/ BSTR bstrANSI, /*[out, retval]*/ BSTR *bstrColored);
	STDMETHOD(FromColored)(/*[in]*/ BSTR bstrColored, /*[out, retval]*/ BSTR *bstrANSI);
};

extern CComObject<CJmcObj>* pJmcObj;
#endif //__JMCOBJ_H_
