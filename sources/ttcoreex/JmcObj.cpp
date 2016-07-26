// JmcObj.cpp : Implementation of CJmcObj
#include "stdafx.h"
#include "Ttcoreex.h"
#include "tintin.h"
#include "JmcSite.h"
#include "JmcObj.h"
#include "telnet.h"

extern jmc_special_variable jmc_vars[JMC_SPECIAL_VARIABLES_NUM];
extern GET_WNDSIZE_FUNC GetWindowSize;
extern SET_WNDSIZE_FUNC SetWindowSize;

/////////////////////////////////////////////////////////////////////////////
// CJmcObj

STDMETHODIMP CJmcObj::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IJmcObj
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CJmcObj::ShowMe(BSTR varText, BSTR varColor)
{
    wchar_t result[BUFFER_SIZE];

    if ( varColor ) {
        add_codes(varText, result, varColor);
    } else {
        wcscpy(result, varText);
	}

	if ( bLogAsUserSeen ) {
		log(processLine(result));
		log(L"\n");
		add_line_to_scrollbuffer(result);
	}
	tintin_puts2(result);

	return S_OK;
}

STDMETHODIMP CJmcObj::Send(BSTR bstrLine)
{
    if ( bstrLine ) {
        write_line_mud(bstrLine);
    }

	return S_OK;
}

STDMETHODIMP CJmcObj::Beep()
{
    ::MessageBeep(MB_OK );

	return S_OK;
}

STDMETHODIMP CJmcObj::Parse(BSTR bstrCommand)
{
	parse_input(bstrCommand, TRUE);

	return S_OK;
}

STDMETHODIMP CJmcObj::Output(BSTR bstrText, BSTR bstrColor)
{
    wchar_t text[BUFFER_SIZE], result[BUFFER_SIZE];

    prepare_actionalias(bstrText, text, sizeof(text)/sizeof(wchar_t)); 
    if ( bstrColor ) {
        add_codes(text, result, bstrColor);
        tintin_puts3(result, 0);
    } else {
        tintin_puts3(text, 0);
	}
	return S_OK;
}

STDMETHODIMP CJmcObj::SetTimer(LONG ID, LONG Interval, LONG preInterval)
{
    std::map<int, TIMER*>::iterator pos;

    pos = TIMER_LIST.find(ID);

    if ( pos != TIMER_LIST.end() ) {
        TIMER* ptm = pos->second;
        ptm ->m_nInterval = Interval;
        ptm ->m_nPreinterval = preInterval;
        ptm ->m_dwLastTickCount= GetTickCount();
    } else {
        TIMER* ptm = new TIMER(ID, Interval, preInterval);
        TIMER_LIST[ID] = ptm;
    }

	return S_OK;
}

STDMETHODIMP CJmcObj::KillTimer(LONG ID)
{
    std::map<int, TIMER*>::iterator pos;

    pos = TIMER_LIST.find(ID);
    if ( pos != TIMER_LIST.end() ) {
        TIMER* ptm =  (*pos).second;
        TIMER_LIST.erase (pos);
        delete ptm;
    }
	return S_OK;
}

STDMETHODIMP CJmcObj::get_IsConnected(BOOL *pVal)
{
	*pVal = (BOOL)MUDSocket;

	return S_OK;
}

STDMETHODIMP CJmcObj::Eval(BSTR bstrCommand)
{
    ParseScriptlet2( bstrCommand);
	return S_OK;
}


