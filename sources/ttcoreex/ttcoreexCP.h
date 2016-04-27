#ifndef _TTCOREEXCP_H_
#define _TTCOREEXCP_H_


template <class T>
class CProxy_IJmcObjEvents : public IConnectionPointImpl<T, &DIID__IJmcObjEvents, CComDynamicUnkArray>
{
	//Warning this class may be recreated by the wizard.
public:
	HRESULT Fire_Connected()
	{
		T* pT = static_cast<T*>(this);

        if ( pT->m_bstrEventsHandlers[ID_Connected].Length()  ) {
            ParseScriptlet2((BSTR)(pT->m_bstrEventsHandlers[ID_Connected]));
            return S_OK;
        }

		CComVariant varResult;
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				VariantClear(&varResult);
				DISPPARAMS disp = { NULL, NULL, 0, 0 };
				pDispatch->Invoke(0x1, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		return varResult.scode;
	
	}
	HRESULT Fire_Disconnected()
	{
		T* pT = static_cast<T*>(this);

        if ( pT->m_bstrEventsHandlers[ID_Disconnected].Length()  ) {
            ParseScriptlet2((BSTR)(pT->m_bstrEventsHandlers[ID_Disconnected]));
            return S_OK;
        }
        
		CComVariant varResult;
        int nConnectionIndex;
		int nConnections = m_vec.GetSize();
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				VariantClear(&varResult);
				DISPPARAMS disp = { NULL, NULL, 0, 0 };
				pDispatch->Invoke(0x7, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		return varResult.scode;
	
	}
	HRESULT Fire_ConnectLost()
	{
		T* pT = static_cast<T*>(this);
        if ( pT->m_bstrEventsHandlers[ID_ConnectLost].Length()  ) {
            ParseScriptlet2((BSTR)(pT->m_bstrEventsHandlers[ID_ConnectLost]));
            return S_OK;
        }

        CComVariant varResult;
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				VariantClear(&varResult);
				DISPPARAMS disp = { NULL, NULL, 0, 0 };
				pDispatch->Invoke(0x2, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		return varResult.scode;
	
	}
	BOOL Fire_Incoming()
	{
		T* pT = static_cast<T*>(this);

        pT->m_bDropped = FALSE;
        BOOL bRet = TRUE;

        if ( pT->m_bstrEventsHandlers[ID_Incoming].Length()  ) {
            ParseScriptlet2((BSTR)(pT->m_bstrEventsHandlers[ID_Incoming]));
            if ( pT->m_bDropped || pT->m_pvarEventParams[0].vt != VT_BSTR) {
                bRet = FALSE;
            }
            return bRet;
        }

        CComVariant varResult;
		int nConnectionIndex;
		CComVariant* pvars = new CComVariant[1];
		int nConnections = m_vec.GetSize();
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
                VariantClear(&varResult);
				pvars[0] = pT->m_pvarEventParams[0].bstrVal;


                DISPPARAMS disp = { pvars, NULL, 1, 0 };
				pDispatch->Invoke(0x3, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
                if ( pT->m_bDropped || pT->m_pvarEventParams[0].vt != VT_BSTR) {
                    bRet = FALSE;
                }
			}
		}
		delete[] pvars;
		return bRet;
	
	}
	BOOL Fire_MultiIncoming()
	{
		T* pT = static_cast<T*>(this);

        pT->m_bDropped = FALSE;
        BOOL bRet = TRUE;

		CComVariant varResult;
		int nConnectionIndex;
		CComVariant* pvars = new CComVariant[1];
		int nConnections = m_vec.GetSize();
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
                VariantClear(&varResult);
				pvars[0] = pT->m_pvarEventParams[0].bstrVal;


                DISPPARAMS disp = { pvars, NULL, 1, 0 };
				pDispatch->Invoke(0xA, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
                if ( pT->m_bDropped || pT->m_pvarEventParams[0].vt != VT_BSTR) {
                    bRet = FALSE;
                }
			}
		}
		delete[] pvars;
		return bRet;
	
	}

    BOOL Fire_Input()
	{
		T* pT = static_cast<T*>(this);

        pT->m_bDropped = FALSE;
        BOOL bRet = TRUE;

        if ( pT->m_bstrEventsHandlers[ID_Input].Length() ) {
            ParseScriptlet2((BSTR)(pT->m_bstrEventsHandlers[ID_Input]));
            if ( pT->m_bDropped || pT->m_pvarEventParams[0].vt != VT_BSTR) {
                bRet = FALSE;
            }
            return bRet;
        }

        CComVariant varResult;
		int nConnectionIndex;
		CComVariant* pvars = new CComVariant[1];
		int nConnections = m_vec.GetSize();
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				VariantClear(&varResult);
				pvars[0] = pT->m_pvarEventParams[0].bstrVal;
                
                DISPPARAMS disp = { pvars, NULL, 1, 0 };
				pDispatch->Invoke(0x4, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
                if ( pT->m_bDropped  || pT->m_pvarEventParams[0].vt != VT_BSTR) {
                    bRet = FALSE;
                }
			}
		}
		delete[] pvars;
		return bRet;
	
	}


