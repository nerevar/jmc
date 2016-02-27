// smcDoc.cpp : implementation of the CSmcDoc class
//

#include "stdafx.h"
#include "smc.h"

#include "mainfrm.h"
#include "smcDoc.h"
#include "smcView.h"

#include "AnsiColorsDlg.h"
#include "scrollOptionDlg.h"

//vls-begin// script files
#include "ttcoreex/ttobjects.h"
//vls-end//

#include "KeyWordsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSmcDoc
/*static COLORREF DefColors[16] = {
    RGB(0, 0, 0), 
    RGB(128, 0, 0), 
    RGB(0, 128, 0), 
    RGB(128, 128, 0), 
    RGB(0, 0, 128), 
    RGB(128, 0, 128),
    RGB(0, 128, 128), 
    RGB(192, 192, 192),

    RGB(128, 128, 128), 
    RGB(255, 0, 0), 
    RGB(0, 255, 0), 
    RGB(255, 255, 0), 
    RGB(0, 0, 255), 
    RGB(255, 0, 255), 
    RGB(0, 255, 255), 
    RGB(255, 255, 255)
};*/

CString strInput;

DWORD dwThreadID;

BOOL bExit;
BOOL bLunchDebuger;
CPtrArray KeySets;
CSmcDoc* pDoc = NULL;
CMainFrame* pMainWnd;
//vls-begin// multiple output
//CString pStrLastESC, pStrLastUpdateESC;
CString pStrLastESC;
CString pStrOutputLastESC[MAX_OUTPUT];
//vls-end//
HANDLE  hInputDoneEvent = NULL;
HANDLE  hStateClosedEvent = NULL;
CCriticalSection InputSection;

CCriticalSection ReloadScriptsSection;
BOOL bReloadScripts = FALSE; // indicates script engine should be reloaded
CString strScriptText;

static int LastParsedBg = 40, LastParsedFg = 37, LastParsedBold = 0;

//vls-begin// script files
BOOL bScriptFileListChanged = FALSE;
//vls-end//

static void ParseAnsiValues (char* AnsiStr, CString* pLastESC)
{
    char* src = AnsiStr;
    char Num[128] ;

    do {
        char* dest = Num;
        while (isdigit(*src) ) {
            *dest++ = *src++;
        }
        *dest = 0;
        if ( Num[0] ) {
            int Value = atoi(Num);
            if ( Value <38 && Value >=30 ) 
                LastParsedFg = Value;
            else 
                if ( Value < 48 && Value >= 40 )
                    LastParsedBg = Value;
                else 
                    if ( Value == 0 ) {
                        LastParsedFg = 37;
                        LastParsedBg = 40;
                        LastParsedBold = 0;
                    }
                    else  
                        if ( Value == 1 )
                            LastParsedBold = 1;

        }
    } while (*src++ ) ;

    pLastESC->Format("%c[%d;%d;%dm" , 0x1B, LastParsedBold, LastParsedFg, LastParsedBg );
}

static void AddToOutList(char* str, int wndCode)
{
    char  strAdd[BUFFER_SIZE];

    CStringList* pList;
    CString* pLastEsc;
    int * pCount ;
    int CharSize ;
    if ( wndCode) {
//vls-begin// multiple output
//        pList = &pDoc->m_strOutoputTempList ;
//        pLastEsc = &pStrLastUpdateESC ;
//        pCount = &pDoc->m_nOutputUpdateCount ;
//        CharSize = pDoc->m_nOutWindowCharsSize ;
        wndCode = wndCode > MAX_OUTPUT ? 0 : wndCode-1;
        pList = &(pDoc->m_strOutputTempList[wndCode]);
        pLastEsc = &pStrOutputLastESC[wndCode];
        pCount = &(pDoc->m_nOutputUpdateCount[wndCode]);
        CharSize = pDoc->m_nOutputWindowCharsSize[wndCode];

//vls-end//
    }  else {
        pList = &pDoc->m_strTempList;
        pLastEsc = &pStrLastESC ;
        pCount = &pDoc->m_nUpdateCount ;
        CharSize = pDoc->m_nWindowCharsSize ;
    }

    CString strTail;
	if ( !pList->GetCount() ) 
		pList->AddTail("");

	strTail = pList->GetTail();
	
    BOOL bSetTail = FALSE;

    int TailLen = strTail.GetLength();
    if ( TailLen && strTail[TailLen-1] != '\n' /*&& strTail[TailLen-1] != '\r' */) {
        strcpy(strAdd , (LPCSTR)strTail);
        strcat(strAdd, str);
        bSetTail = TRUE;
    }
    else {
        strcpy(strAdd, (LPCSTR)*pLastEsc);
        strcat(strAdd, str);
    }

    // Now we are ready to parse string, split it to few strings to fit to window etc.
    // Lets start
    char AllANSIFromCurrString[BUFFER_SIZE] = "";
    char* src = strAdd;
    char* ansi = AllANSIFromCurrString;
    char OutputBuffer[BUFFER_SIZE] = "";
    char* dest = OutputBuffer;
    int OutTextLen = 0;

    do {
        switch ( *src ) {
        case 0x1B:
            // Now skip ansi and save it to AllANSIFromCurrString
            do {
                *ansi++ = *src;
                *dest++ = *src++;
			//} while ( *src == '[' || *src == ';' || isdigit(*src) ) ;
            } while ( *src && *src != 'm') ;
            *ansi++ = *src;
            *dest++ = *src;
            *ansi = 0;
            // Ansi color skipped
            if ( *src == 'm' ) 
                break;
			// All the other esc-sequences skipped and erased
			while ( *ansi != 0x1B )
				ansi--;
			*ansi = 0;
        case 0:
            // End of line - time to do all save operations 
            *dest = 0;
            // Buffer filled , now parse ansi and save values
            ParseAnsiValues(AllANSIFromCurrString, pLastEsc);
            if ( bSetTail ) {
                pList->SetAt(pList->GetTailPosition(), OutputBuffer);
                bSetTail = FALSE;
            }
            else {
                pList->AddTail(OutputBuffer);
                *pCount = *pCount + 1;
            }
            break;
        default:
            // Copy character to OutputBuffer and watch for m_nWindowCharsSize 
            *dest++ = *src;
            OutTextLen++;
            // check for size 
            if ( OutTextLen >= CharSize ) {
                // Save to list and reinit pointers
                *dest++ = '\n';
                *dest = 0;
                if ( bSetTail ) {
                    pList->SetAt(pList->GetTailPosition(), OutputBuffer);
                    bSetTail = FALSE;
                }
                else {
                    pList->AddTail(OutputBuffer);
                    *pCount = *pCount + 1;
                }
                ParseAnsiValues(AllANSIFromCurrString, pLastEsc);
                strcpy(OutputBuffer, (LPCSTR)*pLastEsc);
                dest = OutputBuffer + pLastEsc->GetLength();
                ansi = AllANSIFromCurrString;
                *ansi = 0;
                OutTextLen = 0;
            }
            break;
        };
    } while (*src++ ) ;
}


