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

#include "winsock.h"
#include "tintin.h"
#include "JmcSite.h"
#include "JmcObj.h"
#include "telnet.h"

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
struct listnode *common_subs;
struct listnode *common_antisubs, *common_pathdirs, *common_path;

SOCKET MUDSocket;
sockaddr_in MUDAddress;

LONG DLLEXPORT lPingMUD;
LONG DLLEXPORT lPingProxy;

SOCKET BCASTSocket;
BOOL DLLEXPORT bBCastFilterIP = TRUE;
BOOL DLLEXPORT bBCastFilterPort = TRUE;
WORD DLLEXPORT wBCastUdpPort = 1024;

char LoopBackBuffer[BUFFER_SIZE];
int LoopBackCount = 0;

char vars[10][BUFFER_SIZE]; /* the %0, %1, %2,....%9 variables */

char DLLEXPORT cCommandChar='#';
char DLLEXPORT cCommandDelimiter=';';
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
BOOL DLLEXPORT bMinimizeToTray = FALSE;
UINT DLLEXPORT uBroadcastMessage = 0;

char DLLEXPORT strInfo1[BUFFER_SIZE];
char DLLEXPORT strInfo2[BUFFER_SIZE];
char DLLEXPORT strInfo3[BUFFER_SIZE];
char DLLEXPORT strInfo4[BUFFER_SIZE];
char DLLEXPORT strInfo5[BUFFER_SIZE];
char DLLEXPORT editStr[BUFFER_SIZE];
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
char verbatim_char=DEFAULT_VERBATIM_CHAR;    
int path_length;
int old_more_coming,more_coming;
char last_line[BUFFER_SIZE];
ofstream hLogFile;
ofstream hOutputLogFile[MAX_OUTPUT];
//* en: enhanced logs
char sLogName[BUFFER_SIZE];
char sOutputLogName[MAX_OUTPUT][BUFFER_SIZE];
//*/en
int ignore;
BOOL bInterrupted = TRUE;
// -----------------------

//vls-begin// script files
//int mesvar[8]; // Look like its variables for command
int mesvar[10];
//vls-end//

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
extern char mQueue[BUFFER_SIZE];
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

static char multiline_buf[MULTILINE_BUFFER_SIZE];
static int multiline_length = 0;

static void process_incoming(char* buffer, BOOL FromServer = TRUE);

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
        char *p = strrchr(szBASE_DIR, '\\');
        if (p) *p = '\0';

        strcpy(szSETTINGS_DIR, szBASE_DIR);
        strcat(szSETTINGS_DIR, "\\settings");
//vls-end//

        _Module.Init(ObjectMap, hInstance, &LIBID_TTCOREEXLib);
        DisableThreadLibraryCalls(hInstance);

		hPingThread = CreateThread(NULL, 0, &PingThread, NULL, 0, &dwPingThreadID);

		last_line[0] = '\0';
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
        systemkill_command("all");
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

void tintin_puts2(char *cptr)
{
	char buff[BUFFER_SIZE + 3];
	buff[0] = 0x2;
	buff[1] = 0;
    strcat(buff, cptr);
    strcat(buff, "\n");
    DirectOutputFunction(buff, 0); // out to main wnd
}

void tintin_puts3(char *cptr, int wnd)
{
    char buff[BUFFER_SIZE + 2];

    strcpy ( buff , cptr);
    strcat(buff, "\n");

    if ( hOutputLogFile[wnd] ) {
		log(wnd, processTEXT(string(cptr)));
		log(wnd, "\n");
	}

    DirectOutputFunction(buff, 1+wnd); // out to output window
}

void output_command(char* arg)
{ 
    char left[BUFFER_SIZE], right[BUFFER_SIZE];
    char result[BUFFER_SIZE], strng[BUFFER_SIZE];
    
    arg=get_arg_in_braces(arg, left, WITH_SPACES);
    arg=get_arg_in_braces(arg, right, WITH_SPACES);

    if ( !right[0] ) {  // no colors
        prepare_actionalias(left,strng, sizeof(strng)); 
    } else {
        prepare_actionalias(right,result, sizeof(result)); 
        add_codes(result, strng, left);
    }

    tintin_puts3(strng, 0);
}