STDMETHODIMP CJmcObj::RegisterHandler(BSTR bstrEvent, BSTR bstrCode)
{                     
/*    BSTR bstrName;
	EXCEPINFO    ei;
    HRESULT hr ;

    hr = pSite->m_ScriptParser->AddScriptlet(NULL, 
                         bstrCode,
                         L"jmc",
                         NULL,
                         bstrEvent,
                         L"", 
                         0, 
                         0,
                         0, 
                         &bstrName,
                         &ei);

	return hr;                                                              
*/
    wchar_t event[256];
    wcscpy(event, bstrEvent);
    _wcsupr(event);

    if ( *event == L'C' && !wcsicmp(event, L"CONNECTED" ) ){
        m_bstrEventsHandlers[ID_Connected] = bstrCode;
    } else 
    if ( *event == L'C' && !wcsicmp(event, L"CONNECTLOST" ) ){
        m_bstrEventsHandlers[ID_ConnectLost] = bstrCode;
    } else 
    if ( *event == L'I' && !wcsicmp(event, L"INCOMING" ) ){
        m_bstrEventsHandlers[ID_Incoming] = bstrCode;
    } else 
    if ( *event == L'I' && !wcsicmp(event, L"INPUT" ) ) {
        m_bstrEventsHandlers[ID_Input] = bstrCode;
    } else 
    if ( *event == L'T' && !wcsicmp(event, L"TIMER" ) ){
        m_bstrEventsHandlers[ID_Timer] = bstrCode;
    } else 
    if ( *event == L'P' && !wcsicmp(event, L"PRETIMER" ) ){
        m_bstrEventsHandlers[ID_PreTimer] = bstrCode;
    } else 
    if ( *event == L'D' && !wcsicmp(event, L"DISCONNECTED" ) ){
        m_bstrEventsHandlers[ID_Disconnected] = bstrCode;
    } else 
    if ( *event == L'L' && !wcsicmp(event, L"LOAD" ) ) {
        m_bstrEventsHandlers[ID_Load] = bstrCode;
    } else 
    if ( *event == L'U' && !wcsicmp(event, L"UNLOAD" ) ){
        m_bstrEventsHandlers[ID_Unload] = bstrCode;
    } else
	if ( *event == L'P' && !wcsicmp(event, L"PROMPT" ) ){
        m_bstrEventsHandlers[ID_Prompt] = bstrCode;
    } else 
	if ( *event == L'T' && !wcsicmp(event, L"TELNET" ) ){
        m_bstrEventsHandlers[ID_Telnet] = bstrCode;
    } else 
        return E_INVALIDARG;
    return S_OK;
}

STDMETHODIMP CJmcObj::get_Event(LONG nIndex, VARIANT *pVal)
{
	if ( nIndex >= PARAMS_MAXCOUNT ) 
        return E_INVALIDARG;

    VariantClear(pVal);
    VariantCopy(pVal, &m_pvarEventParams[nIndex]);
	return S_OK;
}

STDMETHODIMP CJmcObj::put_Event(LONG nIndex, VARIANT newVal)
{
	if ( nIndex >= PARAMS_MAXCOUNT ) 
        return E_INVALIDARG;
    
    m_pvarEventParams[nIndex].Clear();
    m_pvarEventParams[nIndex].Copy(&newVal);
	return S_OK;
}


STDMETHODIMP CJmcObj::DropEvent()
{
	// m_pvarEventParams[0].Clear();
    m_bDropped = TRUE;
	return S_OK;
}

STDMETHODIMP CJmcObj::Connect(BSTR bstrAddress, BSTR bstrPort)
{
	if ( MUDSocket ) 
        return S_OK;

    connect_mud(bstrAddress, bstrPort);

	return S_OK;
}

STDMETHODIMP CJmcObj::Disconnect()
{
    if ( MUDSocket ) {
        zap_command(L"\0");
    }

	return S_OK;
}

STDMETHODIMP CJmcObj::get_Profile(BSTR *pVal)
{
	*pVal = m_bstrProfile.Copy ();
	return S_OK;
}

STDMETHODIMP CJmcObj::SetHotkey(BSTR bstrKey, BSTR bstrCommand)
{
    CComBSTR arg = bstrKey;
    arg += L" ";
    arg += bstrCommand;
    SetHotKey(arg);

	return S_OK;
}

STDMETHODIMP CJmcObj::get_CommandChar(BSTR *pVal)
{
    wchar_t buf[2] = {cCommandChar, 0};
    CComBSTR bstr(buf);

    *pVal = bstr.Copy ();

	return S_OK;
}


STDMETHODIMP CJmcObj::SetStatus(LONG StatusNum, BSTR bstrText, BSTR bstrColor)
{
    wchar_t text[BUFFER_SIZE];

    if ( bstrColor ) {
        swprintf(text, L"%d {%s} {%s}", StatusNum , bstrText, bstrColor);
        status_command(text);
    } else {
        swprintf(text, L"%d {%s}", StatusNum , bstrText);
        status_command(text);
    }
	return S_OK;
}