// n - A   \r\n !!!!!
// r - D


static void __stdcall OutTextFrom(char* str, int wndCode) 
{

    if ( pDoc ) {
        CCriticalSection* pSec;
        if ( wndCode )
//vls-begin// multiple output
//            pSec = &pDoc->m_UpdateOutputSection;
            pSec = &(pDoc->m_OutputUpdateSection[wndCode > MAX_OUTPUT ? 0 : wndCode-1]);
        else 
            pSec = &pDoc->m_UpdateSection ;

        pSec->Lock();
        // now i have to split buffer to few strings
        char* src = str;
        char buff[BUFFER_SIZE];
        char* dest = buff;
        buff[0] = 0;
        do {
            if ( /**src == '\r' || */*src == '\n') {
                *dest++ = *src;
                *dest = 0;
                AddToOutList(buff, wndCode);
                dest = buff;
            } else 
                if ( *src == 0 ) {
                    *dest = 0;
                    if ( buff[0] ) {
                        AddToOutList(buff, wndCode);
                    }
                }
                else 
                    if (*src != '\r') 
                        *dest++ = *src;
        } while (*src++ ) ;

        if ( pMainWnd )
			pMainWnd->PostMessage (WM_COMMAND, wndCode ? ID_OUTPUT_TEXT_ADDED :ID_DRAW_TEXT_ADDED, 0 );

        pSec->Unlock();
    }
}

static void __stdcall ClearContents(int wndCode) 
{

    if ( pDoc ) {
        CCriticalSection* pSec;
        if ( wndCode )
            pSec = &(pDoc->m_OutputUpdateSection[wndCode > MAX_OUTPUT ? 0 : wndCode-1]);
        else 
            pSec = &pDoc->m_UpdateSection ;

        pSec->Lock();
        
		CStringList* pList;
		int * pCount ;
		BOOL * pClearContents;
		if ( wndCode) {
			int n = wndCode > MAX_OUTPUT ? 0 : wndCode-1;
			pList = &(pDoc->m_strOutputTempList[n]);
			pCount = &(pDoc->m_nOutputUpdateCount[n]);
			pClearContents = &(pDoc->m_bClearOutputContents[n]);
		}  else {
			pList = &pDoc->m_strTempList;
			pCount = &pDoc->m_nUpdateCount ;
			pClearContents = &pDoc->m_bClearContents;
		}
		
		pList->RemoveAll();
		pList->AddTail("");

		*pCount = 0;
		*pClearContents = TRUE;

		if ( pMainWnd )
			pMainWnd->PostMessage (WM_COMMAND, wndCode ? ID_OUTPUT_TEXT_ADDED :ID_DRAW_TEXT_ADDED, 0 );

        pSec->Unlock();
    }
}



unsigned long __stdcall ClientThread(void * pParam)
{
    CoInitialize (NULL);
    InitState(OutTextFrom, ClearContents, AfxGetMainWnd()->GetSafeHwnd());
    while (1) {
        DWORD dwWait = WaitForSingleObject(hInputDoneEvent, 30 );
        if ( bExit ) {
            CloseState();
            SetEvent(hStateClosedEvent);
            return 0;
        }

        if ( bReloadScripts ) {
            ReloadScriptsSection.Lock();
            // Do reload here 
            ReloadScriptEngine(strScriptText, theApp.m_guidScriptLang, pDoc->m_strProfileName);
            strScriptText.Empty ();
            bReloadScripts = FALSE;
            ReloadScriptsSection.Unlock();

        }

        if ( dwWait == WAIT_OBJECT_0 ) {// got input 
            InputSection.Lock();
            CString str = (LPCSTR)strInput;
            strInput = "";
            InputSection.Unlock();
            // TRACE("Inpuit = %s\n",(LPCSTR)str);
            CompileInput((LPSTR)(LPCSTR)str);
        }


        if ( pDoc->m_ParseDlg.m_bDoParseScriptlet ) {
            ParseScript((LPCSTR)pDoc->m_ParseDlg.m_strText);
            pDoc->m_ParseDlg.m_bDoParseScriptlet = FALSE;
            // SetEvent(pDoc->m_ParseDlg.m_hParseDoneEvent);
        }

        if ( bLunchDebuger ) {
            LunchDebuger();
            bLunchDebuger = FALSE;
        }

        ReadMud();
    }
}