void write_line_mud(char *line)
{
    int len, OriginalLen;
//vls-begin// bugfix
    char* buff = 0;
//vls-end//
    int ret = 0;

    if ( !MUDSocket ) {
        tintin_puts(rs::rs(1182) );
    } else {
        OriginalLen = len = strlen(line);
        buff = (char*)malloc(2*len+3);
        if ( bIACSendSingle ) 
            strcpy (buff, line);
        else {
		    char* ptr = buff;
            int nIACs = 0;
		    for ( int i = 0; i < len; i ++ ) {
			    if ( line[i] == (char)0xff ) {
				    *ptr++ = (char)0xff;
                    nIACs ++;
			    }
			    *ptr++ = line[i];
		    }
            len += nIACs;

        }
        if ( buff[len-1] != 0xA ) {
            buff[len] = 0xA;
            buff[len+1] = 0;
            len ++;
        }


        ret = send(MUDSocket,buff, len, 0 );
    }

//* en
//    if ( bDisplayInput ) {
	char daaString[BUFFER_SIZE+2];
	int daalen = 0;
	if (bDaaMessage) {
		daaString[daalen++] = '<';
		for(int i = 0; i < strlen(line); i++)
			daaString[daalen++]='*';
		daaString[daalen++] = '>';
	}
	daaString[daalen] = 0;

    if(bDisplayInput && strlen(line)>0) 
	{
        std::string str;
        str = "\x01\x1B[0;33m";
		str += bDaaMessage ? daaString : line;
        str += "\x1B[0m";
        tintin_puts2((char*)str.c_str());
    }

    if (hLogFile) {
		log(processLine(bDaaMessage ? daaString : line));
		log("\n");
    }

//* en
	bDaaMessage = FALSE;
//* /en


#ifdef _DEBUG_LOG
    // --------   Write external log
    if (hExLog ) {
        char exLogText[128];
        DWORD Written;
        sprintf(exLogText , "\r\n#SEND got %d bytes sent %d bytes#\r\n" , len, ret );
        WriteFile(hExLog , exLogText , strlen(exLogText) , &Written, NULL);
        WriteFile(hExLog , buff , len , &Written, NULL);
    }
#endif
    if (  ret < 0 ) {
        tintin_puts(rs::rs(1183) );
    }
//vls-begin// bugfix
    if (buff)
		free(buff);
//vls-end//
}


static char strConnectAddress[128], strConnectPort[32];


unsigned long __stdcall ConnectThread(void * pParam)
{
    CoInitialize ((void*)COINIT_MULTITHREADED);

    tintin_puts2(rs::rs(1184));

    SOCKET sock;
    int connectresult;
    struct sockaddr_in sockaddr;

    if(isdigit(*strConnectAddress))                            /* interprete host part */
        sockaddr.sin_addr.s_addr=inet_addr(strConnectAddress);
    else {
        struct hostent *hp;
        if((hp=gethostbyname(strConnectAddress))==NULL) {
            tintin_puts2(rs::rs(1185));
            return 0;
        } else {
    	    struct in_addr inad;
            char ipaddr[256];

	        inad.S_un.S_addr = *(long*)hp->h_addr_list[0];
            sprintf(ipaddr , rs::rs(1186) , (char*)inet_ntoa(inad));
	        tintin_puts2(ipaddr); 
        }
        memcpy((char *)&sockaddr.sin_addr, hp->h_addr, sizeof(sockaddr.sin_addr));
    }

    if(isdigit(*strConnectPort))
        sockaddr.sin_port=htons((short)atoi(strConnectPort));      /* inteprete port part */
    else {
        tintin_puts2(rs::rs(1187));
        return 0;
    }

START1:
    if((sock=socket(AF_INET, SOCK_STREAM, 0))<0)
        tintin_puts2(rs::rs(1188));

    sockaddr.sin_family=AF_INET;


	reset_telnet_protocol();
	multiline_length = 0;

    tintin_puts2(rs::rs(1189));
    connectresult=proxy_connect(sock,(struct sockaddr *)&sockaddr, sizeof(sockaddr));

    if(connectresult) {
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
			acPingBuffer[i] = 'a' + (i % 24);
		hIcmp = LoadLibrary("ICMP.DLL");
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

		lPingMUD = ping_single_host(MUDAddress.sin_addr.s_addr, time_step_ms * 2);
		lPingProxy = ping_single_host(htonl(ulProxyAddress), time_step_ms * 2);

		PostMessage(hwndMAIN, WM_USER+680, (WPARAM)lPingMUD, (LONG)lPingProxy);

		int dt = GetTickCount() - t0;
		if (dt < time_step_ms)
			Sleep(time_step_ms - dt);
	}
    return 0;
}


