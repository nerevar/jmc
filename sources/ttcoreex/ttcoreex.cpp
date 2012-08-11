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

// ------ Importnant variables for global session ------
struct listnode *common_subs;
struct listnode *common_antisubs, *common_pathdirs, *common_path;

SOCKET MUDSocket;

char vars[10][BUFFER_SIZE]; /* the %0, %1, %2,....%9 variables */

char DLLEXPORT cCommandChar='#';
char DLLEXPORT cCommandDelimiter=';';
BOOL DLLEXPORT bDefaultLogMode = FALSE; // Overwrite 
BOOL DLLEXPORT bRMASupport;
BOOL DLLEXPORT bANSILog;
int DLLEXPORT iSecToTick;
BOOL DLLEXPORT bTickStatus;
BOOL DLLEXPORT bPasswordEcho = TRUE;
BOOL DLLEXPORT bConnectBeep;
BOOL DLLEXPORT bAutoReconnect;
BOOL DLLEXPORT bHTML;
BOOL DLLEXPORT bAllowDebug = FALSE;
BOOL DLLEXPORT bIACSendSingle, bIACReciveSingle;
int DLLEXPORT nScripterrorOutput; // 0 - msgbox, 1- window, 2- output
BOOL DLLEXPORT bDisplayCommands = FALSE;
BOOL DLLEXPORT bDisplayInput = TRUE;

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
char DLLEXPORT strMudEmuText[513];

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
HANDLE hLogFile;
ofstream logFile;
//vls-begin// multiple output
HANDLE hOutputLogFile[MAX_OUTPUT];
//vls-end//
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

static BOOL bReadingMud = FALSE;
static char strIncomingResult[BUFFER_SIZE];
static char* pIncomigResultPos;

void tintin_puts2(char *cptr)
{
    if ( bReadingMud ) {
        int n=strlen(cptr);
        memcpy(pIncomigResultPos, cptr, n);
        pIncomigResultPos+=n;
        *pIncomigResultPos++= '\n';// *cpsource++;
    } else {
        char buff[BUFFER_SIZE];
        strcpy ( buff , cptr);
        strcat(buff, "\n");
        DirectOutputFunction(buff, 0); // out to main wnd
    }
}

//vls-begin// multiple output
//void tintin_puts3(char *cptr)
// TODO: wnd
void tintin_puts3(char *cptr, int wnd)
//vls-end
{
    char buff[BUFFER_SIZE];

    strcpy ( buff , cptr);
    strcat(buff, "\n");

//vls-begin// multiple output
//    DirectOutputFunction(buff, 1); // out to output window
    if ( hOutputLogFile[wnd] ) {
        //WriteToLog(wnd, cptr, strlen(cptr));
        //WriteToLog(wnd, "\r\n", 2);
	}

    DirectOutputFunction(buff, 1+wnd); // out to output window
//vls-end//
}

