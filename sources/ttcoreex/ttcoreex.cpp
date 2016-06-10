// ttcoreex.cpp : Implementation of DLL Exports.


// Note: Proxy/Stub Information
//      To build a separate proxy/stub DLL, 
//      run nmake -f ttcoreexps.mk in the project directory.

#include "stdafx.h"
#include "resource.h"
//#include <initguid.h>
#include "ttcoreex.h"

#include <fstream>

#include "ttcoreex_i.c"

#include <winsock.h>
#include "tintin.h"
#include "JmcSite.h"
#include "JmcObj.h"
#include "telnet.h"

UINT DLLEXPORT MudCodePage;
UINT DLLEXPORT MudCodePageUsed;
std::map< UINT, std::wstring > CPNames;
std::map< std::wstring, UINT > CPIDs;

// #define _DEBUG_LOG
DWORD dwTime0;
int ticker_interrupted;
int tick_size= 60;
int echo=DEFAULT_ECHO;
int speedwalk=DEFAULT_SPEEDWALK;
int togglesubs=DEFAULT_TOGGLESUBS;
int presub=DEFAULT_PRESUB;
int verbose=TRUE;
int acnum=0;
int subnum=0;
int hinum=0;
int antisubnum=0;
int verbatim=0;
char E=27;

HANDLE hConnThread;
HANDLE hPingThread;
DWORD dwPingThreadID;

// ------ Importnant variables for global session ------
struct listnode *common_subs = NULL;
struct listnode *common_antisubs = NULL, *common_pathdirs = NULL, *common_path = NULL;

SOCKET MUDSocket;
wchar_t MUDHostName[256];
sockaddr_in MUDAddress;

LONG DLLEXPORT lPingMUD;
LONG DLLEXPORT lPingProxy;

SOCKET BCASTSocket;
BOOL DLLEXPORT bBCastEnabled = FALSE;
BOOL DLLEXPORT bBCastFilterIP = TRUE;
BOOL DLLEXPORT bBCastFilterPort = TRUE;
WORD DLLEXPORT wBCastUdpPort = 8136;

wchar_t LoopBackBuffer[BUFFER_SIZE];
int LoopBackCount = 0;

wchar_t vars[10][BUFFER_SIZE]; /* the %0, %1, %2,....%9 variables */

wchar_t DLLEXPORT cCommandChar=L'#';
wchar_t DLLEXPORT cCommandDelimiter=L';';
BOOL DLLEXPORT bDefaultLogMode = FALSE; // Overwrite 
BOOL DLLEXPORT bRMASupport;
BOOL DLLEXPORT bAppendLogTitle;
BOOL DLLEXPORT bANSILog;
int DLLEXPORT iSecToTick;
BOOL DLLEXPORT bTickStatus;
BOOL DLLEXPORT bConnectBeep;
BOOL DLLEXPORT bAutoReconnect;
BOOL DLLEXPORT bHTML;
BOOL DLLEXPORT bHTMLTimestamps;
BOOL DLLEXPORT bLogAsUserSeen;
BOOL DLLEXPORT bAllowDebug = FALSE;
BOOL DLLEXPORT bIACSendSingle, bIACReciveSingle;
int DLLEXPORT nScripterrorOutput; // 0 - msgbox, 1- window, 2- output
BOOL DLLEXPORT bDisplayCommands = FALSE;
BOOL DLLEXPORT bDisplayInput = TRUE;
BOOL DLLEXPORT bInputOnNewLine = FALSE;
BOOL DLLEXPORT bDisplayPing = TRUE;
BOOL DLLEXPORT bMinimizeToTray = FALSE;
UINT DLLEXPORT uBroadcastMessage = 0;

wchar_t DLLEXPORT strInfo1[BUFFER_SIZE];
wchar_t DLLEXPORT strInfo2[BUFFER_SIZE];
wchar_t DLLEXPORT strInfo3[BUFFER_SIZE];
wchar_t DLLEXPORT strInfo4[BUFFER_SIZE];
wchar_t DLLEXPORT strInfo5[BUFFER_SIZE];
wchar_t DLLEXPORT editStr[BUFFER_SIZE];
CRITICAL_SECTION DLLEXPORT secStatusSection;

HANDLE DLLEXPORT eventAllObjectEvent;

HANDLE DLLEXPORT eventMudEmuTextArrives;
int DLLEXPORT nMudEmuTextSize = 0;
char DLLEXPORT strMudEmuText[EMULATOR_BUFFER_SIZE];

//vls-begin// #system
CRITICAL_SECTION DLLEXPORT secSystemExec;
CRITICAL_SECTION DLLEXPORT secSystemList;
//vls-end//

//vls-begin// script files
CRITICAL_SECTION DLLEXPORT secScriptFiles;
CRITICAL_SECTION DLLEXPORT secReadingConfig;
HANDLE DLLEXPORT eventReadingConfig;
HANDLE DLLEXPORT eventReadingHasUse;
HANDLE DLLEXPORT eventReadingFirst;
//vls-end//

BOOL bDelayedActionDelete = FALSE;

// Is we are in pathing more
BOOL bPathing = FALSE;
wchar_t verbatim_char=DEFAULT_VERBATIM_CHAR;    
int path_length;
int old_more_coming,more_coming;
wchar_t last_line[BUFFER_SIZE];
ofstream hLogFile;
UINT LogFileCodePage;
ofstream hOutputLogFile[MAX_OUTPUT];
//* en: enhanced logs
wchar_t sLogName[BUFFER_SIZE];
wchar_t sOutputLogName[MAX_OUTPUT][BUFFER_SIZE];
//*/en
int ignore;
BOOL bInterrupted = TRUE;
// -----------------------

wchar_t DLLEXPORT strProductName[256];
wchar_t DLLEXPORT strProductVersion[256];

int mesvar[MSG_MAXNUM];

BOOL bMultiAction, bMultiHighlight;

/*END_FUNC  FastEndFunction;*/
DIRECT_OUT_FUNC  DirectOutputFunction;
CLEAR_WINDOW_FUNC  ClearWindowFunction;
HWND hwndMAIN;

#ifdef _DEBUG_LOG
DLLEXPORT HANDLE hExLog;
#endif

HANDLE hConnectedEvent;

//* en
extern wchar_t mQueue[BUFFER_SIZE];
DWORD dwSTime = 0;
//*/en

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_JmcSite, CJmcSite)
OBJECT_ENTRY(CLSID_JmcObj, CJmcObj)
END_OBJECT_MAP()

typedef struct {
    unsigned char Ttl;                         // Time To Live
    unsigned char Tos;                         // Type Of Service
    unsigned char Flags;                       // IP header flags
    unsigned char OptionsSize;                 // Size in bytes of options data
    unsigned char *OptionsData;                // Pointer to options data
} IP_OPTION_INFORMATION, * PIP_OPTION_INFORMATION;
typedef struct {
    DWORD Address;                             // Replying address
    unsigned long  Status;                     // Reply status
    unsigned long  RoundTripTime;              // RTT in milliseconds
    unsigned short DataSize;                   // Echo data size
    unsigned short Reserved;                   // Reserved for system use
    void *Data;                                // Pointer to the echo data
    IP_OPTION_INFORMATION Options;             // Reply options
} IP_ECHO_REPLY, * PIP_ECHO_REPLY;
typedef HANDLE (WINAPI* pfnHV)(VOID);
typedef BOOL (WINAPI* pfnBH)(HANDLE);
typedef DWORD (WINAPI* pfnDHDPWPipPDD)(HANDLE, DWORD, LPVOID, WORD,
									   PIP_OPTION_INFORMATION, LPVOID, DWORD, DWORD); // evil, no?
unsigned long __stdcall PingThread(void *pParam);

HMODULE hMlang = NULL;
HRESULT (__stdcall *fConvertINetMultiByteToUnicode)(LPDWORD, DWORD, LPCSTR, LPINT, LPWSTR, LPINT) = NULL;

static wchar_t multiline_buf[MULTILINE_BUFFER_SIZE];
static int multiline_length = 0;

