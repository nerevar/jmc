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
#define MULTILINE_BUFFER_SIZE	(BUFFER_SIZE*32)
#define EMULATOR_BUFFER_SIZE	(BUFFER_SIZE/2)
#define MAX_VARNAME_LENGTH		128

#define MIN_SCROLL_SIZE			100
#define MAX_SCROLL_SIZE			30000


#define END_OF_PROMPT_MARK		L'\x01'
#define USER_INPUT_MARK			L'\x01'
#define TINTIN_OUTPUT_MARK		L'\x02'
#define ESC_SEQUENCE_MARK		L'\x1B'

typedef void (CALLBACK* END_FUNC)(void);
typedef void (CALLBACK* DIRECT_OUT_FUNC)(const wchar_t*, int);
typedef void (CALLBACK* CLEAR_WINDOW_FUNC)(int);
typedef void (CALLBACK* SET_HOTKEYS)(wchar_t*, wchar_t*);
typedef void (CALLBACK* SET_HOTSET)(wchar_t*);
//vls-begin// multiple output
typedef void (CALLBACK* GET_OUTPUTNAME_FUNC)(int, wchar_t*, int);

void DLLEXPORT InitOutputNameFunc(GET_OUTPUTNAME_FUNC OutNameFunc);
//vls-end//

typedef void (CALLBACK* GET_WNDSIZE_FUNC)(int, int &, int &);
typedef void (CALLBACK* SET_WNDSIZE_FUNC)(int, int  , int  );

void DLLEXPORT InitWindowSizeFunc(GET_WNDSIZE_FUNC GetWndFunc, SET_WNDSIZE_FUNC SetWndFunc);

void  DLLEXPORT InitState(DIRECT_OUT_FUNC OutFunc, CLEAR_WINDOW_FUNC ClearFunc, HWND mainWnd);
void  DLLEXPORT CloseState(void);
void  DLLEXPORT ReloadScriptEngine(const wchar_t* strScriptText, GUID guidEngine, const wchar_t* strProfile);
void  DLLEXPORT CompileInput(const wchar_t* str);
void  DLLEXPORT ReadMud();
BOOL  DLLEXPORT IsConnected();
BOOL  DLLEXPORT IsLogging();
BOOL  DLLEXPORT IsPathing();
BOOL  DLLEXPORT ParseScript(const wchar_t* strScriptlet);
void  DLLEXPORT BreakScript();
void  DLLEXPORT LunchDebuger();

LONG  DLLEXPORT GetCommandsList(wchar_t *List);

LPWSTR DLLEXPORT act_type_to_str(int type);

extern BOOL DLLEXPORT bBCastEnabled;
extern BOOL DLLEXPORT bBCastFilterIP;
extern BOOL DLLEXPORT bBCastFilterPort;
extern WORD DLLEXPORT wBCastUdpPort;
extern void DLLEXPORT reopen_bcast_socket();

extern wchar_t DLLEXPORT strPromptEndSequence[BUFFER_SIZE];
extern wchar_t DLLEXPORT strPromptEndReplace[BUFFER_SIZE];
extern BOOL DLLEXPORT bPromptEndEnabled;
//vls-begin// base dir
extern wchar_t DLLEXPORT szBASE_DIR[MAX_PATH];
extern wchar_t DLLEXPORT szSETTINGS_DIR[MAX_PATH];
extern wchar_t DLLEXPORT szPROFILESCRIPT[MAX_PATH];

extern DLLEXPORT UINT MudCodePage;
extern DLLEXPORT UINT MudCodePageUsed;
extern DLLEXPORT UINT LogCodePage;
extern int DLLEXPORT read_file_contents(const wchar_t *FilePath, wchar_t *Buffer, int Capacity);
extern int DLLEXPORT write_file_contents(const wchar_t *FilePath, const wchar_t *Buffer, int Length);
extern int DLLEXPORT enumerate_codepage(int Index, wchar_t *Name, int MaxNameSize);

int DLLEXPORT MakeLocalPath(wchar_t *loc, const wchar_t *abs, const wchar_t *base);
int DLLEXPORT MakeAbsolutePath(wchar_t *abs, const wchar_t *loc, const wchar_t *base);
//vls-end//

extern wchar_t DLLEXPORT substChars[SUBST_ARRAY_SIZE];
extern BOOL DLLEXPORT bSubstitution;
extern CRITICAL_SECTION DLLEXPORT secSubstSection;

extern wchar_t DLLEXPORT cCommandChar;
extern wchar_t DLLEXPORT cCommandDelimiter;
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

extern wchar_t DLLEXPORT strInfo1[BUFFER_SIZE];
extern wchar_t DLLEXPORT strInfo2[BUFFER_SIZE];
extern wchar_t DLLEXPORT strInfo3[BUFFER_SIZE];
extern wchar_t DLLEXPORT strInfo4[BUFFER_SIZE];
extern wchar_t DLLEXPORT strInfo5[BUFFER_SIZE];
extern CRITICAL_SECTION DLLEXPORT secStatusSection;

extern wchar_t DLLEXPORT langfile[BUFFER_SIZE];
extern wchar_t DLLEXPORT langsect[BUFFER_SIZE];
extern wchar_t DLLEXPORT editStr[BUFFER_SIZE];
extern DWORD DLLEXPORT wposes[MAX_OUTPUT][2];

LPWSTR DLLEXPORT GetHotkeyValue(WORD key);

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

extern wchar_t DLLEXPORT strProductName[256];
extern wchar_t DLLEXPORT strProductVersion[256];

extern wchar_t DLLEXPORT strLastCommand[BUFFER_SIZE];

#endif /* */

