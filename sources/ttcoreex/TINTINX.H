#ifndef __TINTIN_H_
#define __TINTIN_H_

#ifdef _TTCORE_DLL
#define DLLEXPORT __declspec( dllexport )
#else
#define DLLEXPORT __declspec( dllimport )
#endif

//vls-begin// multiple output
#define MAX_OUTPUT 10
//vls-end//

#define SUBST_ARRAY_SIZE		68
#define BUFFER_SIZE				2048
//#define MULTILINE_BUFFER_SIZE	(BUFFER_SIZE*32)
#define MULTILINE_BUFFER_SIZE	(BUFFER_SIZE*1)
#define EMULATOR_BUFFER_SIZE	1024
#define MAX_VARNAME_LENGTH		64

#define MIN_SCROLL_SIZE			100
#define MAX_SCROLL_SIZE			30000


#define END_OF_PROMPT_MARK		'\x01'
#define USER_INPUT_MARK			'\x01'
#define TINTIN_OUTPUT_MARK		'\x02'
#define ESC_SEQUENCE_MARK		'\x1B'

typedef void (CALLBACK* END_FUNC)(void);
typedef void (CALLBACK* DIRECT_OUT_FUNC)(char*, int);
typedef void (CALLBACK* CLEAR_WINDOW_FUNC)(int);
typedef void (CALLBACK* SET_HOTKEYS)(char*, char*);
typedef void (CALLBACK* SET_HOTSET)(char*);
//vls-begin// multiple output
typedef void (CALLBACK* GET_OUTPUTNAME_FUNC)(int, char*, int);

void DLLEXPORT InitOutputNameFunc(GET_OUTPUTNAME_FUNC OutNameFunc);
//vls-end//

typedef void (CALLBACK* GET_WNDSIZE_FUNC)(int, int &, int &);
typedef void (CALLBACK* SET_WNDSIZE_FUNC)(int, int  , int  );

void DLLEXPORT InitWindowSizeFunc(GET_WNDSIZE_FUNC GetWndFunc, SET_WNDSIZE_FUNC SetWndFunc);

void  DLLEXPORT InitState(DIRECT_OUT_FUNC OutFunc, CLEAR_WINDOW_FUNC ClearFunc, HWND mainWnd);
void  DLLEXPORT CloseState(void);
void  DLLEXPORT ReloadScriptEngine(LPCSTR strScriptText, GUID guidEngine, LPCSTR strProfile);
void  DLLEXPORT CompileInput(char* str);
void  DLLEXPORT ReadMud();
BOOL  DLLEXPORT IsConnected();
BOOL  DLLEXPORT IsLogging();
BOOL  DLLEXPORT IsPathing();
BOOL  DLLEXPORT ParseScript(LPCSTR strScriptlet);
void  DLLEXPORT BreakScript();
void  DLLEXPORT LunchDebuger();

LONG  DLLEXPORT GetCommandsList(char *List);

const DLLEXPORT char * act_type_to_str(int type);

extern BOOL DLLEXPORT bBCastFilterIP;
extern BOOL DLLEXPORT bBCastFilterPort;
extern WORD DLLEXPORT wBCastUdpPort;
extern void DLLEXPORT reopen_bcast_socket();

extern unsigned char DLLEXPORT strPromptEndSequence[BUFFER_SIZE];
extern unsigned char DLLEXPORT strPromptEndReplace[BUFFER_SIZE];
extern BOOL DLLEXPORT bPromptEndEnabled;
//vls-begin// base dir
extern char DLLEXPORT szBASE_DIR[MAX_PATH];
extern char DLLEXPORT szSETTINGS_DIR[MAX_PATH];
extern char DLLEXPORT szPROFILESCRIPT[MAX_PATH];

int DLLEXPORT MakeLocalPath(char *loc, const char *abs, const char *base);
int DLLEXPORT MakeAbsolutePath(char *abs, const char *loc, const char *base);
//vls-end//

extern unsigned char DLLEXPORT substChars[SUBST_ARRAY_SIZE];
extern BOOL DLLEXPORT bSubstitution;
extern CRITICAL_SECTION DLLEXPORT secSubstSection;

extern char DLLEXPORT cCommandChar;
extern char DLLEXPORT cCommandDelimiter;
extern BOOL DLLEXPORT bDefaultLogMode; // TRUE - append , FALSE - ovewrite
extern BOOL DLLEXPORT bRMASupport;
extern BOOL DLLEXPORT bAppendLogTitle;
extern BOOL DLLEXPORT bANSILog;
extern int DLLEXPORT iSecToTick;
extern BOOL DLLEXPORT bTickStatus;
extern BOOL DLLEXPORT bConnectBeep;
extern BOOL DLLEXPORT bAutoReconnect;
extern BOOL DLLEXPORT bDisplayCommands;
extern BOOL DLLEXPORT bDisplayInput;
extern BOOL DLLEXPORT bInputOnNewLine;
extern BOOL DLLEXPORT bDisplayPing;
extern BOOL DLLEXPORT bMinimizeToTray;
extern BOOL DLLEXPORT bHTML;
extern BOOL DLLEXPORT bHTMLTimestamps;
extern BOOL DLLEXPORT bLogAsUserSeen;
extern BOOL DLLEXPORT bAllowDebug;
extern BOOL DLLEXPORT bIACSendSingle, bIACReciveSingle;
extern int DLLEXPORT nScripterrorOutput;

extern int DLLEXPORT MoreComingDelay;

extern char DLLEXPORT strInfo1[BUFFER_SIZE];
extern char DLLEXPORT strInfo2[BUFFER_SIZE];
extern char DLLEXPORT strInfo3[BUFFER_SIZE];
extern char DLLEXPORT strInfo4[BUFFER_SIZE];
extern char DLLEXPORT strInfo5[BUFFER_SIZE];
extern CRITICAL_SECTION DLLEXPORT secStatusSection;

extern char DLLEXPORT langfile[BUFFER_SIZE];
extern char DLLEXPORT langsect[BUFFER_SIZE];
extern char DLLEXPORT editStr[BUFFER_SIZE];
extern DWORD DLLEXPORT wposes[MAX_OUTPUT][2];

LPCSTR DLLEXPORT GetHotkeyValue(WORD key);

extern CRITICAL_SECTION DLLEXPORT secHotkeys;
extern HANDLE DLLEXPORT eventAllObjectEvent;

extern HANDLE DLLEXPORT eventMudEmuTextArrives;
extern int DLLEXPORT nMudEmuTextSize;
extern char DLLEXPORT strMudEmuText[EMULATOR_BUFFER_SIZE];

extern int DLLEXPORT nScrollSize;

//vls-begin// #system
extern CRITICAL_SECTION DLLEXPORT secSystemExec;
extern CRITICAL_SECTION DLLEXPORT secSystemList;
//vls-end//

//vls-begin// script files
extern CRITICAL_SECTION DLLEXPORT secScriptFiles;
extern CRITICAL_SECTION DLLEXPORT secReadingConfig;
extern HANDLE DLLEXPORT eventReadingConfig;
extern HANDLE DLLEXPORT eventReadingHasUse;
extern HANDLE DLLEXPORT eventReadingFirst;
//vls-end//

extern DWORD DLLEXPORT ulProxyAddress;
extern WORD DLLEXPORT dwProxyPort;
extern WORD DLLEXPORT dwProxyType;
extern char DLLEXPORT sProxyUserName[256];
extern char DLLEXPORT sProxyUserPassword[256];

#endif /* */

