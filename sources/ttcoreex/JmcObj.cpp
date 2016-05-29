// JmcObj.cpp : Implementation of CJmcObj
#include "stdafx.h"
#include "Ttcoreex.h"
#include "tintin.h"
#include "JmcSite.h"
#include "JmcObj.h"

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
    USES_CONVERSION;

    char result[BUFFER_SIZE];

    if ( varColor ) {
        add_codes(W2A(varText), result, W2A(varColor));
    } else {
        strcpy(result, W2A(varText));
	}

	if ( bLogAsUserSeen ) {
		log(processLine(result));
		log("\n");
		add_line_to_scrollbuffer(result);
	}
	tintin_puts2(result);

	return S_OK;
}

STDMETHODIMP CJmcObj::Send(BSTR bstrLine)
{
	USES_CONVERSION;

    if ( bstrLine ) {
        write_line_mud(W2A(bstrLine));
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
	USES_CONVERSION;
	parse_input(W2A(bstrCommand));

	return S_OK;
}

STDMETHODIMP CJmcObj::Output(BSTR bstrText, BSTR bstrColor)
{
    USES_CONVERSION;

    char text[BUFFER_SIZE], result[BUFFER_SIZE];

    prepare_actionalias(W2A(bstrText) ,text, sizeof(text)); 
    if ( bstrColor ) {
        add_codes(text, result, W2A(bstrColor));
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
    USES_CONVERSION;
    char event[256];
    strcpy(event, W2A(bstrEvent ));
    _tcsupr(event);

    char* p = W2A(bstrCode);


    if ( *event == 'C' && !strcmp(event, "CONNECTED" ) ){
        m_bstrEventsHandlers[ID_Connected] = bstrCode;
    } else 
    if ( *event == 'C' && !strcmp(event, "CONNECTLOST" ) ){
        m_bstrEventsHandlers[ID_ConnectLost] = bstrCode;
    } else 
    if ( *event == 'I' && !strcmp(event, "INCOMING" ) ){
        m_bstrEventsHandlers[ID_Incoming] = bstrCode;
    } else 
    if ( *event == 'I' && !strcmp(event, "INPUT" ) ) {
        m_bstrEventsHandlers[ID_Input] = bstrCode;
    } else 
    if ( *event == 'T' && !strcmp(event, "TIMER" ) ){
        m_bstrEventsHandlers[ID_Timer] = bstrCode;
    } else 
    if ( *event == 'P' && !strcmp(event, "PRETIMER" ) ){
        m_bstrEventsHandlers[ID_PreTimer] = bstrCode;
    } else 
    if ( *event == 'D' && !strcmp(event, "DISCONNECTED" ) ){
        m_bstrEventsHandlers[ID_Disconnected] = bstrCode;
    } else 
    if ( *event == 'L' && !strcmp(event, "LOAD" ) ) {
        m_bstrEventsHandlers[ID_Load] = bstrCode;
    } else 
    if ( *event == 'U' && !strcmp(event, "UNLOAD" ) ){
        m_bstrEventsHandlers[ID_Unload] = bstrCode;
    } else
	if ( *event == 'P' && !strcmp(event, "PROMPT" ) ){
        m_bstrEventsHandlers[ID_Prompt] = bstrCode;
    } else 
	if ( *event == 'T' && !strcmp(event, "TELNETSE" ) ){
        m_bstrEventsHandlers[ID_TelnetSE] = bstrCode;
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
	USES_CONVERSION;
	if ( MUDSocket ) 
        return S_OK;

    connect_mud(W2A(bstrAddress), W2A(bstrPort));

	return S_OK;
}

STDMETHODIMP CJmcObj::Disconnect()
{
    if ( MUDSocket ) {
        zap_command("\0");
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
    USES_CONVERSION;
    CComBSTR arg = bstrKey;
    arg += " ";
    arg += bstrCommand;
    SetHotKey(W2A(arg));

	return S_OK;
}

STDMETHODIMP CJmcObj::get_CommandChar(BSTR *pVal)
{
    char buf[2] = {cCommandChar, 0};
    CComBSTR bstr(buf);

    *pVal = bstr.Copy ();

	return S_OK;
}


STDMETHODIMP CJmcObj::SetStatus(LONG StatusNum, BSTR bstrText, BSTR bstrColor)
{
    USES_CONVERSION;

    char text[BUFFER_SIZE];

    if ( bstrColor ) {
        sprintf(text, "%d {%s} {%s}", StatusNum , W2A(bstrText), W2A(bstrColor));
        status_command(text);
    } else {
        sprintf(text, "%d {%s}", StatusNum , W2A(bstrText));
        status_command(text);
    }
	return S_OK;
}


STDMETHODIMP CJmcObj::SetVar(BSTR bstrVarName, BSTR bstrValue, BOOL bGlobal)
{
    USES_CONVERSION;
	if ( !bstrVarName ) 
        return E_INVALIDARG;

    char varname[BUFFER_SIZE];
	WideCharToMultiByte(CP_ACP, 0, bstrVarName, -1, varname, BUFFER_SIZE, NULL, NULL);

    VAR_INDEX ind = VarList.find (varname);
    VAR* pvar;
    if ( ind != VarList.end() ) {
        pvar = ind->second;
        pvar->m_strVal = W2A(bstrValue);
    }
    else {
        pvar = new VAR(W2A(bstrValue));
        VarList[varname] = pvar;
    }
    pvar->m_bGlobal = bGlobal;

	return S_OK;
}

STDMETHODIMP CJmcObj::GetVar(BSTR bstrVarName, BSTR *bstrRet)
{
	if ( !bstrVarName ) 
        return E_INVALIDARG;

    char varname[BUFFER_SIZE];
	WideCharToMultiByte(CP_ACP, 0, bstrVarName, -1, varname, BUFFER_SIZE, NULL, NULL);

    VAR_INDEX ind = VarList.find (varname);
    CComBSTR ret("");
    if ( ind != VarList.end() ) {
        VAR* pvar = ind->second;
        ret = pvar->m_strVal.data();
    } else {
		for(int i = 0; i < JMC_SPECIAL_VARIABLES_NUM; i++)
			if (!strcmp(varname, jmc_vars[i].name)) {
				char specialVariableValue[BUFFER_SIZE];
				(*(jmc_vars[i].jmcfn))(specialVariableValue);
				ret = specialVariableValue;
				break;
			}
	}
    *bstrRet = ret.Copy ();

    
	return S_OK;
}

//vls-begin// #quit
STDMETHODIMP CJmcObj::Quit()
{
    quit_command("");

    return S_OK;
}
//vls-end//

//vls-begin// #run
STDMETHODIMP CJmcObj::Run(BSTR bstrCmdLine, BSTR bstrParams)
{
    USES_CONVERSION;

    char cmd[BUFFER_SIZE];
    char params[BUFFER_SIZE];

    if (bstrParams && *W2A(bstrParams)) {
        strcpy(cmd, W2A(bstrCmdLine));
        strcpy(params, W2A(bstrParams));
    } else {
        int i, j;
        char arg[BUFFER_SIZE];
        char *p = W2A(bstrCmdLine);
        for (i = j = 0; p[i] && j<BUFFER_SIZE-3; i++) {
            if (p[i] == '\\')
                arg[j++] = '\\';
            arg[j++] = p[i];
        }
        arg[j] = '\0';
        p = get_arg_in_braces(arg, cmd, STOP_SPACES);
        get_arg_in_braces(p, params, WITH_SPACES);
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
    char wave[MAX_PATH+2];
	WideCharToMultiByte(CP_ACP, 0, bstrFileName, -1, wave, MAX_PATH, NULL, NULL);

    char fn[MAX_PATH+2];
    MakeAbsolutePath(fn, wave, szBASE_DIR);
    PlaySound(fn, NULL, SND_ASYNC | SND_FILENAME);

    return S_OK;
}
//vls-end//

//* en
STDMETHODIMP CJmcObj::wOutput(LONG wndNum, BSTR bstrText, BSTR bstrColor)
{
    USES_CONVERSION;

    char text[BUFFER_SIZE], result[BUFFER_SIZE];
	
	if(wndNum>MAX_OUTPUT || wndNum<0)
   	  return S_OK;

    prepare_actionalias(W2A(bstrText) ,text, sizeof(text)); 
    if ( bstrColor ) {
        add_codes(text, result, W2A(bstrColor));
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

STDMETHODIMP CJmcObj::TelnetSB(LONG Option, BSTR bstrData)
{
    USES_CONVERSION;

	if ( !bstrData ) 
        return E_INVALIDARG;

	send_telnet_subnegotiation((unsigned char)Option, W2A(bstrData), SysStringLen(bstrData));

	return S_OK;
}

STDMETHODIMP CJmcObj::ToText(BSTR bstrANSI, BSTR *bstrText)
{
	USES_CONVERSION;

	if ( !bstrANSI ) 
        return E_INVALIDARG;

	CComBSTR ret("");
	char *ansi = W2A(bstrANSI);
	int len = strlen(ansi);
	char *text = new char[len + 1];
	remove_ansi_codes(ansi, text);
	ret = (text);

	*bstrText = ret.Copy();

	delete[] text;

	return S_OK;
}

STDMETHODIMP  CJmcObj::ToColored(BSTR bstrANSI, BSTR *bstrColored)
{
	USES_CONVERSION;

	if ( !bstrANSI ) 
        return E_INVALIDARG;

	CComBSTR ret("");
	char *ansi = W2A(bstrANSI);
	int len = strlen(ansi);
	char *colored = new char[len + 1];
	int state = 37;
	convert_ansi_to_colored(ansi, colored, len, state);
	ret = (colored);

	*bstrColored = ret.Copy();

	delete[] colored;

	return S_OK;
}

STDMETHODIMP CJmcObj::FromColored(BSTR bstrColored, BSTR *bstrANSI)
{
	USES_CONVERSION;

	if ( !bstrColored ) 
        return E_INVALIDARG;

	CComBSTR ret("");
	char *colored = W2A(bstrColored);
	
	int len = strlen(colored);
	int count = 0, i;
	for (i = 0; i < len; i++)
		if (colored[i] == '&')
			count++;
	//&? => ESC[?;??m
	len += count * 5;
	char *ansi = new char[len + 1];
	convert_colored_to_ansi(colored, ansi, len);
	ret = (ansi);

	*bstrANSI = ret.Copy();

	delete[] ansi;

	return S_OK;
}