static void process_incoming(wchar_t* buffer, BOOL FromServer = TRUE);


static LONG GetStringRegKey(HKEY hKey, const std::wstring &strValueName, std::wstring &strValue)
{
    strValue = L"";
    WCHAR szBuffer[512];
    DWORD dwBufferSize = sizeof(szBuffer);
    ULONG nError;
    nError = RegQueryValueEx(hKey, strValueName.c_str(), 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
    if (ERROR_SUCCESS == nError) {
        strValue = szBuffer;
    }
    return nError;
}

/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
    // --CHANGED by Anton Likhtarov for resource support
    rs::hInst = hInstance;
    // --END

	WSADATA			wData;
	WORD			wVersion = MAKEWORD(1,1);
    if (dwReason == DLL_PROCESS_ATTACH)
    {
	    if( WSAStartup(wVersion, &wData) )
	    {
		    return FALSE;
	    }

		hMlang = LoadLibrary(L"mlang.dll");
		fConvertINetMultiByteToUnicode = (HRESULT (__stdcall *)(LPDWORD, DWORD, LPCSTR, LPINT, LPWSTR, LPINT))
			GetProcAddress(hMlang, "ConvertINetMultiByteToUnicode");

		HKEY hKeyDB, hKeyCP;
		wchar_t cp_subkey[256], cp_fullpath[256];

		if (RegOpenKeyEx(HKEY_CLASSES_ROOT, L"MIME\\DataBase\\Codepage", 0, KEY_READ, &hKeyDB) == ERROR_SUCCESS) {
			DWORD n_cps;
			RegQueryInfoKey(hKeyDB,NULL,NULL,NULL,&n_cps,NULL,NULL,NULL,NULL,NULL,NULL,NULL);      

			for (int i = 0; i < n_cps; i++) {
				DWORD dwSize = sizeof(cp_subkey);
				RegEnumKeyEx(hKeyDB,i,cp_subkey,&dwSize,NULL,NULL,NULL,NULL);

				UINT cp = _wtoi(cp_subkey);
				
				swprintf(cp_fullpath, L"MIME\\DataBase\\Codepage\\%ls", cp_subkey);
				if (RegOpenKeyEx(HKEY_CLASSES_ROOT, cp_fullpath, 0, KEY_READ, &hKeyCP) != ERROR_SUCCESS)
					continue;
				
				wstring cp_name;
				GetStringRegKey(hKeyCP, L"WebCharset", cp_name);
				if (cp_name.length() == 0)
					GetStringRegKey(hKeyCP, L"BodyCharset", cp_name);
				if (cp_name.length() > 0 && cp_name[0] != L'_') {
					transform(cp_name.begin(), cp_name.end(), cp_name.begin(), ::towlower);
					CPNames[cp] = cp_name;
					CPIDs[cp_name] = cp;
				}
			}

			RegCloseKey(hKeyDB);
		}

		MudCodePage = GetACP();

        InitializeCriticalSection(&secSubstSection);
        InitializeCriticalSection(&secHotkeys);
        InitializeCriticalSection(&secStatusSection);
//vls-begin// #system
        InitializeCriticalSection(&secSystemExec);
        InitializeCriticalSection(&secSystemList);
//vls-end//
//vls-begin// script files
        InitializeCriticalSection(&secScriptFiles);
        InitializeCriticalSection(&secReadingConfig);
        eventReadingConfig = CreateEvent(NULL, TRUE, FALSE, NULL);
        eventReadingHasUse = CreateEvent(NULL, TRUE, FALSE, NULL);
        eventReadingFirst = CreateEvent(NULL, TRUE, FALSE, NULL);
//vls-end//
        eventAllObjectEvent = CreateEvent(NULL, FALSE, FALSE, NULL );
        SetEvent(eventAllObjectEvent );

        eventMudEmuTextArrives = CreateEvent(NULL, TRUE, FALSE, NULL );

//vls-begin// base dir
        GetModuleFileName(NULL, szBASE_DIR, MAX_PATH);
        wchar_t *p = wcsrchr(szBASE_DIR, L'\\');
        if (p) *p = '\0';

        wcscpy(szSETTINGS_DIR, szBASE_DIR);
        wcscat(szSETTINGS_DIR, L"\\settings");
//vls-end//

        _Module.Init(ObjectMap, hInstance, &LIBID_TTCOREEXLib);
        DisableThreadLibraryCalls(hInstance);

		hPingThread = CreateThread(NULL, 0, &PingThread, NULL, 0, &dwPingThreadID);

		last_line[0] = L'\0';
    }
    else if (dwReason == DLL_PROCESS_DETACH){
//vls-begin// multiple output
        StopLogging();
//vls-end//
//vls-begin// bugfix
        CloseHandle(eventMudEmuTextArrives);
//vls-end//
//vls-begin// script files
        CloseHandle(eventReadingFirst);
        CloseHandle(eventReadingHasUse);
        CloseHandle(eventReadingConfig);
        DeleteCriticalSection(&secReadingConfig);
        DeleteCriticalSection(&secScriptFiles);
//vls-end//
//vls-begin// #system
        systemkill_command(L"all");
        DeleteCriticalSection(&secSystemList);
        DeleteCriticalSection(&secSystemExec);
//vls-end//

        DeleteCriticalSection(&secSubstSection);
        DeleteCriticalSection(&secHotkeys);
        DeleteCriticalSection(&secStatusSection);
        CloseHandle(eventAllObjectEvent);

#ifdef _DEBUG_LOG
        if ( hExLog ) 
            CloseHandle(hExLog);
#endif
        _Module.Term();
    }
    return TRUE;    // ok
}

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void)
{
    // registers object, typelib and all interfaces in typelib
    return _Module.RegisterServer(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
    return _Module.UnregisterServer(TRUE);
}


// JMC functons starts here 

void tintin_puts2(const wchar_t *cptr)
{
	wchar_t buff[BUFFER_SIZE + 32];
	buff[0] = TINTIN_OUTPUT_MARK;
	buff[1] = 0;
    wcscat(buff, cptr);
    wcscat(buff, L"\n");
    DirectOutputFunction(buff, 0); // out to main wnd
}

void tintin_puts3(const wchar_t *cptr, int wnd)
{
	wchar_t buff[BUFFER_SIZE + 32];
	buff[0] = TINTIN_OUTPUT_MARK;
	buff[1] = 0;
    wcscat(buff, cptr);
    wcscat(buff, L"\n");

    if ( hOutputLogFile[wnd].is_open() ) {
		log(wnd, processTEXT(wstring(cptr)));
		log(wnd, L"\n");
	}

    DirectOutputFunction(buff, 1+wnd); // out to output window
}

void output_command(wchar_t* arg)
{ 
    wchar_t left[BUFFER_SIZE], right[BUFFER_SIZE];
    wchar_t result[BUFFER_SIZE], strng[BUFFER_SIZE];
    
    arg=get_arg_in_braces(arg,left,WITH_SPACES,sizeof(left)/sizeof(wchar_t)-1);
    arg=get_arg_in_braces(arg,right,WITH_SPACES,sizeof(right)/sizeof(wchar_t)-1);

    if ( !right[0] ) {  // no colors
        prepare_actionalias(left,strng, sizeof(strng)/sizeof(wchar_t)); 
    } else {
		prepare_actionalias(right,result, sizeof(result)/sizeof(wchar_t)); 
        add_codes(result, strng, left);
    }

    tintin_puts3(strng, 0);
}


void write_line_mud(const wchar_t *line)
{
    int len, OriginalLen;
    char buff[BUFFER_SIZE*8], coded[BUFFER_SIZE*4];
    int ret = 0;

    if ( !MUDSocket ) {
        tintin_puts(rs::rs(1182) );
    } else {
        OriginalLen = len = wcslen(line);

		int count;
		if (MudCodePageUsed == 1200) {
			count = len * 2;
			memcpy(coded, line, count);
		} else if (MudCodePageUsed == 1201) {
			count = len * 2;
			utf16le_to_utf16be((wchar_t*)coded, line, len);
		} else {
			count = WideCharToMultiByte(MudCodePageUsed, 0, &line[ret], len, coded, sizeof(coded) - 1, NULL, NULL);
		}
        
		if ( bIACSendSingle ) 
			memcpy (buff, coded, count);
		else {
			char* ptr = buff;
			int nIACs = 0;
			for ( int i = 0; i < count; i ++ ) {
				if ( coded[i] == (char)0xff ) {
					*ptr++ = (char)0xff;
					nIACs ++;
				}
				*ptr++ = coded[i];
			}
			count += nIACs;

		}
		if ( buff[count-1] != 0xA ) {
			buff[count] = 0xA;
			buff[count+1] = 0;
			count ++;
		}

		int sent = tls_send(MUDSocket, buff, count);

		if (sent < 0)
			ret = sent;
		else
			ret = len;
    }

//* en
	wchar_t daaString[BUFFER_SIZE+2];
	int daalen = 0;
	if (bDaaMessage) {
		daaString[daalen++] = L'<';
		for(int i = 0; i < wcslen(line) && i < BUFFER_SIZE; i++)
			daaString[daalen++]=L'*';
		daaString[daalen++] = L'>';
	}
	daaString[daalen] = 0;

    if (bDisplayInput && !(SocketFlags & SOCKECHO) && wcslen(line) > 0) 
	{
        std::wstring str;
		str = USER_INPUT_MARK;
        str += L"\x1B[0;33m";
		str += bDaaMessage ? daaString : line;
        str += L"\x1B[0m";
        tintin_puts2(str.c_str());

		if (hLogFile) {
			log(processLine(bDaaMessage ? daaString : line));
			log(L"\n");
		}
		add_line_to_scrollbuffer(bDaaMessage ? daaString : line);
    }    

//* en
	bDaaMessage = FALSE;
//* /en


#ifdef _DEBUG_LOG
    // --------   Write external log
    if (hExLog ) {
        char exLogText[128];
        DWORD Written;
        swprintf(exLogText , L"\r\n#SEND got %d bytes sent %d bytes#\r\n" , len, ret );
        WriteFile(hExLog , exLogText , wcslen(exLogText) , &Written, NULL);
        WriteFile(hExLog , buff , len , &Written, NULL);
    }
#endif
    if (  ret < 0 ) {
        tintin_puts(rs::rs(1183) );
    }
}


static wchar_t strConnectAddress[128], strConnectPort[32];


unsigned long __stdcall ConnectThread(void * pParam)
{
	USES_CONVERSION;

    CoInitialize ((void*)COINIT_MULTITHREADED);

    tintin_puts2(rs::rs(1184));

    SOCKET sock;
    int connectresult;
    struct sockaddr_in sockaddr;

    if(iswdigit(*strConnectAddress)) {                            /* interprete host part */
        sockaddr.sin_addr.s_addr=inet_addr(W2A(strConnectAddress));

		struct hostent *hp;
		if((hp=gethostbyaddr((const char*)&sockaddr, sizeof(sockaddr), AF_INET))==NULL) {
            wcscpy(MUDHostName, strConnectAddress);
        } else {
			wcscpy(MUDHostName, A2W(hp->h_name));
        }	
    } else {
		wcscpy(MUDHostName, strConnectAddress);

        struct hostent *hp;
        if((hp=gethostbyname(W2A(strConnectAddress)))==NULL) {
            tintin_puts2(rs::rs(1185));
            return 0;
        } else {
    	    struct in_addr inad;
            wchar_t ipaddr[256];

	        inad.S_un.S_addr = *(long*)hp->h_addr_list[0];
            swprintf(ipaddr , rs::rs(1186) , A2W(inet_ntoa(inad)));
	        tintin_puts2(ipaddr); 
        }
        memcpy((char *)&sockaddr.sin_addr, hp->h_addr, sizeof(sockaddr.sin_addr));
    }

    if(iswdigit(*strConnectPort))
        sockaddr.sin_port=htons((short)_wtoi(strConnectPort));      /* inteprete port part */
    else {
        tintin_puts2(rs::rs(1187));
        return 0;
    }

START1:
    if((sock=socket(AF_INET, SOCK_STREAM, 0))<0)
        tintin_puts2(rs::rs(1188));

	BOOL enable_opt = TRUE;
	/*
	 Something more than TCP_NODELAY should be done in case MUD server frequently sends tiny packets.
	 For example, if server sends prompt-line and IAC-GA in separate calls of send() then the first
	 packet (prompt) wouldn't ACKed by windows core for 200 milliseconds regardless of Nagle's algorithm,
	 leading to IAC-GA delayed for the same time. It's not a problem for visualizing since now JMC
	 prints out all incoming data immidiately; but processing of prompt line in described situation
	 by #acion and jmc.Incoming handlers will be delayed for 200 ms or value of Uncomplete line delay
	 setting in case it is less than 200.
	 Setting registry value on the client's machine (TcpAckFrequency := 1) is ugly, but effective solution.
	 No other solution (i.e. disabling delayed acknowledgement for particular socket) was found for
	 WinSock/WinSock2, WinXP/Vista/Win7/Win8. So it is responsibility of MUD servers to do wise and
	 accurate TCP bufferisation, though obviously it is possible only in MUDs where the smallest time period
	 between events (game-state updating period, "tick", time-quant) is less than 200ms.
	 At this point I giving up to solve this issue completely and mark it as part 
	 of global MUD problem called "Telnet Curse".
	*/
	if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char*)&enable_opt, sizeof(enable_opt))) {
		tintin_puts2(L"#Can't disable Nagle's algorithm");
	}

    sockaddr.sin_family=AF_INET;


	reset_telnet_protocol();
	multiline_length = 0;

	if (ulProxyAddress) {
		wchar_t buf[BUFFER_SIZE];
		swprintf(buf, rs::rs(1303),
			(ulProxyAddress >> 24) & 0xff, (ulProxyAddress >> 16) & 0xff, (ulProxyAddress >> 8) & 0xff, (ulProxyAddress >> 0) & 0xff);
		tintin_puts2(buf);
	} else {
		tintin_puts2(rs::rs(1189));
	}
    connectresult = proxy_connect(sock,(struct sockaddr *)&sockaddr, sizeof(sockaddr));

    if(connectresult || tls_open(sock) < 0) {
        proxy_close(sock);
        switch(connectresult) {
        case WSAECONNREFUSED:
            tintin_puts(rs::rs(1190));
            break;
        case WSAENETUNREACH:
            tintin_puts(rs::rs(1191));
            break;
        default:
            tintin_puts(rs::rs(1192));
        }
        if ( bAutoReconnect ) {
            Sleep(200);
            goto START1;
        }
        else 
            return 0;
    }

    if ( bConnectBeep ) 
        MessageBeep(MB_OK);
    MUDSocket = sock;
	MUDAddress = sockaddr;
    SetEvent(hConnectedEvent );
	tintin_puts(rs::rs(1193));
    return 0;
}