void output_command(char* arg)
{ 
    char left[BUFFER_SIZE], right[BUFFER_SIZE];
    char result[BUFFER_SIZE], strng[BUFFER_SIZE];
    
    arg=get_arg_in_braces(arg, left, WITH_SPACES);
    arg=get_arg_in_braces(arg, right, WITH_SPACES);

    if ( !right[0] ) {  // no colors
        prepare_actionalias(left,strng); 
    } else {
        prepare_actionalias(right,result); 
        add_codes(result, strng, left);
    }
//vls-begin// multiple output
//    tintin_puts3(strng);
    tintin_puts3(strng, 0);
//vls-end//
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
	char daaString[BUFFER_SIZE];
    daaString[0] = '<';
	for(int i=2;i<strlen(line);i++)
		daaString[i-1]='*';
    daaString[strlen(line)-1] = '>';
	daaString[strlen(line)] = 0;
	

    if(bDisplayInput && strlen(line)>0) 
	{
        std::string str;
        str = "\x1B[0;33m";
		str += bDaaMessage ? daaString : line;
        str += "\x1B[0m";
        tintin_puts2((char*)str.c_str());
    }

    if(logFile) {
		log(processLine(bDaaMessage ? daaString : line, strlen(line)));
		log(processLine("\n", 1));
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


    tintin_puts2(rs::rs(1189));
    connectresult=connect(sock,(struct sockaddr *)&sockaddr, sizeof(sockaddr));

    if(connectresult) {
        closesocket(sock);
        switch(connectresult) {
        case WSAECONNREFUSED:
            tintin_puts2(rs::rs(1190));
            break;
        case WSAENETUNREACH:
            tintin_puts2(rs::rs(1191));
            break;
        default:
            tintin_puts2(rs::rs(1192));
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
    tintin_puts(rs::rs(1193));
    MUDSocket = sock;
    SetEvent(hConnectedEvent );
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
    check_all_actions(cptr); 
}


CComObject<CJmcSite>* pSite = NULL;
CComObject<CJmcObj>* pJmcObj = NULL;



void  DLLEXPORT InitState(/*END_FUNC EndFunc, */DIRECT_OUT_FUNC OutFunc, HWND mainWnd)
{
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
void do_one_line(char *line);
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
                        strcat(buff, "\n");
                        DirectOutputFunction(buff, 0);
                    }
//*/en
}

//* en:fix to allow DROPPING reload message
//void do_one_line(char *line);
//*/en

void DLLEXPORT CompileInput(char* str)
{
    USES_CONVERSION;

    if ( WaitForSingleObject (eventAllObjectEvent, 0) == WAIT_OBJECT_0  ) {
        // Make event 
        pJmcObj->m_pvarEventParams[0] = str;

        if ( !pJmcObj->Fire_Input () ) {
            SetEvent(eventAllObjectEvent );
            return;
        }

        char line[BUFFER_SIZE];
        strcpy(line, W2A(pJmcObj->m_pvarEventParams[0].bstrVal ));
        parse_input(line);
        SetEvent(eventAllObjectEvent );
    } else 
        write_line_mud(str);
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
                        strcat(buff, "\n");
                        DirectOutputFunction(buff, 0);
                    }
              }
            Done = 1;
          }
        if(iSecToTick==10 && Done ){
            if ( MUDSocket ) {
                char* buff = rs::rs(1198);
                do_one_line(buff); 
                if ( !(*buff== '.' && *(buff+1) == 0 ) ) {
                    strcat(buff, "\n");
                    DirectOutputFunction(buff, 0);
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

void do_one_line(char *line)
{   
    USES_CONVERSION;

    pJmcObj->m_pvarEventParams[0] = (line);
    BOOL bRet = pJmcObj->Fire_Incoming();
    if ( pJmcObj->m_pvarEventParams[0].vt == VT_BSTR) 
        strcpy(line, W2A(pJmcObj->m_pvarEventParams[0].bstrVal) );
    else 
        strcpy(line, "." );

    if ( bRet ) {
        if (!presub && !ignore)
          check_all_actions(line);
        if (!togglesubs)                            
            if(!do_one_antisub(line))
            do_one_sub(line);
        if (presub && !ignore)
          check_all_actions(line);
        do_one_high(line);
    }
}  


unsigned char DLLEXPORT substChars[SUBST_ARRAY_SIZE];
BOOL DLLEXPORT bSubstitution;
CRITICAL_SECTION DLLEXPORT secSubstSection;

// for delayed more-coming 
DWORD lastRecvd;
int DLLEXPORT MoreComingDelay = 150;

int read_buffer_mud(char *buffer)
{
    int didget;
    char tmpbuf[BUFFER_SIZE], *cpsource, *cpdest;
    
    didget=recv(MUDSocket, tmpbuf, 512, 0);

    old_more_coming=more_coming;
/*    if (didget==512  && tmpbuf[511] != 0xA && !(tmpbuf[510]==TN_IAC && tmpbuf[510]==TN_GA)) {
        more_coming=1;
    }
    else 
        more_coming=0;
*/
   if(didget<=0)  {
        return -1; 
    } else {
#ifdef _DEBUG_LOG
        // --------   Write external log
        if (hExLog ) {
            char exLogText[128];
            DWORD Written;
            sprintf(exLogText , "\r\n#RECV got %d bytes#\r\n" , didget );
            WriteFile(hExLog , exLogText , strlen(exLogText) , &Written, NULL);
            WriteFile(hExLog , tmpbuf , didget , &Written, NULL);
        }
#endif

        tmpbuf[didget] = 0;
        cpsource=tmpbuf;
        cpdest=buffer;

        didget = do_telnet_protecol((unsigned char*)cpsource, cpdest, didget);
        if ( didget && buffer[didget] != '\n' && buffer[didget] != 0x1 ) {
            lastRecvd = GetTickCount();
            more_coming=1;
        } else 
            more_coming=0;

/*        if ( buffer[didget-1] != 0xA && buffer[didget-1]!= 1 )
            more_coming = 1;
        else 
            more_coming = 0;
*/
#ifdef _DEBUG_LOG
        if (hExLog ) {
            char exLogText[128];
            DWORD Written;
            sprintf(exLogText , "\r\n#BUFFER AFTER TELNET#\r\n" );
            WriteFile(hExLog , exLogText , strlen(exLogText) , &Written, NULL);
            WriteFile(hExLog , buffer , didget , &Written, NULL);
        }
#endif

        
  }

  // ---- substituting characters 

    if ( bSubstitution ) {
        int i;
    // start crit section
        EnterCriticalSection(&secSubstSection);
        for ( i= 0 ; i < *((int*)substChars) ; i++ ) {
            int j;
            for ( j = 0 ; j < didget ; j++ ) {
                if ( buffer[j] == substChars[i*2+sizeof(int)] ) {
                    buffer[j] = substChars[i*2+sizeof(int)+1];
                }
            }
        }
        LeaveCriticalSection(&secSubstSection);
    }

  // ---- end substituting characters
/*    if ( didget < 0 ) 
        return 0;
    else   
        return didget;
*/
    return didget;
}



/*************************************************************/
/* read text from mud and test for actions/substitutes */
/*************************************************************/

static void process_incoming(char* buffer)
{
    char linebuffer[BUFFER_SIZE], *cpsource, *cpdest;
    int LastLineLen = 0, n;
    
    
    bReadingMud = TRUE;

    cpsource=buffer; 
    cpdest=linebuffer;
    if (old_more_coming==1) {
        LastLineLen = strlen(last_line);
        strcpy(linebuffer,last_line);
        cpdest+=LastLineLen;
    } 
    pIncomigResultPos = strIncomingResult;
    *pIncomigResultPos = 0;

    BOOL bProcess = TRUE;
    if ( WaitForSingleObject (eventAllObjectEvent, 0) != WAIT_OBJECT_0  ) 
        bProcess = FALSE;

    while(*cpsource) { 
        if ( *cpsource == '\r' ) {
            cpsource++;
            continue;
        }

        if(*cpsource=='\n' /*|| *cpsource=='\r'*/ || *cpsource==0x1) {
            *cpdest='\0';
//vls-begin// #logadd + #logpass // multiple output
//            if(hLogFile) {
//                WriteToLog(linebuffer, strlen(linebuffer)); 
//                WriteToLog("\r\n", 2); 
//            }
//            if ( bProcess  ) 
//                do_one_line(linebuffer);
            if(logFile.is_open() && !bLogPassedLine) {
				log(processLine(linebuffer, strlen(linebuffer)));
				log(processLine("\n", 1));
            }
            if ( bProcess  ) 
                do_one_line(linebuffer);
            bLogPassedLine = FALSE;
//vls-end//
            if( /**linebuffer != 0x1 &&  */ !(*linebuffer=='.' && !*(linebuffer+1)) ) {
                n=strlen(linebuffer);
                memcpy(pIncomigResultPos, linebuffer, n);
                pIncomigResultPos+=n;
                if ( *cpsource != 0x1 ) 
                    *pIncomigResultPos++= '\n';// *cpsource++;
                cpsource++;

                // !!! Dont need anymore. Only \n is valid combination
                // if((*cpsource=='\n' || *cpsource=='\r')&& *(cpsource-1) != *cpsource )
                //    cpsource++; 
            }
            else {
                cpsource++;
                // !!! Dont need anymore. Only \n is valid combination
                // if((*cpsource=='\n' || *cpsource=='\r')&& *(cpsource-1) != *cpsource )
                //    cpsource++;
            }

            cpdest=linebuffer;
        }
        else
            *cpdest++= *cpsource++;
    }
    *cpdest='\0';
    if (more_coming==1) {
        strcpy(last_line , linebuffer);
    }
    else { 
//vls-begin// #logadd + #logpass // multiple output
//        if(hLogFile) {
//            WriteToLog(linebuffer, strlen(linebuffer)); 
//        }
//        if ( bProcess  ) 
//            do_one_line(linebuffer);
        if ( bProcess  ) 
            do_one_line(linebuffer);
        if(logFile.is_open() && !bLogPassedLine) {
			log(processLine(linebuffer, strlen(linebuffer)));
//            WriteToLog(-1, linebuffer, strlen(linebuffer)); 
        }
        bLogPassedLine = FALSE;
//vls-end//
        if( !(*linebuffer=='.' && !*(linebuffer+1)) ) {
            n=strlen(linebuffer);
            memcpy(pIncomigResultPos, linebuffer, n);
            pIncomigResultPos+=n;
        }
    }
    *pIncomigResultPos='\0';

    if ( bProcess  ) 
        SetEvent(eventAllObjectEvent );

    // Sending few string to draw. Need to split it
    DirectOutputFunction(strIncomingResult, 0);// out to main window
    bReadingMud = FALSE;
}

void read_mud(void )
{
    char buffer[BUFFER_SIZE];
    int didget;

                                                                                                               
    if( (didget=read_buffer_mud(buffer)) < 0 ) {
        cleanup_session();
//* en:fix to allow ACTING zap message
					char* buff = rs::rs(1199);
                    do_one_line(buff);
                    if ( !(*buff== '.' && *(buff+1) == 0 ) ) {
                        strcat(buff, "\n");
                        DirectOutputFunction(buff, 0);
                    }
//*/en
        MUDSocket = NULL;
        if ( WaitForSingleObject (eventAllObjectEvent, 0) == WAIT_OBJECT_0  ) {
            pJmcObj->Fire_ConnectLost();
            SetEvent(eventAllObjectEvent );
        }
        
        if ( bAutoReconnect) {
            DWORD dwThreadID;
            hConnThread = CreateThread(NULL, 0, &ConnectThread, NULL, 0, &dwThreadID);
        }
        bReadingMud = FALSE;
        return ;
    }

    process_incoming (buffer);
    return;
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
        select(32, &readfdmask,  0, 0, &timeout);

        if(FD_ISSET(MUDSocket,&readfdmask))
            read_mud();
        else { // do delayed delete 
            // check we have delayed string without \n
            if ( more_coming && GetTickCount() - lastRecvd >= MoreComingDelay ) {
                more_coming = 0;
                old_more_coming = 1;
                process_incoming("\n");
            } else 
                if ( bDelayedActionDelete ) 
                    if ( WaitForSingleObject (eventAllObjectEvent, 0) == WAIT_OBJECT_0 ) {
                        ACTION_INDEX aind = ActionList.begin();
                        while ( aind != ActionList.end() ) {
                            // CActionPtr pac = *aind;
                            ACTION* pac = *aind;
                            if ( pac->m_bDeleted){
                                ACTION_INDEX aind1 = aind;
                                aind++;
                                delete pac;
                                ActionList.erase(aind1);
                            } else 
                                aind++;
                        }
                        bDelayedActionDelete = FALSE;
                        SetEvent(eventAllObjectEvent);
                    } else 
                        return;
        }
    } else {
        if ( WaitForSingleObject (eventMudEmuTextArrives, 0 ) == WAIT_OBJECT_0 ) {
            char buf[BUFFER_SIZE];
            memcpy(buf, strMudEmuText, nMudEmuTextSize);
            buf[nMudEmuTextSize] = 0;
            ResetEvent(eventMudEmuTextArrives);
            old_more_coming=more_coming;
            if ( buf[nMudEmuTextSize-1] != 0xA && buf[nMudEmuTextSize-1]!= 1 )
                more_coming = 1;
            else 
                more_coming = 0;

            process_incoming (buf);
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

        if ( (dwTick - ptm->m_dwLastTickCount)/100 >= ptm->m_nInterval ) {
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
        if ( !strcmpi("on", left) ) {
            bMultiAction = TRUE;
            tintin_puts2(rs::rs(1200));
        } else 
            if ( !strcmpi("off", left) ) {
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
        if ( !strcmpi("on", left) ) {
            bMultiHighlight  = TRUE;
            tintin_puts2(rs::rs(1205));
        } else 
            if ( !strcmpi("off", left) ) {
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
    return (BOOL)hLogFile;
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
        prepare_actionalias(left,strng); 
    } else {
        prepare_actionalias(right,result); 
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

	enable = is_abrev(option,"disable") ? 0 : 1;

    if (!ok || !sscanf(number, "%d", &wnd) || wnd < 0 || wnd >= MAX_OUTPUT) {
        tintin_puts(rs::rs(1244));
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