IMPLEMENT_DYNCREATE(CSmcDoc, CDocument)

BEGIN_MESSAGE_MAP(CSmcDoc, CDocument)
	//{{AFX_MSG_MAP(CSmcDoc)
    ON_COMMAND(ID_SCRIPTING_ADDSCRIPTLET, OnParseScript)
	ON_UPDATE_COMMAND_UI(ID_SCRIPTING_ADDSCRIPTLET, OnUpdateParseScript)
	ON_COMMAND(ID_OPTIONS_FONT, OnOptionsFont)
	ON_COMMAND(ID_SCRIPTING_RELOADSCRIPT, OnScriptingReload)
	ON_COMMAND(ID_OPTIONS_COLORS, OnOptionsColors)
    ON_COMMAND(ID_DRAW_TEXT_ADDED, OnDrawTextAdded)
    ON_COMMAND(ID_OUTPUT_TEXT_ADDED, OnOutputTextAdded)
	ON_COMMAND(ID_PAUSE, OnPause)
	ON_UPDATE_COMMAND_UI(ID_PAUSE, OnUpdatePause)
	ON_COMMAND(ID_OPTIONS_SCROLLBUFFER, OnOptionsScrollbuffer)
	ON_COMMAND(ID_OPTIONS_KEYWORDS, OnOptionsKeywords)
	ON_COMMAND(ID_SCRIPTING_LUNCHDEBUGER, OnScriptingLunchdebuger)
	ON_UPDATE_COMMAND_UI(ID_SCRIPTING_LUNCHDEBUGER, OnUpdateScriptingLunchdebuger)
	ON_COMMAND(ID_VIEW_MUDEMULATOR, OnViewMudemulator)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MUDEMULATOR, OnUpdateViewMudemulator)
	ON_COMMAND(ID_SHOW_OUTPUT, OnShowOutput)
	//}}AFX_MSG_MAP
    ON_COMMAND(ID_SCRIPTING_BREAK, OnBreakScript)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSmcDoc construction/destruction

CSmcDoc::CSmcDoc() : m_ParseDlg(AfxGetMainWnd() ), m_MudEmulator(AfxGetMainWnd() )
{
    m_nWindowCharsSize = 1;
//vls-begin// multiple output
//    m_nOutWindowCharsSize = 1;
//    m_nUpdateCount = 0;
//    m_nOutputUpdateCount = 0;
    m_nUpdateCount = 0;
	m_bClearContents = FALSE;
    for (int i = 0; i < MAX_OUTPUT; i++) {
        m_nOutputWindowCharsSize[i] = 1;
        m_nOutputUpdateCount[i] = 0;
		m_bClearOutputContents[i] = FALSE;
    }
//vls-end//

    pStrLastESC = 0x1B;
    pStrLastESC += "[0m";

    m_bFrozen = FALSE;

    hInputDoneEvent = CreateEvent(
        NULL,    // no security attributes
        FALSE,    // manual reset event
        FALSE,   // not-signalled
        NULL);   // no name

    hStateClosedEvent = CreateEvent(
        NULL,    // no security attributes
        FALSE,    // manual reset event
        FALSE,   // not-signalled
        NULL);   // no name

	
    // TODO: add one-time construction code here
    memcpy ( m_ForeColors  , DefColors , sizeof(DefColors) );
    memcpy ( m_BackColors  , DefColors , sizeof(DefColors) );



    // Init default command char 
    bConnectBeep = ::GetPrivateProfileInt("Options" , "ConnectBeep" , FALSE , szGLOBAL_PROFILE);
    bAutoReconnect = ::GetPrivateProfileInt("Options" , "AutoReconnect" , FALSE , szGLOBAL_PROFILE);
    cCommandChar = m_cCommandChar = (char)::GetPrivateProfileInt("Options" , "CommandChar" , '#' , szGLOBAL_PROFILE);

    m_nScrollSize = ::GetPrivateProfileInt("Options" , "Scroll" , 300, szGLOBAL_PROFILE);
    m_bSplitOnBackscroll = ::GetPrivateProfileInt("Options" , "SplitOnBackscroll" , 1, szGLOBAL_PROFILE);

    nScripterrorOutput  = ::GetPrivateProfileInt("Script" , "ErrOutput", 0 , szGLOBAL_PROFILE);

    UINT  nSize;
    LPBYTE pData;
	// Font initialization
	memset ( &m_lfText, 0 , sizeof(m_lfText) );
    if ( !::GetPrivateProfileBinary ("Font" , "LOGFONT" ,&pData, &nSize, szGLOBAL_PROFILE) ) {
	    m_lfText.lfHeight = -13;
	    m_lfText.lfWeight = FW_NORMAL;
	    m_lfText.lfCharSet = ANSI_CHARSET;
	    m_lfText.lfPitchAndFamily = FIXED_PITCH;
	    strcpy(m_lfText.lfFaceName, "Fixedsys");
    }
    else {
        ASSERT(nSize == sizeof(m_lfText));
        memcpy( &m_lfText, pData , nSize);
        delete pData;
    }
	m_fntText.CreateFontIndirect(&m_lfText);


    RecalcCharSize();

    // Load color settings 
    if ( ::GetPrivateProfileBinary ("Colors" , "Foreground" ,&pData, &nSize, szGLOBAL_PROFILE) ) {
        ASSERT(nSize==sizeof(m_ForeColors));
        memcpy(m_ForeColors, pData, nSize);
        delete pData;
        pData = NULL;
    }
    if ( ::GetPrivateProfileBinary ("Colors" , "Background" ,&pData, &nSize, szGLOBAL_PROFILE) ) {
        ASSERT(nSize==sizeof(m_BackColors));
        memcpy(m_BackColors, pData, nSize);
        delete pData;
        pData = NULL;
    }
    m_bDarkOnly = ::GetPrivateProfileInt("Colors" , "DarkOnly" ,0 , szGLOBAL_PROFILE) ;

    bAllowDebug = ::GetPrivateProfileInt("Script" , "AllowDebug" ,0 , szGLOBAL_PROFILE) ;

    pDoc = this;

    // Load keywords list 
//vls-begin// base dir
//    HANDLE hFile  = CreateFile("tabwords.txt", GENERIC_READ, 0, NULL, OPEN_EXISTING, NULL, NULL );
    CString strTabwordsFile = szBASE_DIR;
    strTabwordsFile += "\\tabwords.txt";
    HANDLE hFile  = CreateFile(strTabwordsFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, NULL, NULL );
//vls-end//
    if ( hFile!=INVALID_HANDLE_VALUE ) { 
//vls-begin// bugfix
//        DWORD dwSize;
//        dwSize = GetFileSize(hFile, &dwSize);
        DWORD dwSize = GetFileSize(hFile, NULL);
//vls-end//
        char* buff = new char[dwSize+2];
        buff[dwSize] = 0;
        ReadFile(hFile, buff, dwSize, &dwSize, NULL);
        CloseHandle(hFile);
        FillTabWords(buff);
        delete buff;
    } 
}