STDMETHODIMP CJmcObj::SetVar(BSTR bstrVarName, BSTR bstrValue, BOOL bGlobal)
{
	if ( !bstrVarName ) 
        return E_INVALIDARG;

    VAR_INDEX ind = VarList.find (bstrVarName);
    VAR* pvar;
    if ( ind != VarList.end() ) {
        pvar = ind->second;
        pvar->m_strVal = bstrValue;
    }
    else {
        pvar = new VAR(bstrValue);
        VarList[bstrVarName] = pvar;
    }
    pvar->m_bGlobal = bGlobal;

	return S_OK;
}

STDMETHODIMP CJmcObj::GetVar(BSTR bstrVarName, BSTR *bstrRet)
{
	if ( !bstrVarName ) 
        return E_INVALIDARG;

    VAR_INDEX ind = VarList.find (bstrVarName);
    CComBSTR ret("");
    if ( ind != VarList.end() ) {
        VAR* pvar = ind->second;
        ret = pvar->m_strVal.data();
    } else {
		wchar_t specialVariableValue[BUFFER_SIZE];
		bool found = false;

		for(int i = 0; i < JMC_SPECIAL_VARIABLES_NUM; i++)
			if (!wcscmp(bstrVarName, jmc_vars[i].name)) {
				(*(jmc_vars[i].jmcfn))(specialVariableValue);
				ret = specialVariableValue;
				found = true;
				break;
			}
		if (!found) {
			 if (get_oob_variable(bstrVarName, specialVariableValue, sizeof(specialVariableValue) / sizeof(wchar_t) - 1) > 0)
				ret = specialVariableValue;
		}
	}
    *bstrRet = ret.Copy ();

    
	return S_OK;
}

//vls-begin// #quit
STDMETHODIMP CJmcObj::Quit()
{
    quit_command(L"");

    return S_OK;
}
//vls-end//

//vls-begin// #run
STDMETHODIMP CJmcObj::Run(BSTR bstrCmdLine, BSTR bstrParams)
{
    wchar_t cmd[BUFFER_SIZE];
    wchar_t params[BUFFER_SIZE];

    if (bstrParams && bstrParams[0]) {
        wcscpy(cmd, bstrCmdLine);
        wcscpy(params, bstrParams);
    } else {
        int i, j;
        wchar_t arg[BUFFER_SIZE];
        wchar_t *p = bstrCmdLine;
        for (i = j = 0; p[i] && j<BUFFER_SIZE-3; i++) {
            if (p[i] == L'\\')
                arg[j++] = L'\\';
            arg[j++] = p[i];
        }
        arg[j] = L'\0';
        p = get_arg_in_braces(arg,cmd,STOP_SPACES,sizeof(cmd)/sizeof(wchar_t)-1);
        get_arg_in_braces(p,params,WITH_SPACES,sizeof(params)/sizeof(wchar_t)-1);
    }

    if ( *cmd ) {
        ShellExecute(NULL, NULL, cmd, params[0] ? params : NULL, NULL, SW_SHOW);
    }

    return S_OK;
}
//vls-end//

//vls-begin// #play
STDMETHODIMP CJmcObj::Play(BSTR bstrFileName)
{
    wchar_t fn[MAX_PATH+2];
    MakeAbsolutePath(fn, bstrFileName, szBASE_DIR);
    PlaySound(fn, NULL, SND_ASYNC | SND_FILENAME);

    return S_OK;
}
//vls-end//

//* en
STDMETHODIMP CJmcObj::wOutput(LONG wndNum, BSTR bstrText, BSTR bstrColor)
{
    wchar_t text[BUFFER_SIZE], result[BUFFER_SIZE];
	
	if(wndNum>MAX_OUTPUT || wndNum<0)
   	  return S_OK;

    prepare_actionalias(bstrText ,text, sizeof(text)/sizeof(wchar_t)); 
    if ( bstrColor ) {
        add_codes(text, result, bstrColor);
        tintin_puts3(result, wndNum);
    } else 
        tintin_puts3(text, wndNum);
	return S_OK;
}
//*/en

STDMETHODIMP CJmcObj::wGetWidth(LONG wndNum, LONG *nWidth)
{
	if ( wndNum < 0 || wndNum >= MAX_OUTPUT ) 
		wndNum = -1;

	int w, h;
	GetWindowSize(wndNum, w, h);
	
	*nWidth = w;

	return S_OK;
}