static unsigned short tcpip_checksum(const unsigned char *buffer, int length) {
    unsigned long sum = 0, i;
    for(i = 0; i < length - 1; i += 2)
        sum += (((unsigned short)(buffer[i])) << 8) | (unsigned short)(buffer[i + 1]);
    if(length & 1) {
        sum += ((unsigned short)buffer[length - 1]) << 8;
    }
    while(sum >> 16UL)
        sum = (sum & 0xffffUL) + (sum >> 16UL);
    return ~((unsigned short)sum);
}

static int ping_single_host(unsigned long ipaddr, int timeout_ms)
{
	static HINSTANCE hIcmp = NULL;
	static pfnHV pIcmpCreateFile = NULL;
    static pfnBH pIcmpCloseHandle = NULL;
    static pfnDHDPWPipPDD pIcmpSendEcho = NULL;
	static HANDLE hIP = INVALID_HANDLE_VALUE;
	static PIP_ECHO_REPLY pIpe = NULL;

	static char acPingBuffer[64];

	if (!ipaddr)
		return -2;

	if (!hIcmp) {
		for (int i = 0; i < sizeof(acPingBuffer); i++)
			acPingBuffer[i] = L'a' + (i % 24);
		hIcmp = LoadLibrary(L"ICMP.DLL");
	}
	if (!hIcmp)
		return -3;

	if (!pIcmpCreateFile)
		pIcmpCreateFile = (pfnHV)GetProcAddress(hIcmp, "IcmpCreateFile");
	if (!pIcmpCloseHandle)
		pIcmpCloseHandle = (pfnBH)GetProcAddress(hIcmp, "IcmpCloseHandle");
	if (!pIcmpSendEcho)
		pIcmpSendEcho = (pfnDHDPWPipPDD)GetProcAddress(hIcmp, "IcmpSendEcho");
	if (!pIcmpCreateFile || !pIcmpCloseHandle || !pIcmpSendEcho)
		return -3;
	
	// Open the ping service
	if (hIP == INVALID_HANDLE_VALUE)
		hIP = pIcmpCreateFile();
	if (hIP == INVALID_HANDLE_VALUE)
		return -3;

	// Build ping packet
	if (!pIpe)
		pIpe = (PIP_ECHO_REPLY)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,
			sizeof(IP_ECHO_REPLY) + sizeof(acPingBuffer));
	if (!pIpe) 
		return -3;

	pIpe->Data = acPingBuffer;
	pIpe->DataSize = sizeof(acPingBuffer);      
							
	// Send the ping packet
	DWORD dwStatus = pIcmpSendEcho(hIP, ipaddr, acPingBuffer, sizeof(acPingBuffer), NULL, pIpe, 
								   sizeof(IP_ECHO_REPLY) + sizeof(acPingBuffer), timeout_ms);
	if (!dwStatus)
		return -3;
	
	if (pIpe->Status && (pIpe->Status != 11000 /*IP_SUCCESS*/))
		return -3;
	if ((pIpe->Status == 11010 /*IP_REQ_TIMED_OUT*/) || (pIpe->Status == 11013 /*IP_TTL_EXPIRED_TRANSIT*/))
		return -1;

	return pIpe->RoundTripTime;
}