CSmcDoc::~CSmcDoc()
{
    ::WritePrivateProfileInt("Options" , "Scroll" , m_nScrollSize, szGLOBAL_PROFILE);
    ::WritePrivateProfileInt("Options" , "CommandChar" , cCommandChar, szGLOBAL_PROFILE);
    ::WritePrivateProfileInt("Options" , "ConnectBeep" , bConnectBeep , szGLOBAL_PROFILE);
    ::WritePrivateProfileInt("Options" , "AutoReconnect" , bAutoReconnect , szGLOBAL_PROFILE);
    ::WritePrivateProfileInt("Options" , "SplitOnBackscroll" , m_bSplitOnBackscroll, szGLOBAL_PROFILE);
    ::WritePrivateProfileInt("Script" , "AllowDebug" ,bAllowDebug  , szGLOBAL_PROFILE) ;

    ::WritePrivateProfileInt("Script" , "ErrOutput", nScripterrorOutput , szGLOBAL_PROFILE);

    bExit = TRUE;
    SetEvent(hInputDoneEvent);
    WaitForSingleObject ( hStateClosedEvent, INFINITE);
    CloseHandle(hInputDoneEvent);
    CloseHandle(hStateClosedEvent);

	CString strTabwordsFile = szBASE_DIR;
	strTabwordsFile += "\\tabwords.txt";

	if (m_lstTabWords.IsEmpty()) {
		// delete keywords empty file
		DeleteFile((LPCSTR)strTabwordsFile);
	} else {
		//save tabwords
		CString strWords;
		POSITION pos = m_lstTabWords.GetHeadPosition ();
		while ( pos ) {
			strWords += m_lstTabWords.GetNext (pos) + "\r\n";
		}

		HANDLE hFile;
		//vls-begin// base dir
		//    hFile = CreateFile("tabwords.txt", GENERIC_READ| GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, NULL, NULL );
		hFile = CreateFile(strTabwordsFile, GENERIC_READ| GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, NULL, NULL );
		//vls-end//
		if ( hFile!=INVALID_HANDLE_VALUE ) { 
			DWORD Written;
			WriteFile(hFile , (LPCSTR)strWords, strWords.GetLength() , &Written, NULL );
			CloseHandle(hFile);
		}
	}
}

BOOL CSmcDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
    
    CSmcApp* pApp = (CSmcApp*)AfxGetApp();
    
    m_strProfileName = pApp->m_strCurrentProfile;
    SetTitle(m_strProfileName);

    if ( pApp->m_pszProfileName )
        free((void*)pApp->m_pszProfileName);
//vls-begin//
//    CString strProfileFile (".\\settings\\");
//    strProfileFile += m_strProfileName + ".opt";
    CString strProfileFile (szSETTINGS_DIR);
    strProfileFile += "\\" + m_strProfileName + ".opt";
//vls-end//
    
    pApp->m_pszProfileName = _tcsdup(strProfileFile);
    