void connect_mud(char *host, char *port)
{
    DWORD dwThreadID;

    if ( strlen(host) > 120 || strlen(port) > 60 ) {
        tintin_puts2(rs::rs(1194));
        return;
    }
    strcpy(strConnectAddress, host);
    strcpy(strConnectPort, port);

	hConnThread = CreateThread(NULL, 0, &ConnectThread, NULL, 0, &dwThreadID);
}


void ShowError (char* strError)
{
    MessageBox(hwndMAIN, strError , rs::rs(1195) , MB_OK | MB_ICONSTOP );
}


void tintin_puts(char *cptr)
{
    tintin_puts2(cptr);  
    check_all_actions(cptr, false); 
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
    if ( GetPrivateProfileInt("Debug" , "ExLog" , 0 , "jmc.ini" ) ) {
        hExLog = CreateFile("socket.log" , GENERIC_READ| GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, NULL, NULL );
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

}

//* en:fix to allow DROPPING reload message
void add_line_to_multiline(char *line);
void do_one_line(char *line);
void do_multiline();
//*/en

void  DLLEXPORT ReloadScriptEngine(LPCSTR strScriptText, GUID guidEngine, LPCSTR strProfile)
{
    pJmcObj->m_bstrProfile = strProfile;
    pSite->InitSite (hwndMAIN, strScriptText,  guidEngine);
//* en:fix to allow DROPPING reload message
    //tintin_puts(rs::rs(1196));
	char* buff = rs::rs(1196);
                    do_one_line(buff);
                    if ( !(*buff== '.' && *(buff+1) == 0 ) ) {
						tintin_puts2(buff);
                    }
//*/en
}

//* en:fix to allow DROPPING reload message
//void do_one_line(char *line);
//*/en

void DLLEXPORT CompileInput(char* str)
{
    USES_CONVERSION;

	if (last_line[0]) {
		//process_incoming(" [GA forced]\x01");
		process_incoming("\x01");
	}

    if ( WaitForSingleObject (eventAllObjectEvent, 0) == WAIT_OBJECT_0  ) {
        // Make event 
        pJmcObj->m_pvarEventParams[0] = str;

        if ( !pJmcObj->Fire_Input () ) {
            SetEvent(eventAllObjectEvent );
            return;
        }

        char line[BUFFER_SIZE];
        strcpy(line, W2A(pJmcObj->m_pvarEventParams[0].bstrVal ));
        //parse_input(line);
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
	  if(!iWaitState && strlen(mQueue))
	  {
		  parse_input(mQueue);
		  mQueue[0]='\0';
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
                    char* buff = rs::rs(1197);
                    do_one_line(buff);
                    if ( !(*buff== '.' && *(buff+1) == 0 ) ) {
						tintin_puts2(buff);
                    }
              }
            Done = 1;
          }
        if(iSecToTick==10 && Done ){
            if ( MUDSocket ) {
                char* buff = rs::rs(1198);
                do_one_line(buff); 
                if ( !(*buff== '.' && *(buff+1) == 0 ) ) {
					tintin_puts2(buff);
                }
            }
          Done = 0;
        }
      }
  } 

}