unsigned long __stdcall PingThread(void * pParam)
{
    CoInitialize ((void*)COINIT_MULTITHREADED);

	int time_step_ms = 500;

	for(;;) {
		int t0 = GetTickCount();

		if (bDisplayPing) {
			lPingMUD = ping_single_host(MUDAddress.sin_addr.s_addr, time_step_ms * 2);
			lPingProxy = ping_single_host(htonl(ulProxyAddress), time_step_ms * 2);
		} else {
			lPingMUD = lPingProxy = -4;
		}
		PostMessage(hwndMAIN, WM_USER+680, (WPARAM)lPingMUD, (LONG)lPingProxy);

		int dt = GetTickCount() - t0;
		if (dt < time_step_ms)
			Sleep(time_step_ms - dt);
	}
    return 0;
}


void connect_mud(wchar_t *host, wchar_t *port)
{
    DWORD dwThreadID;

    if ( wcslen(host) > 120 || wcslen(port) > 60 ) {
        tintin_puts2(rs::rs(1194));
        return;
    }
    wcscpy(strConnectAddress, host);
    wcscpy(strConnectPort, port);

	hConnThread = CreateThread(NULL, 0, &ConnectThread, NULL, 0, &dwThreadID);
}


void ShowError (wchar_t* strError)
{
    MessageBox(hwndMAIN, strError , rs::rs(1195) , MB_OK | MB_ICONSTOP );
}

void add_line_to_multiline(wchar_t *line);
void do_one_line(wchar_t *line);
void do_multiline();

void tintin_puts(const wchar_t *cptr)
{
	// allow substitutions and script-handling
	//check_all_actions(cptr, false);
	do_one_line((wchar_t*)cptr);
	if (wcscmp(cptr, L"."))
		tintin_puts2(cptr);
}


CComObject<CJmcSite>* pSite = NULL;
CComObject<CJmcObj>* pJmcObj = NULL;



void  DLLEXPORT InitState(/*END_FUNC EndFunc, */DIRECT_OUT_FUNC OutFunc, CLEAR_WINDOW_FUNC ClearFunc, HWND mainWnd)
{
	ClearWindowFunction = ClearFunc;
    DirectOutputFunction = OutFunc;
    hwndMAIN = mainWnd;

    dwTime0 = GetTickCount()/1000;

    hConnectedEvent = CreateEvent(NULL, FALSE, FALSE, NULL );

    newactive_session();
    // init group list
    CGROUP* pGroup = new CGROUP (DEFAULT_GROUP_NAME);
    GroupList[DEFAULT_GROUP_NAME] = pGroup;

    HRESULT hr = CComObject<CJmcObj>::CreateInstance(&pJmcObj);
    pJmcObj->AddRef();

    hr = CComObject<CJmcSite>::CreateInstance(&pSite);
    pSite->AddRef();

    // create debug manager 

    hr = CoCreateInstance(CLSID_ProcessDebugManager, 
                      NULL, 
                      CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
                      IID_IProcessDebugManager,
                      (void**)&(pSite->m_pdm));


    if ( SUCCEEDED(hr) ) {
        hr = pSite->m_pdm->CreateApplication(&(pSite->m_pDebugApp));
        hr = pSite->m_pDebugApp->SetName(L"JMC Application");
        hr = pSite->m_pdm->AddApplication(pSite->m_pDebugApp, &pSite->m_dwAppCookie);
    } else {
        pSite->m_pdm = NULL;
    }





#ifdef _DEBUG_LOG
    if ( GetPrivateProfileInt(L"Debug" , L"ExLog" , 0 , L"jmc.ini" ) ) {
        hExLog = CreateFile(L"socket.log" , GENERIC_READ| GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, NULL, NULL );
    }
#endif


}

void  DLLEXPORT CloseState(void)
{
    pJmcObj->Fire_Unload();

    if ( pSite->m_pdm ) 
        pSite->m_pdm.Release ();
    
    if ( pSite ) 
        pSite->Release ();
    if ( pJmcObj ) 
        pJmcObj->Release ();


    std::map<int, TIMER*>::iterator pos = TIMER_LIST.begin ();
    for ( ; pos != TIMER_LIST.end() ; pos = TIMER_LIST.begin ()) {
        TIMER* ptm =  (*pos).second;
        TIMER_LIST.erase (pos);
        delete ptm;
    }

	KillAll(END, L"0");
	if (pScrollLinesBuffer)
		free(pScrollLinesBuffer);
	free_telnet_buffer();
	free_parse_stack();
}

void  DLLEXPORT ReloadScriptEngine(const wchar_t* strScriptText, GUID guidEngine, const wchar_t* strProfile)
{
    pJmcObj->m_bstrProfile = strProfile;
    pSite->InitSite (hwndMAIN, (LPWSTR)strScriptText,  guidEngine);
	tintin_puts(rs::rs(1196));
}

void DLLEXPORT CompileInput(const wchar_t* str)
{
	if (last_line[0]) {
		//process_incoming(L" [GA forced]\x01");
		process_incoming(L"\x01");
	}

    if ( WaitForSingleObject (eventAllObjectEvent, 0) == WAIT_OBJECT_0  ) {
        // Make event 
        pJmcObj->m_pvarEventParams[0] = str;

        if ( !pJmcObj->Fire_Input () ) {
            SetEvent(eventAllObjectEvent );
            return;
        }

        wchar_t line[BUFFER_SIZE];
		int len = SysStringLen(pJmcObj->m_pvarEventParams[0].bstrVal) + 1;
		len = min(len, BUFFER_SIZE) - 1;
        wcsncpy(line, pJmcObj->m_pvarEventParams[0].bstrVal, len);
		line[len] = '\0';
		SetEvent(eventAllObjectEvent );
		parse_input(line);
    } else {
        write_line_mud(str);
	}
}