STDMETHODIMP CJmcObj::wGetHeight(LONG wndNum, LONG *nHeight)
{
	if ( wndNum < 0 || wndNum >= MAX_OUTPUT ) 
		wndNum = -1;

	int w, h;
	GetWindowSize(wndNum, w, h);
	
	*nHeight = h;

	return S_OK;
}

STDMETHODIMP CJmcObj::DoTelnet(LONG Command, LONG Option, BSTR bstrData)
{
	if (!bstrData)
		send_telnet_command(Command, Option);
	else if (Command != TN_SB)
		return E_INVALIDARG;

	send_telnet_subnegotiation((unsigned char)Option, bstrData, SysStringLen(bstrData), false);

	return S_OK;
}

STDMETHODIMP CJmcObj::ToText(BSTR bstrANSI, BSTR *bstrText)
{
	if ( !bstrANSI ) 
        return E_INVALIDARG;

	CComBSTR ret("");
	wchar_t *ansi = bstrANSI;
	int len = SysStringLen(bstrANSI);
	wchar_t *text = new wchar_t[len + 1];
	remove_ansi_codes(ansi, text);
	ret = (text);

	*bstrText = ret.Copy();

	delete[] text;

	return S_OK;
}

STDMETHODIMP  CJmcObj::ToColored(BSTR bstrANSI, BSTR *bstrColored)
{
	if ( !bstrANSI ) 
        return E_INVALIDARG;

	CComBSTR ret("");
	wchar_t *ansi = bstrANSI;
	int len = SysStringLen(bstrANSI);
	wchar_t *colored = new wchar_t[len + 1];
	int state = 37;
	convert_ansi_to_colored(ansi, colored, len, state);
	ret = (colored);

	*bstrColored = ret.Copy();

	delete[] colored;

	return S_OK;
}

STDMETHODIMP CJmcObj::FromColored(BSTR bstrColored, BSTR *bstrANSI)
{
	if ( !bstrColored ) 
        return E_INVALIDARG;

	CComBSTR ret(L"");
	wchar_t *colored = bstrColored;
	
	int len = SysStringLen(bstrColored);
	int count = 0, i;
	for (i = 0; i < len; i++)
		if (colored[i] == L'&')
			count++;
	//&? => ESC[?;??m
	len += count * 5;
	wchar_t *ansi = new wchar_t[len + 1];
	convert_colored_to_ansi(colored, ansi, len);
	ret = (ansi);

	*bstrANSI = ret.Copy();

	delete[] ansi;

	return S_OK;
}

STDMETHODIMP CJmcObj::MSDP2GMCP(BSTR bstrMSDP, BSTR *bstrGMCP)
{
	if ( !bstrMSDP ) 
        return E_INVALIDARG;

	CComBSTR ret(L"");
	wchar_t *msdp = bstrMSDP;
	
	int len = SysStringLen(bstrMSDP);
	wstring gmcp = convert_msdp2gmcp(msdp, len);
	ret = gmcp.c_str();

	*bstrGMCP = ret.Copy();

	return S_OK;
}

STDMETHODIMP CJmcObj::GMCP2MSDP(BSTR bstrGMCP, BSTR *bstrMSDP)
{
	if ( !bstrGMCP ) 
        return E_INVALIDARG;

	CComBSTR ret(L"");
	wchar_t *gmcp = bstrGMCP;
	
	int len = SysStringLen(bstrGMCP);
	wstring msdp = convert_gmcp2msdp(gmcp, len);
	ret = msdp.c_str();

	*bstrMSDP = ret.Copy();

	return S_OK;
}

STDMETHODIMP CJmcObj::MSSP2GMCP(BSTR bstrMSSP, BSTR *bstrGMCP)
{
	if ( !bstrMSSP ) 
        return E_INVALIDARG;

	CComBSTR ret(L"");
	wchar_t *mssp = bstrMSSP;
	
	int len = SysStringLen(bstrMSSP);
	wstring gmcp = convert_mssp2gmcp(mssp, len);
	ret = gmcp.c_str();

	*bstrGMCP = ret.Copy();

	return S_OK;
}
