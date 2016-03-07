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

#define SUBST_ARRAY_SIZE 68
#define BUFFER_SIZE 2048

typedef void (CALLBACK* END_FUNC)(void);
typedef void (CALLBACK* DIRECT_OUT_FUNC)(char*, BOOL);
typedef void (CALLBACK* CLEAR_WINDOW_FUNC)(int);
typedef void (CALLBACK* SET_HOTKEYS)(char*, char*);
typedef void (CALLBACK* SET_HOTSET)(char*);
//vls-begin// multiple output
typedef void (CALLBACK* GET_OUTPUTNAME_FUNC)(int, char*, int);

void DLLEXPORT InitOutputNameFunc(GET_OUTPUTNAME_FUNC OutNameFunc);
//vls-end//

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
extern BOOL DLLEXPORT bPasswordEcho ;
extern BOOL DLLEXPORT bConnectBeep;
extern BOOL DLLEXPORT bAutoReconnect;
extern BOOL DLLEXPORT bDisplayCommands;
extern BOOL DLLEXPORT bDisplayInput;
extern BOOL DLLEXPORT bMinimizeToTray;
extern BOOL DLLEXPORT bHTML;
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
extern char DLLEXPORT strMudEmuText[513];

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

#endif /* */