static void tick_func()
{ 
  static int Done = 1;
  static int lastS = 0;
  static int lastT = 0;

  iSecToTick=tick_size-((GetTickCount()/1000-dwTime0)%tick_size);
  if(lastT != iSecToTick)
  {//every SECOND
  //* en:status refresh
	  for(int i=0;i<6;++i)
  		  PostMessage(hwndMAIN, WM_USER+650+i, 0, 0);
  }
  lastT=iSecToTick;

  int iDSecToTick=tick_size-((GetTickCount()/100-dwTime0)%tick_size);
  if(lastS!=iDSecToTick)
  {//every DECI-SECOND
  //* en:wait state
	  if(iWaitState)
		  iWaitState--;
	  if(!iWaitState && wcslen(mQueue))
	  {
		  parse_input(mQueue);
		  mQueue[0]=L'\0';
	  }
  //* en:cycles
	  dwSTime = iDSecToTick;
	  update_timers(iDSecToTick);
  }
  lastS=iDSecToTick;
  //*/en
  if(iSecToTick==tick_size || iSecToTick==10) {
      if(bTickStatus) {
          if(iSecToTick==tick_size && !Done ){
              if ( MUDSocket ) {
                    tintin_puts(rs::rs(1197));
              }
            Done = 1;
          }
        if(iSecToTick==10 && Done ){
            if ( MUDSocket ) {
                tintin_puts(rs::rs(1198));
            }
          Done = 0;
        }
      }
  } 

}



/**********************************************************/
/* do all of the functions to one line of buffer          */
/**********************************************************/
void add_line_to_multiline(wchar_t *line)
{
	int multiline_capacity = sizeof(multiline_buf)/sizeof(wchar_t) - 1; //hold one char for null-terminator
	int rest = multiline_capacity - multiline_length;

	if (rest <= 0)
		return;

	int len = wcslen(line);

	if (len <= 0)
		return;

	if (multiline_length > 0) { //add new line
		multiline_buf[multiline_length++] = L'\n';
		rest--;

		if (rest <= 0)
			return;
	}

	if (rest < len) {
		// show error?
		len = rest;
	}
    
	memcpy(&multiline_buf[multiline_length], line, len*sizeof(wchar_t));
	multiline_length += len;
}

void do_one_line(wchar_t *line)
{   
    pJmcObj->m_pvarEventParams[0] = (line);
    BOOL bRet = pJmcObj->Fire_Incoming();
    if ( bRet && pJmcObj->m_pvarEventParams[0].vt == VT_BSTR) {
        int len = SysStringLen(pJmcObj->m_pvarEventParams[0].bstrVal) + 1;
		len = min(len, BUFFER_SIZE) - 1;
		wcsncpy(line, pJmcObj->m_pvarEventParams[0].bstrVal, len);
		line[len] = L'\0';
    } else {
        wcscpy(line, L"." );
	}

    if ( bRet ) {
		if (!presub && !ignore) {
			add_line_to_multiline(line);
			check_all_actions(line, false);
		}

        if (!togglesubs) {
			if(!do_one_antisub(line))
				do_one_sub(line);
		}

        if (presub && !ignore) {
			add_line_to_multiline(line);
			check_all_actions(line, false);
		}

        do_one_high(line);
    }
}

void do_multiline()
{
	if (!multiline_length)
		return;
	multiline_buf[multiline_length] = 0;
	
    pJmcObj->m_pvarEventParams[0] = (multiline_buf);
    BOOL bRet = pJmcObj->Fire_Prompt();
	if ( bRet && pJmcObj->m_pvarEventParams[0].vt == VT_BSTR) {
		int len = SysStringLen(pJmcObj->m_pvarEventParams[0].bstrVal) + 1;
		len = min(len, sizeof(multiline_buf)/sizeof(wchar_t)) - 1;
		wcsncpy(multiline_buf, pJmcObj->m_pvarEventParams[0].bstrVal, len);
		multiline_buf[len] = L'\0';
		check_all_actions(multiline_buf, true);
    }

	multiline_length = 0;
}


wchar_t DLLEXPORT substChars[SUBST_ARRAY_SIZE];
BOOL DLLEXPORT bSubstitution;
CRITICAL_SECTION DLLEXPORT secSubstSection;

// for delayed more-coming 
DWORD lastRecvd;
int DLLEXPORT MoreComingDelay = 150;

/*************************************************************/
/* read text from mud and test for actions/substitutes */
/*************************************************************/

static void process_incoming(wchar_t* buffer, BOOL FromServer)
{
    wchar_t linebuffer[BUFFER_SIZE], *cpsource, *cpdest;
	wchar_t line_to_log[BUFFER_SIZE];
    int LastLineLen = 0, n;

    cpsource = buffer; 
    cpdest = linebuffer;
	if (last_line[0]) {
        LastLineLen = wcslen(last_line);
        wcscpy(linebuffer,last_line);
        cpdest += LastLineLen;
    } 

    BOOL bProcess = TRUE;
    if ( WaitForSingleObject (eventAllObjectEvent, 0) != WAIT_OBJECT_0  ) 
        bProcess = FALSE;

    while(*cpsource) { 
        if ( *cpsource == L'\r' ) {
            cpsource++;
            continue;
        }

        if(*cpsource == L'\n' || *cpsource == END_OF_PROMPT_MARK) {
            *cpdest = L'\0';
			
			if ( !bLogAsUserSeen ) {
				wcscpy(line_to_log, linebuffer);
			}
            if ( bProcess ) { 
                do_one_line(linebuffer);
				if (*cpsource == END_OF_PROMPT_MARK)
					do_multiline();
			}
			if ( bLogAsUserSeen ) {
				wcscpy(line_to_log, linebuffer);
			}

            if(!bLogPassedLine && (bLogAsUserSeen || FromServer) && wcscmp(line_to_log, L".")) {
				if(hLogFile.is_open()) {
					log(processLine(line_to_log));
					log(L"\n");
				}

				add_line_to_scrollbuffer(line_to_log);
            }

            bLogPassedLine = FALSE;

            if( wcscmp(linebuffer, L".") ) {
                n = wcslen(linebuffer);
				linebuffer[n++] = *cpsource;
				linebuffer[n] = L'\0';
				DirectOutputFunction(linebuffer, 0);// out to main window
            }
			last_line[0] = L'\0';

            cpsource++;

            cpdest = linebuffer;
        } else {
            *cpdest++ = *cpsource++;
		}
    }
    *cpdest=L'\0';

	if (wcscmp(linebuffer, L".")) {
		wcscpy(last_line , linebuffer);
		DirectOutputFunction(linebuffer, 0);// out to main window
		lastRecvd = GetTickCount();
	} else {
		last_line[0] = L'\0';
	}

    if ( bProcess  ) 
        SetEvent(eventAllObjectEvent );
}