//vls-begin// base dir
//    m_strDefSetFile = AfxGetApp()->GetProfileString("Options" , "AutoLoadFile" , m_strProfileName + ".set");
//    m_strDefSaveFile = AfxGetApp()->GetProfileString("Options" , "AutoSaveFile" , m_strDefSetFile);
    char *p = m_strDefSetFile.GetBuffer(MAX_PATH+2);
    MakeAbsolutePath(p, AfxGetApp()->GetProfileString("Options" , "AutoLoadFile" , m_strProfileName + ".set"), szBASE_DIR);
    m_strDefSetFile.ReleaseBuffer();

    p = m_strDefSaveFile.GetBuffer(MAX_PATH+2);
    MakeAbsolutePath(p, AfxGetApp()->GetProfileString("Options" , "AutoSaveFile" , m_strDefSetFile), szBASE_DIR);
    m_strDefSaveFile.ReleaseBuffer();
//vls-end//
	m_strSaveCommand = AfxGetApp()->GetProfileString("Options" , "AutoSaveCommand" , "");
	CString Delimiter = AfxGetApp()->GetProfileString("Options" , "CommandDelimiter" , ";");
	cCommandDelimiter = Delimiter[0];


    CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
    ASSERT_KINDOF(CMainFrame , pFrm);

    pFrm->m_editBar.m_nCursorPosWhileListing = AfxGetApp()->GetProfileInt("Main" , "CursorWileList" , 1);
    pFrm->m_editBar.m_nMinStrLen = AfxGetApp()->GetProfileInt("Main", "MinStrLen" , 2);

    // Load ANSi settings 
	bRMASupport = AfxGetApp()->GetProfileInt("ANSI" , "RMAsupport" , 0);
	bAppendLogTitle = AfxGetApp()->GetProfileInt("ANSI" , "AppendLogTitle" , 0);
	bANSILog = AfxGetApp()->GetProfileInt("ANSI" , "ANSILog" , 0);
	bDefaultLogMode = AfxGetApp()->GetProfileInt("ANSI" , "AppendMode" , 0);
	bHTML = AfxGetApp()->GetProfileInt("ANSI" , "HTMLLog" , 0);
	bHTMLTimestamps = AfxGetApp()->GetProfileInt("ANSI" , "HTMLLogTimestamps" , 0);
	bLogAsUserSeen = AfxGetApp()->GetProfileInt("ANSI" , "LogAsUserSeen" , 0);

	bIACSendSingle = AfxGetApp()->GetProfileInt("Substitution" , "IACSendSingle" , 0);
	bIACReciveSingle = AfxGetApp()->GetProfileInt("Substitution" , "IACReciveSingle" , 0);


    // Chars substitutions restore
    UINT  nSize;
    LPBYTE pData;
    bSubstitution = AfxGetApp()->GetProfileInt("Options" , "bSubstitution" , 0);
    if ( AfxGetApp()->GetProfileBinary( "Options" , "charsSubstitution" , &pData, &nSize) ) {
        memcpy(substChars, pData , nSize);
        delete pData;
    }
    // End substitutions restore

    InputSection.Lock();
    strInput += cCommandDelimiter ;
    strInput += cCommandChar;
	strInput += "read ";
//vls-begin// base dir
//    strInput += pDoc->m_strDefSetFile;
    strInput += "{" + m_strDefSetFile + "}";
//vls-end//
    InputSection.Unlock();

//vls-begin// script files
    OnScriptingReload();
//vls-end//

    SetEvent(hInputDoneEvent);

    SetModifiedFlag(FALSE);

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CSmcDoc serialization

void CSmcDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSmcDoc diagnostics

#ifdef _DEBUG
void CSmcDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CSmcDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSmcDoc commands