/**********************************************************/
/* do all of the functions to one line of buffer          */
/**********************************************************/
void add_line_to_multiline(char *line)
{
	int multiline_capacity = sizeof(multiline_buf) - 1; //hold one char for null-terminator
	int rest = multiline_capacity - multiline_length;

	if (rest <= 0)
		return;

	int len = strlen(line);

	if (len <= 0)
		return;

	if (multiline_length > 0) { //add new line
		multiline_buf[multiline_length++] = '\n';
		rest--;

		if (rest <= 0)
			return;
	}

	if (rest < len) {
		// show error?
		len = rest;
	}
    
	memcpy(&multiline_buf[multiline_length], line, len);
	multiline_length += len;
}

void do_one_line(char *line)
{   
    USES_CONVERSION;

    pJmcObj->m_pvarEventParams[0] = (line);
    BOOL bRet = pJmcObj->Fire_Incoming();
    if ( bRet && pJmcObj->m_pvarEventParams[0].vt == VT_BSTR) {
        //strcpy(line, W2A(pJmcObj->m_pvarEventParams[0].bstrVal) );
		strncpy(line, W2A(pJmcObj->m_pvarEventParams[0].bstrVal), BUFFER_SIZE - 1);
		line[BUFFER_SIZE - 1] = '\0';
    } else {
        strcpy(line, "." );
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
	USES_CONVERSION;

	if (!multiline_length)
		return;
	multiline_buf[multiline_length] = 0;
	
    pJmcObj->m_pvarEventParams[0] = (multiline_buf);
    BOOL bRet = pJmcObj->Fire_Prompt();
	if ( bRet && pJmcObj->m_pvarEventParams[0].vt == VT_BSTR) {
		strncpy(multiline_buf, W2A(pJmcObj->m_pvarEventParams[0].bstrVal), sizeof(multiline_buf) - 1);
		multiline_buf[sizeof(multiline_buf)-1] = '\0';
		check_all_actions(multiline_buf, true);
    }

	multiline_length = 0;
}


unsigned char DLLEXPORT substChars[SUBST_ARRAY_SIZE];
BOOL DLLEXPORT bSubstitution;
CRITICAL_SECTION DLLEXPORT secSubstSection;

// for delayed more-coming 
DWORD lastRecvd;
int DLLEXPORT MoreComingDelay = 150;

/*************************************************************/
/* read text from mud and test for actions/substitutes */
/*************************************************************/

static void process_incoming(char* buffer, BOOL FromServer)
{
    char linebuffer[BUFFER_SIZE], *cpsource, *cpdest;
	char line_to_log[BUFFER_SIZE];
    int LastLineLen = 0, n;

    cpsource = buffer; 
    cpdest = linebuffer;
	if (last_line[0]) {
        LastLineLen = strlen(last_line);
        strcpy(linebuffer,last_line);
        cpdest += LastLineLen;
    } 

    BOOL bProcess = TRUE;
    if ( WaitForSingleObject (eventAllObjectEvent, 0) != WAIT_OBJECT_0  ) 
        bProcess = FALSE;

    while(*cpsource) { 
        if ( *cpsource == '\r' ) {
            cpsource++;
            continue;
        }

        if(*cpsource == '\n' || *cpsource == 0x1) {
            *cpdest = '\0';
			
			if ( !bLogAsUserSeen ) {
				strcpy(line_to_log, linebuffer);
			}
            if ( bProcess ) { 
                do_one_line(linebuffer);
				if (*cpsource == 0x1)
					do_multiline();
			}
			if ( bLogAsUserSeen ) {
				strcpy(line_to_log, linebuffer);
			}

			//vls-begin// #logadd + #logpass // multiple output
            if(hLogFile.is_open() && !bLogPassedLine && (bLogAsUserSeen || FromServer)) {
				log(processLine(line_to_log));
				log("\n");
            }

            bLogPassedLine = FALSE;
			//vls-end//

            if( strcmp(linebuffer, ".") ) {
                n = strlen(linebuffer);
				linebuffer[n++] = *cpsource;
				linebuffer[n] = '\0';
				DirectOutputFunction(linebuffer, 0);// out to main window
            }
			last_line[0] = '\0';

            cpsource++;

            cpdest = linebuffer;
        } else {
            *cpdest++ = *cpsource++;
		}
    }
    *cpdest='\0';

	if (linebuffer[0] && strcmp(linebuffer, ".")) {
		strcpy(last_line , linebuffer);
		DirectOutputFunction(linebuffer, 0);// out to main window
		lastRecvd = GetTickCount();
	} else {
		last_line[0] = '\0';
	}

    if ( bProcess  ) 
        SetEvent(eventAllObjectEvent );
}

void read_mud(void )
{
    char buffer[BUFFER_SIZE], processed[BUFFER_SIZE];
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
			if( (didget = recv(MUDSocket, buffer, to_read, 0)) <= 0 ) {
				error = true;
				break;
			}
			len -= didget;

#ifdef _DEBUG_LOG
			// --------   Write external log
			if (hExLog ) {
				char exLogText[128];
				DWORD Written;
				sprintf(exLogText , "\r\n#RECV got %d bytes#\r\n" , didget );
				WriteFile(hExLog , exLogText , strlen(exLogText) , &Written, NULL);
				WriteFile(hExLog , buffer , didget , &Written, NULL);
			}
#endif

			telnet_push_back(buffer, didget);
			while( (didget = telnet_pop_front(processed, sizeof(processed) - 1)) > 0 ) {
				decoded += didget;
				processed[didget] = '\0';

#ifdef _DEBUG_LOG
				if (hExLog ) {
					char exLogText[128];
					DWORD Written;
					sprintf(exLogText , "\r\n#BUFFER AFTER TELNET#\r\n" );
					WriteFile(hExLog , exLogText , strlen(exLogText) , &Written, NULL);
					WriteFile(hExLog , processed , didget , &Written, NULL);
				}
#endif

				//old_more_coming = more_coming;
				//if ( didget > 0 && processed[didget - 1] != '\n' && processed[didget - 1] != 0x1 ) {
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
//* en:fix to allow ACTING zap message
		char* buff = rs::rs(1199);
        do_one_line(buff);
        if ( !(*buff== '.' && *(buff+1) == 0 ) ) {
			tintin_puts2(buff);
        }
//*/en
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


BOOL DLLEXPORT ParseScript(LPCSTR strScriptlet)
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
    char dir[MAX_PATH+2];
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
    
    USES_CONVERSION;
    char* p = W2A(bstrScriptlet);


//vls-begin// base dir
//    hr = pSite->m_ScriptParser->ParseScriptText(bstrScriptlet, NULL, NULL, NULL, 0, 0, 0, NULL, &ei);
    char dir[MAX_PATH+2];
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
				//process_incoming("\n");
				process_incoming("\x01");
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
            static char buf[BUFFER_SIZE];
			if (nMudEmuTextSize > 0) {
				memcpy(buf, strMudEmuText, nMudEmuTextSize);
				buf[nMudEmuTextSize] = 0;
            
				if ( buf[nMudEmuTextSize-1] != 0xA && buf[nMudEmuTextSize-1]!= 1 )
					more_coming = 1;
				else 
					more_coming = 0;

				nMudEmuTextSize = 0;
			} else {
				buf[0] = '\0';
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

		if (BCASTSocket != INVALID_SOCKET) {
			static char buf[BUFFER_SIZE];
			unsigned long len;

			for(;;) {	
				if( ioctlsocket(BCASTSocket, FIONREAD, &len) == SOCKET_ERROR )
					break;
				if( !len )
					break;

				if( len >= sizeof(buf) - 1 )
					len = sizeof(buf) - 2;

				struct sockaddr_in dest;
				int tmp = sizeof(dest);

				if( (len = recvfrom(BCASTSocket, buf, len, 0, (struct sockaddr*)&dest, &tmp)) <= 0)
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
					buf[len++] = '\n';
					buf[len] = '\0';
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

void MultiactionCommand(char* arg)
{
    char left[BUFFER_SIZE];
    arg=get_arg_in_braces(arg, left, STOP_SPACES);

    if ( *left ) {
        if ( !_strcmpi("on", left) ) {
            bMultiAction = TRUE;
            tintin_puts2(rs::rs(1200));
        } else 
            if ( !_strcmpi("off", left) ) {
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

void MultiHlightCommand(char* arg)
{
    char left[BUFFER_SIZE];
    arg=get_arg_in_braces(arg, left, STOP_SPACES);

    if ( *left ) {
        if ( !_strcmpi("on", left) ) {
            bMultiHighlight  = TRUE;
            tintin_puts2(rs::rs(1205));
        } else 
            if ( !_strcmpi("off", left) ) {
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
void woutput_command(char* arg)
{ 
    char number[BUFFER_SIZE];
    char left[BUFFER_SIZE], right[BUFFER_SIZE];
    char result[BUFFER_SIZE], strng[BUFFER_SIZE];
    int wnd = MAX_OUTPUT;
    int i, ok;
    
    arg=get_arg_in_braces(arg, number, STOP_SPACES);
    arg=get_arg_in_braces(arg, left,   WITH_SPACES);
    arg=get_arg_in_braces(arg, right,  WITH_SPACES);

    // checking first parameter to be all digits
    ok = 1;
    for (i = 0; number[i]; i++) {
        if (number[i] < '0' || number[i] > '9') {
            ok = 0;
            break;
        }
    }

    if (!ok || !sscanf(number, "%d", &wnd) || wnd < 0 || wnd >= MAX_OUTPUT) {
        tintin_puts(rs::rs(1239));
        return;
    }

    if ( !right[0] ) {  // no colors
        prepare_actionalias(left,strng, sizeof(strng)); 
    } else {
        prepare_actionalias(right,result, sizeof(result)); 
        add_codes(result, strng, left);
    }
    tintin_puts3(strng, wnd);
}

void wshow_command(char *arg)
{
    char number[BUFFER_SIZE];
    char option[BUFFER_SIZE];
    int wnd = MAX_OUTPUT;
    int opt;
    int i, ok;

    arg=get_arg_in_braces(arg, number, STOP_SPACES);
    arg=get_arg_in_braces(arg, option, STOP_SPACES);

    // checking first parameter to be all digits
    ok = 1;
    for (i = 0; number[i]; i++) {
        if (number[i] < '0' || number[i] > '9') {
            ok = 0;
            break;
        }
    }

    if (!ok || !sscanf(number, "%d", &wnd) || wnd < 0 || wnd >= MAX_OUTPUT) {
        tintin_puts(rs::rs(1240));
        return;
    }

    if ( option[0] == 0 || (option[0] == 't' && is_abrev(option, "toggle")) )
        opt = 0;
    else if ( option[0] == 'h' && is_abrev(option, "hide") )
        opt = 1;
    else if ( option[0] == 's' && is_abrev(option, "show") )
        opt = 2;
    else {
        tintin_puts(rs::rs(1240));
        return;
    }

    PostMessage(hwndMAIN, WM_USER+500, (WPARAM)(wnd), (LPARAM)(opt));
}

void wname_command(char *arg)
{
    char number[BUFFER_SIZE];
    char option[BUFFER_SIZE];
    int wnd = MAX_OUTPUT;
    int i, ok;

    arg=get_arg_in_braces(arg, number, STOP_SPACES);
    arg=get_arg_in_braces(arg, option, STOP_SPACES);

    // checking first parameter to be all digits
    ok = 1;
    for (i = 0; number[i]; i++) {
        if (number[i] < '0' || number[i] > '9') {
            ok = 0;
            break;
        }
    }

    if (!ok || !sscanf(number, "%d", &wnd) || wnd < 0 || wnd >= MAX_OUTPUT) {
        tintin_puts(rs::rs(1244));
        return;
    }

    HGLOBAL hg = GlobalAlloc(GHND, strlen(option)+2);
    char* p = (char*)GlobalLock(hg);
    strcpy(p, option);
    GlobalUnlock (hg);

    PostMessage(hwndMAIN, WM_USER+501, (WPARAM)wnd, (LPARAM)hg);
}

void wdock_command(char *arg)
{
    char number[BUFFER_SIZE];
    char option[BUFFER_SIZE];
    LONG enable;
    int wnd = MAX_OUTPUT;
    int i, ok;

    arg=get_arg_in_braces(arg, number, STOP_SPACES);
    arg=get_arg_in_braces(arg, option, STOP_SPACES);

    // checking first parameter to be all digits
    ok = 1;
    for (i = 0; number[i]; i++) {
        if (number[i] < '0' || number[i] > '9') {
            ok = 0;
            break;
        }
    }

	enable = 1;
	if ( is_abrev(option, "disable") )
		enable = 0;
	else if ( is_abrev(option, "left") )
		enable = 2;
	else if ( is_abrev(option, "top") )
		enable = 3;
	else if ( is_abrev(option, "right") )
		enable = 4;
	else if ( is_abrev(option, "bottom") )
		enable = 5;

    if (!ok || !sscanf(number, "%d", &wnd) || wnd < 0 || wnd >= MAX_OUTPUT) {
        tintin_puts(rs::rs(1260));
        return;
    }

    PostMessage(hwndMAIN, WM_USER+502, (WPARAM)wnd, enable);
}

DWORD DLLEXPORT wposes[MAX_OUTPUT][2];
void wpos_command(char *arg)
{
    char number[BUFFER_SIZE];
    char point1[BUFFER_SIZE];
    char point2[BUFFER_SIZE];
    LONG points;
    LONG wnd = MAX_OUTPUT;
    int i, ok, p1,p2;

    arg=get_arg_in_braces(arg, number, STOP_SPACES);
    arg=get_arg_in_braces(arg, point1, STOP_SPACES);
    arg=get_arg_in_braces(arg, point2, STOP_SPACES);

    // checking first parameter to be all digits
    ok = TRUE;
    for (i = 0; number[i]; i++) {
        if (number[i] < '0' || number[i] > '9') {
            ok = 0;
            break;
        }
    }

	p1 = (atoi(point1) > 2048) ? 2048 : atoi(point1);
	p2 = (atoi(point2) > 2048) ? 2048 : atoi(point2);

    if (!ok || !sscanf(number, "%d", &wnd) || wnd < 0 || wnd >= MAX_OUTPUT) {
        tintin_puts(rs::rs(1244));
        return;
    }

	if(!(*point1))
	{
		sprintf(number,rs::rs(1256),wnd,wposes[wnd][0],wposes[wnd][1]);
		tintin_puts2(number);
		return;
	}
	if(is_abrev(point1,"reposition"))
	{
		p1 = wposes[wnd][0];
		p2 = wposes[wnd][1];
	}

	wposes[wnd][0] = p1;
	wposes[wnd][1] = p2;
	points = MAKELPARAM(p1,p2);
    PostMessage(hwndMAIN, WM_USER+505, wnd, points);
}

//vls-end//

void DLLEXPORT reopen_bcast_socket()
{
	if( BCASTSocket != INVALID_SOCKET ) {
		closesocket(BCASTSocket);
	}
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