void read_mud(void )
{
    char buffer[BUFFER_SIZE];
	wchar_t processed[BUFFER_SIZE+32];
    int didget, decoded;
	unsigned long len;
	bool error = false;

	if( ioctlsocket(MUDSocket, FIONREAD, &len) == SOCKET_ERROR || len == 0 ) {
		error = true;
	} else {
		decoded = 0;
		while( len > 0 ) {
			int to_read = len;
			if( to_read > sizeof(buffer) )
				to_read = sizeof(buffer);
			if( (didget = tls_recv(MUDSocket, buffer, to_read)) < 0 ) {
				error = true;
				break;
			} else if (didget == 0) {
				break;
			}
			len -= didget;

#ifdef _DEBUG_LOG
			// --------   Write external log
			if (hExLog ) {
				wchar_t exLogText[128];
				DWORD Written;
				swprintf(exLogText , L"\r\n#RECV got %d bytes#\r\n" , didget );
				WriteFile(hExLog , exLogText , wcslen(exLogText) , &Written, NULL);
				WriteFile(hExLog , buffer , didget , &Written, NULL);
			}
#endif

			telnet_push_back(buffer, didget);
			while( (didget = telnet_pop_front(processed, sizeof(processed)/sizeof(wchar_t) - 1)) > 0 ) {
				decoded += didget;
				processed[didget] = L'\0';

#ifdef _DEBUG_LOG
				if (hExLog ) {
					wchar_t exLogText[128];
					DWORD Written;
					swprintf(exLogText , L"\r\n#BUFFER AFTER TELNET#\r\n" );
					WriteFile(hExLog , exLogText , wcslen(exLogText) , &Written, NULL);
					WriteFile(hExLog , processed , didget , &Written, NULL);
				}
#endif
				more_coming = telnet_more_coming();

				if ( bSubstitution ) {
					int i;
				// start crit section
					EnterCriticalSection(&secSubstSection);
					for ( i= 0 ; i < *((int*)substChars) ; i++ ) {
						int j;
						for ( j = 0 ; j < didget ; j++ ) {
							if (processed[j] == substChars[i*2+sizeof(int)])
								processed[j] = substChars[i*2+sizeof(int)+1];
						}
					}
					LeaveCriticalSection(&secSubstSection);
				}

				process_incoming(processed);
			}
			if (didget < 0) {
				error = true;
				break;
			}
		}
	}

	if(error) {
		cleanup_session();
		tintin_puts(rs::rs(1199));
        MUDSocket = NULL;
		memset(&MUDAddress, 0, sizeof(MUDAddress));
        if ( WaitForSingleObject (eventAllObjectEvent, 0) == WAIT_OBJECT_0  ) {
            pJmcObj->Fire_ConnectLost();
            SetEvent(eventAllObjectEvent );
        }
        
        if ( bAutoReconnect) {
            DWORD dwThreadID;
            hConnThread = CreateThread(NULL, 0, &ConnectThread, NULL, 0, &dwThreadID);
        }
        //bReadingMud = FALSE;
	}
}


BOOL DLLEXPORT ParseScript(const wchar_t* strScriptlet)
{
	EXCEPINFO    ei;
    CComBSTR bstr(strScriptlet);

    HRESULT  hr; // = pSite->m_ScriptEngine->GetScriptState (&ss);
    if ( bInterrupted ) {
        hr = pSite->m_ScriptEngine->SetScriptState(SCRIPTSTATE_CONNECTED);
        bInterrupted = FALSE;
    }
//vls-begin// base dir
//    hr = pSite->m_ScriptParser->ParseScriptText((BSTR)bstr, NULL, NULL, NULL, 0, 0, 0, NULL, &ei);
    wchar_t dir[MAX_PATH+2];
    GetCurrentDirectory(MAX_PATH, dir);
    SetCurrentDirectory(szBASE_DIR);
    hr = pSite->m_ScriptParser->ParseScriptText((BSTR)bstr, NULL, NULL, NULL, 0, 0, 0, NULL, &ei);
    SetCurrentDirectory(dir);
//vls-end//

    return TRUE;
}

void ParseScriptlet2(BSTR bstrScriptlet)
{
	EXCEPINFO    ei;

    if ( !bstrScriptlet ) 
        return;

    HRESULT  hr; // = pSite->m_ScriptEngine->GetScriptState (&ss);
    if ( bInterrupted ) {
        hr = pSite->m_ScriptEngine->SetScriptState(SCRIPTSTATE_CONNECTED);
        bInterrupted = FALSE;
    }
    
//vls-begin// base dir
//    hr = pSite->m_ScriptParser->ParseScriptText(bstrScriptlet, NULL, NULL, NULL, 0, 0, 0, NULL, &ei);
    wchar_t dir[MAX_PATH+2];
    GetCurrentDirectory(MAX_PATH, dir);
    SetCurrentDirectory(szBASE_DIR);
    hr = pSite->m_ScriptParser->ParseScriptText(bstrScriptlet, NULL, NULL, NULL, 0, 0, 0, NULL, &ei);
    SetCurrentDirectory(dir);
//vls-end//

}
void DLLEXPORT ReadMud()
{    
    fd_set readfdmask;
    struct timeval timeout={0,0};

    if ( MUDSocket ) {
        if ( WaitForSingleObject (hConnectedEvent , 0 ) == WAIT_OBJECT_0 ) 
            if ( WaitForSingleObject (eventAllObjectEvent, 0) == WAIT_OBJECT_0  ) {
                pJmcObj->Fire_Connected ();
                SetEvent(eventAllObjectEvent);
            }


        FD_ZERO(&readfdmask);
        FD_SET(MUDSocket,&readfdmask);

        /*ticker_interrupted=FALSE;*/
        int nRet = select(0, &readfdmask,  0, 0, &timeout);

        if(nRet > 0 && FD_ISSET(MUDSocket,&readfdmask))
            read_mud();
        else { // do delayed delete 
            // check we have delayed string without \n
			more_coming = 0;
			if (last_line[0] && ((int)(GetTickCount() - lastRecvd) >= MoreComingDelay)) {
				//process_incoming(L"\n");
				process_incoming(L"\x01");
            } else if ( bDelayedActionDelete ) {
				if ( WaitForSingleObject (eventAllObjectEvent, 0) == WAIT_OBJECT_0 ) {
					ACTION_INDEX aind = ActionList.begin();
                    while ( aind != ActionList.end() ) {
						// CActionPtr pac = *aind;
                        ACTION* pac = *aind;
                        if ( pac->m_bDeleted) {
							ACTION_INDEX aind1 = aind;
                            aind++;
                            delete pac;
                            ActionList.erase(aind1);
                        } else {
							aind++;
                        }
					}
                    bDelayedActionDelete = FALSE;
                    SetEvent(eventAllObjectEvent);
                } else {
					return;
				}
			}
        }
    } else {
        if ( WaitForSingleObject (eventMudEmuTextArrives, 0 ) == WAIT_OBJECT_0 ) {
            static wchar_t buf[BUFFER_SIZE+32];
			if (nMudEmuTextSize > 0) {
				static DWORD mode;

				int len, processed;

				len = sizeof(buf)/sizeof(wchar_t) - 1;
				processed = nMudEmuTextSize;

				int retConv;
				if (MudCodePage == 1200) {
					int count = min(len, processed) / sizeof(wchar_t);
					memcpy(buf, strMudEmuText, count * sizeof(wchar_t));
					len = processed = count * sizeof(wchar_t);
					retConv = S_OK;
				} else if (MudCodePage == 1201) {
					int count = min(len, processed) / sizeof(wchar_t);
					utf16le_to_utf16be(buf, (const wchar_t*)strMudEmuText, count);
					len = processed = count * sizeof(wchar_t);
					retConv = S_OK;
				} else {
					retConv = fConvertINetMultiByteToUnicode(&mode, MudCodePage, strMudEmuText, &processed, buf, &len);
				}

				if (retConv != S_OK) {
					//???
				}

				buf[len] = 0;
            
				if ( len > 0 && buf[len-1] != L'\x0A' && buf[len-1]!= L'\x01' )
					more_coming = 1;
				else 
					more_coming = 0;

				if (nMudEmuTextSize > processed)
					memmove(&strMudEmuText[0], &strMudEmuText[processed], nMudEmuTextSize - processed);

				nMudEmuTextSize -= processed;
			} else {
				buf[0] = L'\0';
			}

			ResetEvent(eventMudEmuTextArrives);

            process_incoming (buf, FALSE);
        }
    }

	if ( !more_coming ) {
		if (LoopBackCount > 0) {
			process_incoming (LoopBackBuffer, FALSE);
			LoopBackCount = 0;
		}

		if (bBCastEnabled && BCASTSocket != INVALID_SOCKET) {
			wchar_t buf[BUFFER_SIZE];
			unsigned long len;

			for(;;) {	
				if( ioctlsocket(BCASTSocket, FIONREAD, &len) == SOCKET_ERROR )
					break;
				if( !len )
					break;

				if( len >= sizeof(buf) - 1*sizeof(wchar_t) )
					len = sizeof(buf) - 2*sizeof(wchar_t);

				struct sockaddr_in dest;
				int tmp = sizeof(dest);

				if( (len = recvfrom(BCASTSocket, (char*)buf, len, 0, (struct sockaddr*)&dest, &tmp)) <= 0)
					break;
				if( bBCastFilterPort && htons(dest.sin_port) != wBCastUdpPort )
					continue;
				if( bBCastFilterIP ) {
					char hostname[256];
					struct hostent *host;
					
					if( gethostname(hostname, sizeof(hostname)) )
						continue;
					if( !(host = gethostbyname(hostname)) )
						continue;

					bool found = false;
					for (int i = 0; host->h_addr_list[i] != NULL && !found; i++)
						found = found || (!memcmp(host->h_addr_list[i], &dest.sin_addr, 4));
					
					if (!found)
						continue;
				}
				if( !bBCastFilterPort || htons(dest.sin_port) == wBCastUdpPort ) {
					len /= sizeof(wchar_t);
					buf[len++] = L'\n';
					buf[len] = L'\0';

					process_incoming (buf, FALSE);
				}
			}
		}
	}

    // Do timer events 
    std::map<int, TIMER*>::iterator pos = TIMER_LIST.begin ();
    DWORD dwTick = GetTickCount();
    while ( pos != TIMER_LIST.end() ) {
        TIMER* ptm =  (*pos).second;
        pJmcObj->m_pvarEventParams[0] = ptm->m_nID;
        if ( ptm->m_nPreinterval != 0 && ptm->m_bPreTimerDone == FALSE && (ptm->m_nInterval - (dwTick - ptm->m_dwLastTickCount)/100) <= ptm->m_nPreinterval ) {
            pJmcObj->Fire_PreTimer (ptm->m_nID);
            ptm->m_bPreTimerDone = TRUE;
        }

        if ( ((int)(dwTick - ptm->m_dwLastTickCount))/100 >= (int)ptm->m_nInterval ) {
            ptm->m_dwLastTickCount = dwTick ;
            ptm->m_bPreTimerDone = FALSE;
            pJmcObj->Fire_Timer (ptm->m_nID);
            pos = TIMER_LIST.begin ();
            continue;
        }
        pos++;
    }

    tick_func();
}