    HRESULT Fire_Timer(LONG ID)
	{
		T* pT = static_cast<T*>(this);
		
        if ( pT->m_bstrEventsHandlers[ID_Timer].Length()  ) {
            ParseScriptlet2((BSTR)(pT->m_bstrEventsHandlers[ID_Timer]));
            return S_OK;
        }
        
        CComVariant varResult;

        int nConnectionIndex;
		CComVariant* pvars = new CComVariant[1];
		int nConnections = m_vec.GetSize();
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				VariantClear(&varResult);
				pvars[0]=ID ;
				
                pT->m_pvarEventParams[0] = ID ;
                
                DISPPARAMS disp = { pvars, NULL, 1, 0 };
				pDispatch->Invoke(0x5, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		delete[] pvars;
		return varResult.scode;
	
	}

    HRESULT Fire_PreTimer(LONG ID)
	{
		T* pT = static_cast<T*>(this);
        if ( pT->m_bstrEventsHandlers[ID_PreTimer].Length()  ) {
            ParseScriptlet2((BSTR)(pT->m_bstrEventsHandlers[ID_PreTimer]));
            return S_OK;
        }

		CComVariant varResult;
        int nConnectionIndex;
		CComVariant* pvars = new CComVariant[1];
		int nConnections = m_vec.GetSize();
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				VariantClear(&varResult);
				pvars[0].vt = VT_I4;
                pvars[0].lVal =ID ;

                pT->m_pvarEventParams[0].vt =VT_I4;;
                pT->m_pvarEventParams[0].lVal =ID ;
                
                DISPPARAMS disp = { pvars, NULL, 1, 0 };
				pDispatch->Invoke(0x6, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		delete[] pvars;
		return varResult.scode;
	
	}

	HRESULT Fire_Load()
	{
		T* pT = static_cast<T*>(this);
        if ( pT->m_bstrEventsHandlers[ID_Load].Length()  ) {
            ParseScriptlet2((BSTR)(pT->m_bstrEventsHandlers[ID_Load]));
            return S_OK;
        }

		CComVariant varResult;
        int nConnectionIndex;
		int nConnections = m_vec.GetSize();
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				VariantClear(&varResult);
				DISPPARAMS disp = { NULL, NULL, 0, 0 };
				pDispatch->Invoke(0x8, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		return varResult.scode;
	
	}

	HRESULT Fire_Unload()
	{
		T* pT = static_cast<T*>(this);
        if ( pT->m_bstrEventsHandlers[ID_Unload].Length()  ) {
            ParseScriptlet2((BSTR)(pT->m_bstrEventsHandlers[ID_Unload]));
            return S_OK;
        }

		CComVariant varResult;
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				VariantClear(&varResult);
				DISPPARAMS disp = { NULL, NULL, 0, 0 };
				HRESULT hr = pDispatch->Invoke(0x9, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		return varResult.scode;
	
	}

	BOOL Fire_Prompt()
	{
		T* pT = static_cast<T*>(this);

        pT->m_bDropped = FALSE;
        BOOL bRet = TRUE;

        if ( pT->m_bstrEventsHandlers[ID_Prompt].Length()  ) {
            ParseScriptlet2((BSTR)(pT->m_bstrEventsHandlers[ID_Prompt]));
            if ( pT->m_bDropped || pT->m_pvarEventParams[0].vt != VT_BSTR) {
                bRet = FALSE;
            }
            return bRet;
        }

        CComVariant varResult;
		int nConnectionIndex;
		CComVariant* pvars = new CComVariant[1];
		int nConnections = m_vec.GetSize();
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
                VariantClear(&varResult);
				pvars[0] = pT->m_pvarEventParams[0].bstrVal;


                DISPPARAMS disp = { pvars, NULL, 1, 0 };
				pDispatch->Invoke(0xB, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
                if ( pT->m_bDropped || pT->m_pvarEventParams[0].vt != VT_BSTR) {
                    bRet = FALSE;
                }
			}
		}
		delete[] pvars;
		return bRet;
	
	}

};
#endif