BOOL CSmcDoc::DoProfileSave()
{
	// Do save/macro
	InputSection.Lock();
	if ( m_strSaveCommand.GetLength() ) {
        strInput += cCommandDelimiter ;
		strInput += m_strSaveCommand;
	}
	
	if ( m_strDefSaveFile.GetLength() ) {
		strInput += cCommandDelimiter;
		strInput += cCommandChar;
		strInput += "write ";
//vls-begin// 
//        strInput += m_strDefSaveFile;
        strInput += "{" + m_strDefSaveFile + "}";
//vls-end//
	}
	// Save hotkeys, settings, macro
    // KillAll(0, 0 ); 
	InputSection.Unlock();
	SetEvent(hInputDoneEvent);

	// Lets save all 

    // Start characters substitutions save
    AfxGetApp()->WriteProfileInt("Options" , "bSubstitution" , bSubstitution);
    AfxGetApp()->WriteProfileBinary( "Options" , "charsSubstitution" , (LPBYTE)substChars, SUBST_ARRAY_SIZE);
    // End characters substitutions save

	 AfxGetApp()->WriteProfileInt("Substitution" , "IACSendSingle" , bIACSendSingle);
	 AfxGetApp()->WriteProfileInt("Substitution" , "IACReciveSingle" , bIACReciveSingle);

//vls-begin// base dir
//    AfxGetApp()->WriteProfileString("Options" , "AutoLoadFile" , m_strDefSetFile);
//	AfxGetApp()->WriteProfileString("Options" , "AutoSaveFile" , m_strDefSaveFile);
    char loc[MAX_PATH+2];
    MakeLocalPath(loc, m_strDefSetFile, szBASE_DIR);
    AfxGetApp()->WriteProfileString("Options" , "AutoLoadFile" , loc);
    MakeLocalPath(loc, m_strDefSaveFile, szBASE_DIR);
    AfxGetApp()->WriteProfileString("Options" , "AutoSaveFile" , loc);
//vls-end//
	AfxGetApp()->WriteProfileString("Options" , "AutoSaveCommand" , m_strSaveCommand);

	CString Delimiter(cCommandDelimiter);
	AfxGetApp()->WriteProfileString("Options" , "CommandDelimiter" , Delimiter);

    // Save editbar settings
    CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
    ASSERT_KINDOF(CMainFrame , pFrm);
    AfxGetApp()->WriteProfileInt("Main" , "CursorWileList" , pFrm->m_editBar.m_nCursorPosWhileListing);
    AfxGetApp()->WriteProfileInt("Main", "MinStrLen" , pFrm->m_editBar.m_nMinStrLen);

    // Save ANSI settings
	AfxGetApp()->WriteProfileInt("ANSI" , "RMAsupport" , bRMASupport);
	AfxGetApp()->WriteProfileInt("ANSI" , "AppendLogTitle" , bAppendLogTitle);
	AfxGetApp()->WriteProfileInt("ANSI" , "ANSILog" , bANSILog);
	AfxGetApp()->WriteProfileInt("ANSI" , "AppendMode" , bDefaultLogMode);
	AfxGetApp()->WriteProfileInt("ANSI" , "HTMLLog" , bHTML);
	AfxGetApp()->WriteProfileInt("ANSI" , "HTMLLogTimestamps" , bHTMLTimestamps);
	AfxGetApp()->WriteProfileInt("ANSI" , "LogAsUserSeen" , bLogAsUserSeen);


    ::WritePrivateProfileBinary("Colors" , "Foreground", (LPBYTE)m_ForeColors, sizeof(m_ForeColors), szGLOBAL_PROFILE);
    ::WritePrivateProfileBinary("Colors" , "Background", (LPBYTE)m_BackColors, sizeof(m_BackColors), szGLOBAL_PROFILE);
    ::WritePrivateProfileInt("Colors" , "DarkOnly" ,m_bDarkOnly , szGLOBAL_PROFILE);


    // Save scripts here 
    // saves all scriptlets here in one file 
//vls-begin// script files
//    if ( m_ParseDlg.m_lstScriptlets.GetCount () > 2) {
    int size;
    GetScriptFileList(&size);
    if ( m_ParseDlg.m_lstScriptlets.GetCount () > 2 + size) {
//vls-end//
        CString strAllScripts;
        for ( POSITION pos = m_ParseDlg.m_lstScriptlets.GetHeadPosition () ; pos ; ) {
            CString strScriptlet = m_ParseDlg.m_lstScriptlets.GetNext(pos);
            strAllScripts += strScriptlet;
            strAllScripts += "\r\n";
        }
        // now save it to file ".\\settings\\union.scr"
        HANDLE hFile;
//vls-begin// base dir
//        hFile = CreateFile(".\\settings\\union.scr", GENERIC_READ| GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, NULL, NULL );
        CString strFile = szSETTINGS_DIR;
        strFile += "\\union.scr";
        hFile = CreateFile(strFile, GENERIC_READ| GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, NULL, NULL );
//vls-end//
        if ( hFile!=INVALID_HANDLE_VALUE ) { 
            DWORD Written;
            WriteFile(hFile , (LPCSTR)strAllScripts, strAllScripts.GetLength() , &Written, NULL );
            CloseHandle(hFile);
        }
    }


   
	return TRUE;
}


BOOL CSmcDoc::CanCloseFrame(CFrameWnd* pFrame) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	// return CDocument::CanCloseFrame(pFrame);
	DoProfileSave();
	return TRUE;
}

void CSmcDoc::DeleteContents() 
{
	// Clear interpretator
	InputSection.Lock();
//* en
//	strInput += cCommandDelimiter;
//	strInput += cCommandChar;
	strInput = cCommandChar;
//* /en
	strInput += "killall";
	InputSection.Unlock();
	SetEvent(hInputDoneEvent);

	CDocument::DeleteContents();
}

void CSmcDoc::RecalcCharSize()
{
    CDC dc;
    dc.CreateDC("display" , NULL , NULL, NULL);
    CFont* pOldFont = dc.SelectObject(&m_fntText);

    CSize size = dc.GetTextExtent("A");
    
    dc.SelectObject(pOldFont);

    dc.DeleteDC();

	m_nYsize = size.cy;
    m_nCharX = size.cx;
}

void CSmcDoc::OnOptionsFont() 
{
	CFontDialog fd(&m_lfText,  CF_FIXEDPITCHONLY | CF_NOSCRIPTSEL | CF_SCREENFONTS , NULL , AfxGetMainWnd() );
	if ( fd.DoModal() == IDOK ) {
		m_fntText.DeleteObject();
		m_fntText.CreateFontIndirect(&m_lfText);
        ::WritePrivateProfileBinary("Font" , "LOGFONT" ,(LPBYTE)&m_lfText, sizeof(m_lfText), szGLOBAL_PROFILE);

        RecalcCharSize();
        ((CMainFrame*)AfxGetMainWnd())->m_editBar.GetDlgItem(IDC_EDIT)->SetFont(&m_fntText);

        POSITION pos = GetFirstViewPosition();
        while ( pos) {
            CView* pView = GetNextView(pos);
            CRect rect;
            pView->GetClientRect(&rect);
            pView->SendMessage(WM_SIZE, 0, MAKELPARAM(rect.Width(), rect.Height()) );
            pView->InvalidateRect(NULL, FALSE);
            pView->UpdateWindow();
        }
    }
}