void MultiactionCommand(wchar_t* arg)
{
    wchar_t left[BUFFER_SIZE];
    arg=get_arg_in_braces(arg,left,STOP_SPACES,sizeof(left)/sizeof(wchar_t)-1);

    if ( *left ) {
        if ( !wcsicmp(L"on", left) ) {
            bMultiAction = TRUE;
            tintin_puts2(rs::rs(1200));
        } else 
            if ( !wcsicmp(L"off", left) ) {
                bMultiAction = FALSE;
                tintin_puts2(rs::rs(1201));
            } else 
                tintin_puts2(rs::rs(1202));
    } else {
        if ( bMultiAction ) {
            bMultiAction = FALSE;
            tintin_puts2(rs::rs(1201));
        }
        else {
            bMultiAction = TRUE;
            tintin_puts2(rs::rs(1200));
        }
    }
}

void MultiHlightCommand(wchar_t* arg)
{
    wchar_t left[BUFFER_SIZE];
    arg=get_arg_in_braces(arg,left,STOP_SPACES,sizeof(left)/sizeof(wchar_t)-1);

    if ( *left ) {
        if ( !wcsicmp(L"on", left) ) {
            bMultiHighlight  = TRUE;
            tintin_puts2(rs::rs(1205));
        } else 
            if ( !wcsicmp(L"off", left) ) {
                bMultiHighlight = FALSE;
                tintin_puts2(rs::rs(1206));
            } else 
                tintin_puts2(rs::rs(1207));
    } else {
        if ( bMultiHighlight ) {
            bMultiHighlight = FALSE;
            tintin_puts2(rs::rs(1206));
        }
        else {
            bMultiHighlight = TRUE;
            tintin_puts2(rs::rs(1205));
        }
    }
}


BOOL  DLLEXPORT IsConnected()
{
    return (BOOL)MUDSocket;
}

BOOL  DLLEXPORT IsLogging()
{
    return (BOOL)hLogFile.is_open();
}

BOOL  DLLEXPORT IsPathing()
{
    return bPathing;
}

void  DLLEXPORT BreakScript()
{
	EXCEPINFO ei;
    memset ( &ei, 0 , sizeof(ei));
    pSite->m_ScriptEngine->InterruptScriptThread (SCRIPTTHREADID_ALL, &ei, SCRIPTINTERRUPT_DEBUG );
    bInterrupted = TRUE;
}


void  DLLEXPORT LunchDebuger()
{
    if ( pSite->m_pDebugApp ) 
        pSite->m_pDebugApp->StartDebugSession();
}

//vls-begin// multiple output
void woutput_command(wchar_t* arg)
{ 
    wchar_t number[BUFFER_SIZE];
    wchar_t left[BUFFER_SIZE], right[BUFFER_SIZE];
    wchar_t result[BUFFER_SIZE], strng[BUFFER_SIZE];
    int wnd = MAX_OUTPUT;
    
    arg=get_arg_in_braces(arg,number,STOP_SPACES,sizeof(number)/sizeof(wchar_t)-1);
    arg=get_arg_in_braces(arg,left,WITH_SPACES,sizeof(left)/sizeof(wchar_t)-1);
    arg=get_arg_in_braces(arg,right,WITH_SPACES,sizeof(right)/sizeof(wchar_t)-1);

    if (!is_all_digits(number) || !swscanf(number, L"%d", &wnd) || wnd < 0 || wnd >= MAX_OUTPUT) {
        tintin_puts2(rs::rs(1239));
        return;
    }

    if ( !right[0] ) {  // no colors
        prepare_actionalias(left,strng, sizeof(strng)/sizeof(wchar_t)); 
    } else {
        prepare_actionalias(right,result, sizeof(result)/sizeof(wchar_t)); 
        add_codes(result, strng, left);
    }
    tintin_puts3(strng, wnd);
}

void wshow_command(wchar_t *arg)
{
    wchar_t number[BUFFER_SIZE];
    wchar_t option[BUFFER_SIZE];
    int wnd = MAX_OUTPUT;
    int opt;

    arg=get_arg_in_braces(arg,number,STOP_SPACES,sizeof(number)/sizeof(wchar_t)-1);
    arg=get_arg_in_braces(arg,option,STOP_SPACES,sizeof(option)/sizeof(wchar_t)-1);

    if (!is_all_digits(number) || !swscanf(number, L"%d", &wnd) || wnd < 0 || wnd >= MAX_OUTPUT) {
        tintin_puts2(rs::rs(1240));
        return;
    }

    if ( option[0] == 0 || (is_abrev(option, L"toggle")) )
        opt = 0;
    else if ( is_abrev(option, L"hide") )
        opt = 1;
    else if ( is_abrev(option, L"show") )
        opt = 2;
    else {
        tintin_puts2(rs::rs(1240));
        return;
    }

    PostMessage(hwndMAIN, WM_USER+500, (WPARAM)(wnd), (LPARAM)(opt));
}

void wname_command(wchar_t *arg)
{
    wchar_t number[BUFFER_SIZE];
    wchar_t option[BUFFER_SIZE];
    int wnd = MAX_OUTPUT;

    arg=get_arg_in_braces(arg,number,STOP_SPACES,sizeof(number)/sizeof(wchar_t)-1);
    arg=get_arg_in_braces(arg,option,STOP_SPACES,sizeof(option)/sizeof(wchar_t)-1);

    if (!is_all_digits(number) || !swscanf(number, L"%d", &wnd) || wnd < 0 || wnd >= MAX_OUTPUT) {
        tintin_puts2(rs::rs(1244));
        return;
    }

    HGLOBAL hg = GlobalAlloc(GHND, (wcslen(option)+2)*sizeof(wchar_t));
    wchar_t* p = (wchar_t*)GlobalLock(hg);
    wcscpy(p, option);
    GlobalUnlock (hg);

    PostMessage(hwndMAIN, WM_USER+501, (WPARAM)wnd, (LPARAM)hg);
}

void wdock_command(wchar_t *arg)
{
    wchar_t number[BUFFER_SIZE];
    wchar_t option[BUFFER_SIZE];
    LONG enable;
    int wnd = MAX_OUTPUT;

    arg=get_arg_in_braces(arg,number,STOP_SPACES,sizeof(number)/sizeof(wchar_t)-1);
    arg=get_arg_in_braces(arg,option,STOP_SPACES,sizeof(option)/sizeof(wchar_t)-1);

	enable = 1;
	if ( is_abrev(option, L"disable") )
		enable = 0;
	else if ( is_abrev(option, L"left") )
		enable = 2;
	else if ( is_abrev(option, L"top") )
		enable = 3;
	else if ( is_abrev(option, L"right") )
		enable = 4;
	else if ( is_abrev(option, L"bottom") )
		enable = 5;

    if (!is_all_digits(number) || !swscanf(number, L"%d", &wnd) || wnd < 0 || wnd >= MAX_OUTPUT) {
        tintin_puts2(rs::rs(1260));
        return;
    }

    PostMessage(hwndMAIN, WM_USER+502, (WPARAM)wnd, enable);
}

DWORD DLLEXPORT wposes[MAX_OUTPUT][2];
void wpos_command(wchar_t *arg)
{
    wchar_t number[BUFFER_SIZE];
    wchar_t point1[BUFFER_SIZE];
    wchar_t point2[BUFFER_SIZE];
    LONG points;
    LONG wnd = MAX_OUTPUT;
    int p1,p2;

    arg=get_arg_in_braces(arg,number,STOP_SPACES,sizeof(number)/sizeof(wchar_t)-1);
    arg=get_arg_in_braces(arg,point1,STOP_SPACES,sizeof(point1)/sizeof(wchar_t)-1);
    arg=get_arg_in_braces(arg,point2,STOP_SPACES,sizeof(point2)/sizeof(wchar_t)-1);

	p1 = (_wtoi(point1) > 2048) ? 2048 : _wtoi(point1);
	p2 = (_wtoi(point2) > 2048) ? 2048 : _wtoi(point2);

    if (!is_all_digits(number) || !swscanf(number, L"%d", &wnd) || wnd < 0 || wnd >= MAX_OUTPUT) {
        tintin_puts2(rs::rs(1244));
        return;
    }

	if(!(*point1))
	{
		swprintf(number,rs::rs(1256),wnd,wposes[wnd][0],wposes[wnd][1]);
		tintin_puts2(number);
		return;
	}
	if(is_abrev(point1,L"reposition"))
	{
		p1 = wposes[wnd][0];
		p2 = wposes[wnd][1];
	}

	wposes[wnd][0] = p1;
	wposes[wnd][1] = p2;
	points = MAKELPARAM(p1,p2);
    PostMessage(hwndMAIN, WM_USER+505, wnd, points);
}

extern GET_WNDSIZE_FUNC GetWindowSize;
extern SET_WNDSIZE_FUNC SetWindowSize;
void wsize_command(wchar_t *arg)
{
    wchar_t number[BUFFER_SIZE];
    wchar_t width[BUFFER_SIZE];
    wchar_t height[BUFFER_SIZE];
    LONG wnd = MAX_OUTPUT;
    int w, h;

    arg = get_arg_in_braces(arg,number,STOP_SPACES,sizeof(number)/sizeof(wchar_t)-1);
    arg = get_arg_in_braces(arg,width,STOP_SPACES,sizeof(width)/sizeof(wchar_t)-1);
    arg = get_arg_in_braces(arg,height,STOP_SPACES,sizeof(height)/sizeof(wchar_t)-1);

    if (!is_all_digits(number) || !swscanf(number, L"%d", &wnd) ||
		!is_all_digits(width)  || !swscanf(width,  L"%d", &w  ) ||
		!is_all_digits(height) || !swscanf(height, L"%d", &h  ) ) {
        tintin_puts2(rs::rs(1281));
        return;
    }
	if(wnd < 0 || wnd >= MAX_OUTPUT)
		wnd = MAX_OUTPUT; //main window

	PostMessage(hwndMAIN, WM_USER+506, wnd, MAKELPARAM(w,h));
}

//vls-end//

void codepage_command(wchar_t *arg)
{
	wchar_t temp[BUFFER_SIZE], codepage[BUFFER_SIZE];

	arg = get_arg_in_braces(arg,temp,WITH_SPACES,sizeof(codepage)/sizeof(wchar_t)-1);

	if (wcslen(temp) == 0) {
		if (CPNames.find(MudCodePage) == CPNames.end()) // should not ever happen
			swprintf(temp, rs::rs(1293), MudCodePage, L"unknown");
		else
			swprintf(temp, rs::rs(1293), MudCodePage, CPNames[MudCodePage].c_str());
		tintin_puts2(temp);
	} else if (is_abrev(temp, L"list")) {
		wstring list;
		list = rs::rs(1294);
		for (map< wstring, UINT >::iterator it = CPIDs.begin(); it != CPIDs.end(); it++) {
			if (it != CPIDs.begin())
				list += L", ";
			list += it->first;
		}
		tintin_puts2(list.c_str());
	} else {
		prepare_actionalias(temp,codepage, sizeof(codepage)/sizeof(wchar_t));

		if (is_abrev(codepage, L"default"))
			swprintf(codepage, L"%d", GetACP());

		if (is_all_digits(codepage)) {
			UINT cp = _wtoi(codepage);
			if (CPNames.find(cp) == CPNames.end()) {
				swprintf(temp, rs::rs(1295), cp);
				tintin_puts2(temp);
			} else {
				MudCodePage = cp;
				swprintf(temp, rs::rs(1296), cp, CPNames[cp].c_str());
				tintin_puts2(temp);
			}
		} else {
			wstring cpname = codepage;
			if (CPIDs.find(cpname) == CPIDs.end()) {
				swprintf(temp, rs::rs(1297), cpname.c_str());
				tintin_puts2(temp);
			} else {
				MudCodePage = CPIDs[cpname];
				swprintf(temp, rs::rs(1296), CPIDs[cpname], cpname.c_str());
				tintin_puts2(temp);
			}
		}
	}

}

int DLLEXPORT enumerate_codepage(int Index, wchar_t *Name, int MaxNameSize)
{
	if (Index < 0 || Index >= CPIDs.size())
		return -1;
	for (map < wstring, UINT >::iterator it = CPIDs.begin(); it != CPIDs.end(); it++)
		if (Index-- == 0) {
			if (Name) {
				int len = min(MaxNameSize - 1, it->first.length());
				wcsncpy(Name, it->first.c_str(), len);
				Name[len] = L'\0';
			}
			return it->second;
		}
	return -1;
}

void DLLEXPORT reopen_bcast_socket()
{
	if( BCASTSocket != INVALID_SOCKET ) {
		closesocket(BCASTSocket);
		BCASTSocket = INVALID_SOCKET;
	}
	if( !bBCastEnabled )
		return;
	if( (BCASTSocket = socket(AF_INET, SOCK_DGRAM, 0)) != INVALID_SOCKET ) {
		BOOL bVal = TRUE;
		if( setsockopt(BCASTSocket, SOL_SOCKET, SO_BROADCAST, (const char*)&bVal, sizeof(bVal)) == SOCKET_ERROR ||
			setsockopt(BCASTSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&bVal, sizeof(bVal)) == SOCKET_ERROR ) {
			closesocket(BCASTSocket);
			BCASTSocket = INVALID_SOCKET;
		} else {
			struct sockaddr_in local;
			local.sin_family = AF_INET;
			local.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
			local.sin_port = htons(wBCastUdpPort);
			if( bind(BCASTSocket, (const sockaddr*)&local, sizeof(local)) == SOCKET_ERROR ) {
				closesocket(BCASTSocket);
				BCASTSocket = INVALID_SOCKET;
			}
		}
	}
}