void CSmcDoc::OnOptionsColors() 
{
	CAnsiColorsDlg dlg(AfxGetMainWnd());
    
    int i;
    for ( i = 0 ; i < 16 ; i++ ) {
        dlg.m_Btn[i].SetColor(m_ForeColors[i]);
    }
    dlg.m_bDarkOnly = m_bDarkOnly;
    if ( dlg.DoModal() == IDOK ) {

        for ( i = 0 ; i < 16 ; i++ ) {
            m_ForeColors[i] = m_BackColors[i] = dlg.m_Btn[i].GetColor();
        }

         m_bDarkOnly = dlg.m_bDarkOnly;
        
        POSITION pos = GetFirstViewPosition();
        while ( pos) {
            CView* pView = GetNextView(pos);
            pView->InvalidateRect(NULL, FALSE);
            pView->UpdateWindow();
        }
    }
}

void CSmcDoc::OnDrawTextAdded()
{
    if ( m_bFrozen ) 
        return;
    m_UpdateSection.Lock();
    UpdateAllViews(NULL, TEXT_ARRIVED, NULL );
    m_nUpdateCount = 0;
	m_bClearContents = FALSE;
    CString str = m_strTempList.GetTail();
    m_strTempList.RemoveAll ();
    m_strTempList.AddTail(str);
    m_UpdateSection.Unlock();
}

//vls-begin// multiple output
//void CSmcDoc::OnOutputTextAdded()
//{
//    if ( m_bFrozen ) 
//        return;
//    m_UpdateOutputSection.Lock();
//    pMainWnd->m_coolBar.m_wndAnsi.OnUpdate(TEXT_ARRIVED);
//    m_nOutputUpdateCount =0;
//    CString str = m_strOutoputTempList.GetTail();
//    m_strOutoputTempList.RemoveAll ();
//    m_strOutoputTempList.AddTail(str);
//    m_UpdateOutputSection.Unlock();
//}
void CSmcDoc::OnOutputTextAdded()
{
    if ( m_bFrozen ) 
        return;

    for (int i = 0; i < MAX_OUTPUT; i++) {
        if (m_strOutputTempList[i].GetCount() > 0) {
            m_OutputUpdateSection[i].Lock();
            pMainWnd->m_coolBar[i].m_wndAnsi.OnUpdate(TEXT_ARRIVED);
            m_nOutputUpdateCount[i] = 0;
			m_bClearOutputContents[i] = FALSE;
            CString str = m_strOutputTempList[i].GetTail();
            m_strOutputTempList[i].RemoveAll();
            m_strOutputTempList[i].AddTail(str);
            m_OutputUpdateSection[i].Unlock();
        }
    }
}
//vls-end//

void CSmcDoc::OnPause() 
{
    m_bFrozen = !m_bFrozen;
    if ( !m_bFrozen ) 
        OnDrawTextAdded();
}

void CSmcDoc::OnUpdatePause(CCmdUI* pCmdUI) 
{
	if ( m_bFrozen ) 
        pCmdUI->SetCheck(1);
    else 
        pCmdUI->SetCheck(0);
	
}

void CSmcDoc::DrawSome(LPSTR str)
{
    OutTextFrom(str, 0);
}



void CSmcDoc::OnOptionsScrollbuffer() 
{
	CScrollOptionDlg dlg(AfxGetMainWnd());

    dlg.m_nCount = pDoc->m_nScrollSize;
    if ( dlg.DoModal() == IDOK ) {
        m_nScrollSize = dlg.m_nCount;

        UpdateAllViews(NULL, SCROLL_SIZE_CHANGED, NULL );
//vls-begin// multiple output
//        pMainWnd->m_coolBar.m_wndAnsi.OnUpdate(SCROLL_SIZE_CHANGED);
        for (int i = 0; i < MAX_OUTPUT; i++)
            pMainWnd->m_coolBar[i].m_wndAnsi.OnUpdate(SCROLL_SIZE_CHANGED);
//vls-end//

    }
}

void CSmcDoc::OnScriptingReload()
{
    // load common library 
    ReloadScriptsSection.Lock ();
    m_ParseDlg.m_lstScriptlets.RemoveAll();
    m_ParseDlg.m_nScriptletIndex = 0;

//vls-begin// base dir
//    HANDLE hFile = CreateFile(".\\settings\\commonlib.scr", GENERIC_READ, 0, NULL, OPEN_EXISTING, NULL, NULL );
    CString ScriptFileName = szSETTINGS_DIR;
    ScriptFileName += "\\commonlib.scr";
    HANDLE hFile = CreateFile(ScriptFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, NULL, NULL );
//vls-end//
    if ( hFile!=INVALID_HANDLE_VALUE ) { 
        DWORD dwSize;
//vls-begin// bugfix
//        dwSize = GetFileSize(hFile, &dwSize);
        dwSize = GetFileSize(hFile, NULL);
//vls-end//
        void* buff = strScriptText.GetBuffer ((int)dwSize);
        ReadFile(hFile, buff, dwSize, &dwSize, NULL);
        CloseHandle(hFile);
        strScriptText.ReleaseBuffer ();
        m_ParseDlg.m_lstScriptlets.AddTail (strScriptText);
    } else {
        strScriptText.Empty ();
    }
    m_ParseDlg.m_strText = strScriptText;
    
    // Load scripts text
//vls-begin// base dir
//    CString ScriptFileName;
//    ScriptFileName.Format (".\\settings\\%s.scr" , (LPCSTR)m_strProfileName);
    char p[MAX_PATH+2];
    //vls-begin// script files
    MakeAbsolutePath(p, szPROFILESCRIPT, szSETTINGS_DIR);
    ScriptFileName = p;
//vls-end//
    CString strText;

    hFile = CreateFile(ScriptFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, NULL, NULL );
    if ( hFile!=INVALID_HANDLE_VALUE ) { 
        DWORD dwSize;
//vls-begin// bugfix
//        dwSize = GetFileSize(hFile, &dwSize);
        dwSize = GetFileSize(hFile, NULL);
//vls-end//
        void* buff = strText.GetBuffer ((int)dwSize);
        ReadFile(hFile, buff, dwSize, &dwSize, NULL);
        CloseHandle(hFile);
        strText.ReleaseBuffer ();
        m_ParseDlg.m_lstScriptlets.AddTail (strText);
        strScriptText += strText;
    } 
    m_ParseDlg.m_strText = strText;

//vls-begin// script files
    int size;
    GetScriptFileList(&size);
    for (int pos = 0; pos < size; pos++) {
        PCScriptFile pScr = GetScriptFile(pos);
        char p[MAX_PATH+2];
        MakeAbsolutePath(p, pScr->m_strName.data(), szSETTINGS_DIR);
        hFile = CreateFile(p, GENERIC_READ, 0, NULL, OPEN_EXISTING, NULL, NULL );
        if ( hFile!=INVALID_HANDLE_VALUE ) { 
            DWORD dwSize = GetFileSize(hFile, NULL);
            CString strText;
            void* buff = strText.GetBuffer ((int)dwSize);
            ReadFile(hFile, buff, dwSize, &dwSize, NULL);
            CloseHandle(hFile);
            strText.ReleaseBuffer ();
            m_ParseDlg.m_lstScriptlets.AddTail (strText);
            strScriptText += strText;
        }
        m_ParseDlg.m_strText = strText;
    }
//vls-end//

    if( m_ParseDlg.m_lstScriptlets.GetCount () ) 
        m_ParseDlg.m_nScriptletIndex = m_ParseDlg.m_lstScriptlets.GetCount () ;

    if ( m_ParseDlg.m_hWnd ) {
        m_ParseDlg.m_nScriptletIndex = 0;
        m_ParseDlg.UpdateData(FALSE);
        m_ParseDlg.SetPNKeys();
    }

    bReloadScripts = TRUE;
    ReloadScriptsSection.Unlock();

}

void CSmcDoc::OnParseScript()
{
    if ( !m_ParseDlg.m_hWnd ) {
        m_ParseDlg.Create(IDD_SCRIPT_PARSE, AfxGetMainWnd() );
    } else {
        if ( m_ParseDlg.IsWindowVisible () ) 
            m_ParseDlg.ShowWindow(SW_HIDE);
        else 
            m_ParseDlg.ShowWindow(SW_SHOW);
    }
    
}

void CSmcDoc::OnUpdateParseScript(CCmdUI* pCmdUI) 
{
    if ( !m_ParseDlg.m_hWnd || !m_ParseDlg.IsWindowVisible () ) 
        pCmdUI->SetCheck (FALSE);
    else 
        pCmdUI->SetCheck (TRUE);
}

void CSmcDoc::OnBreakScript()
{
    BreakScript();
}

void CSmcDoc::FillTabWords(LPCSTR strWords)
{
    m_lstTabWords.RemoveAll ();
    unsigned char* ptr  = (UCHAR*)strWords;
    while ( *ptr ) {
        while ( *ptr && *ptr <= ' ' ) 
            ptr++;
        if ( !*ptr ) 
            break;

        unsigned char* ptrWord = ptr++;
        while ( *ptr && *ptr > ' ' ) 
            ptr++;

        int size = ptr-ptrWord;
        char* buff = new char[size+1];
        strncpy((char*)buff, (LPCSTR)ptrWord, size);
        buff[size] = 0;
        m_lstTabWords.AddTail(buff);
        delete buff;
    }
}

void CSmcDoc::OnOptionsKeywords() 
{
	CKeyWordsDlg dlg(AfxGetMainWnd() );

    POSITION pos = m_lstTabWords.GetHeadPosition ();
    while ( pos ) {
        dlg.m_strKeys += m_lstTabWords.GetNext (pos) + "\r\n";
    }

    if ( dlg.DoModal() ) {
        FillTabWords(dlg.m_strKeys);
    }
}

void CSmcDoc::OnScriptingLunchdebuger() 
{
	bLunchDebuger = TRUE;
    SetEvent(hInputDoneEvent);
	
}

void CSmcDoc::OnUpdateScriptingLunchdebuger(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (bAllowDebug);
}


void CSmcDoc::OnViewMudemulator() 
{
    if ( !m_MudEmulator.m_hWnd ) {
        m_MudEmulator.Create(IDD_TESTTEXT, AfxGetMainWnd() );
        m_MudEmulator.ShowWindow(SW_SHOW);
    } else {
        if ( m_MudEmulator.IsWindowVisible () ) 
            m_MudEmulator.ShowWindow(SW_HIDE);
        else 
            m_MudEmulator.ShowWindow(SW_SHOW);
    }
    
}

void CSmcDoc::OnUpdateViewMudemulator(CCmdUI* pCmdUI) 
{
    if ( !m_MudEmulator.m_hWnd || !m_MudEmulator.IsWindowVisible () ) 
        pCmdUI->SetCheck (FALSE);
    else 
        pCmdUI->SetCheck (TRUE);
}

void CSmcDoc::OnShowOutput() 
{
	// TODO: Add your command handler code here
	
}
