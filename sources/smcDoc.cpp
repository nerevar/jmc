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

static void ParseAnsiValues (const wchar_t* AnsiStr, CString* pLastESC)
{
    const wchar_t* src = AnsiStr;
    wchar_t Num[128] ;

    do {
        wchar_t* dest = Num;
        while (iswdigit(*src) ) {
            *dest++ = *src++;
        }
        *dest = 0;
        if ( Num[0] ) {
            int Value = _wtoi(Num);
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

    pLastESC->Format(L"%c[%d;%d;%dm" , L'\x1B', LastParsedBold, LastParsedFg, LastParsedBg );
}

static void AddToOutList(const wchar_t* str, int wndCode)
{
    wchar_t  strAdd[BUFFER_SIZE+32];

    CStringList* pList;
    CString* pLastEsc;
    int * pCount ;
    int CharSize ;
    if (wndCode) {
        wndCode = wndCode > MAX_OUTPUT ? 0 : wndCode-1;
        pList = &(pDoc->m_strOutputTempList[wndCode]);
        pLastEsc = &pStrOutputLastESC[wndCode];
        pCount = &(pDoc->m_nOutputUpdateCount[wndCode]);
        CharSize = pDoc->m_nOutputWindowCharsSize[wndCode];
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

	int len, maxlength = sizeof(strAdd)/sizeof(wchar_t) - 1;

	bool set_tail = false;
	bool user_input = false,
		 jmc_message = false;

	if (str[0] == TINTIN_OUTPUT_MARK) {
		jmc_message = true;
		str++;
		if (str[0] == USER_INPUT_MARK) {
			user_input = true;
			str++;
		}
	}

    int TailLen = strTail.GetLength();
	bool uncomplete_line = TailLen && (strTail[TailLen-1] != L'\n') && (strTail[TailLen-1] != END_OF_PROMPT_MARK);
	bool prompt_line = TailLen && (strTail[TailLen-1] == END_OF_PROMPT_MARK);
	bool copy_tail = false;

	if (uncomplete_line) {
		if (user_input) {
			//????
			set_tail = true;
		} else if (jmc_message) {
			pList->InsertBefore(pList->GetTailPosition(), str);
			*pCount = *pCount + 1;
			return;
		} else {
			set_tail = true;
		}
	} else if (prompt_line) {
		TailLen--; //suppress END_OF_PROMPT_MARK char
		if (user_input && !bInputOnNewLine) {
			set_tail = true;
			copy_tail = true;
		} else {
			strTail.Replace(END_OF_PROMPT_MARK, L'\n');
			set_tail = false;
		}
	}

    if (copy_tail) {
		len = TailLen;
		if ( len > maxlength )
			len = maxlength;
		wcsncpy(strAdd , strTail, len);
		strAdd[len] = '\0';
		maxlength -= len;
    } else {
		len = wcslen(*pLastEsc);
		if ( len > maxlength )
			len = maxlength;
		wcsncpy(strAdd , *pLastEsc, len);
		strAdd[len] = L'\0';
		maxlength -= len;
	}
	
	len = wcslen(str);
	if ( len > maxlength )
		len = maxlength;
    wcsncat(strAdd, str, len);
	maxlength -= len;

    // Now we are ready to parse string, split it to few strings to fit to window etc.
    // Lets start
    wchar_t AllANSIFromCurrString[BUFFER_SIZE];
    wchar_t* src = strAdd;
    wchar_t* ansi = AllANSIFromCurrString;
    wchar_t OutputBuffer[BUFFER_SIZE+32];
    wchar_t* dest = OutputBuffer;
    int OutTextLen = 0;

    do {
        switch ( *src ) {
        case L'\x1B':
            // Now skip ansi and save it to AllANSIFromCurrString
            do {
				*ansi++ = *src;
                *dest++ = *src++;
            } while ( *src && *src != L'm' ) ;
			*ansi++ = *src;
			*ansi = 0;
            *dest++ = *src;
            // Ansi skipped
            if ( *src ) 
                break;
        case 0:
            // End of line - time to do all save operations 
            *dest = 0;
			*ansi = 0;
            // Buffer filled , now parse ansi and save values
			if (src > strAdd && !user_input)
				ParseAnsiValues(AllANSIFromCurrString, pLastEsc);
            if (set_tail) {
                pList->SetAt(pList->GetTailPosition(), OutputBuffer);
                set_tail = false;
				if (*pCount == 0)
					*pCount = 1;
            } else {
                pList->AddTail(OutputBuffer);
                *pCount = *pCount + 1;
            }
            break;
        default:
            // Copy character to OutputBuffer and watch for m_nWindowCharsSize 
            *dest++ = *src;
            OutTextLen++;
            break;
        };
    } while (*src++ ) ;
}

static void __stdcall OutTextFrom(const wchar_t* str, int wndCode) 
{
    if ( pDoc ) {
        CCriticalSection* pSec;
        if ( wndCode )
//vls-begin// multiple output
//            pSec = &pDoc->m_UpdateOutputSection;
			pSec = &(pDoc->m_OutputUpdateSection[wndCode > MAX_OUTPUT ? 0 : wndCode-1]);
			//pSec = &(pDoc->m_OutputUpdateSection[wndCode-1]);
        else 
            pSec = &pDoc->m_UpdateSection ;

        pSec->Lock();
        // now i have to split buffer to few strings
        const wchar_t* src = str;
        wchar_t buff[BUFFER_SIZE+32];
        wchar_t* dest = buff;
		int maxlength = sizeof(buff)/sizeof(wchar_t);
        buff[0] = 0;
        do {
            if ( *src == L'\n' ) {
				if ( maxlength > 1 ) {
					*dest++ = *src;
					--maxlength;
				}
                *dest = 0;
                AddToOutList(buff, wndCode);
                dest = buff;
				maxlength = sizeof(buff)/sizeof(wchar_t);
            } else if ( *src == 0 ) {
				*dest = 0;
                if ( buff[0] )
					AddToOutList(buff, wndCode);
            } else if (maxlength > 1) {
				*dest++ = *src;
				--maxlength;
			}
        } while (*src++ ) ;

        if ( pMainWnd && !wndCode) 
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
            CString str = strInput;
            strInput = L"";
            InputSection.Unlock();
            // TRACE("Inpuit = %s\n",(LPCSTR)str);
            CompileInput(str);
        }


        if ( pDoc->m_ParseDlg.m_bDoParseScriptlet ) {
            ParseScript(pDoc->m_ParseDlg.m_strText);
            pDoc->m_ParseDlg.m_bDoParseScriptlet = FALSE;
            // SetEvent(pDoc->m_ParseDlg.m_hParseDoneEvent);
        }

        if ( bLunchDebuger ) {
            LunchDebuger();
            bLunchDebuger = FALSE;
        }

		if ( pMainWnd ) {
			pMainWnd->PostMessage (WM_COMMAND, ID_OUTPUT_TEXT_ADDED, 0);
			pMainWnd->PostMessage (WM_COMMAND, ID_DRAW_TEXT_ADDED, 0);
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

    pStrLastESC = L"\x1B";
    pStrLastESC += L"[0m";

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
    bConnectBeep = ::GetPrivateProfileInt(L"Options" , L"ConnectBeep" , FALSE , szGLOBAL_PROFILE);
    bAutoReconnect = ::GetPrivateProfileInt(L"Options" , L"AutoReconnect" , FALSE , szGLOBAL_PROFILE);
    cCommandChar = m_cCommandChar = (wchar_t)::GetPrivateProfileInt(L"Options" , L"CommandChar" , L'#' , szGLOBAL_PROFILE);

	wBCastUdpPort = ::GetPrivateProfileInt(L"Options" , L"BroadCastUdpPort" , 8326 , szGLOBAL_PROFILE);
	bBCastFilterIP = ::GetPrivateProfileInt(L"Options" , L"BroadCastFilterIP" , TRUE , szGLOBAL_PROFILE);
	bBCastFilterPort = ::GetPrivateProfileInt(L"Options" , L"BroadCastFilterPort" , TRUE , szGLOBAL_PROFILE);
	reopen_bcast_socket();

    m_bSplitOnBackscroll = ::GetPrivateProfileInt(L"Options" , L"SplitOnBackscroll" , 1, szGLOBAL_PROFILE);

	m_bRectangleSelection = ::GetPrivateProfileInt(L"Options" , L"RectangleSelection" , 0, szGLOBAL_PROFILE);
	m_bRemoveESCSelection = ::GetPrivateProfileInt(L"Options" , L"RemoveESCSelection" , 1, szGLOBAL_PROFILE);
	m_bLineWrap = ::GetPrivateProfileInt(L"Options" , L"LineWrap" , 1, szGLOBAL_PROFILE);
	m_bShowTimestamps = ::GetPrivateProfileInt(L"Options" , L"LineTimeStamps" , 0, szGLOBAL_PROFILE);
	m_bShowHiddenText = ::GetPrivateProfileInt(L"Options" , L"ShowHiddenText" , 1, szGLOBAL_PROFILE);

    nScripterrorOutput  = ::GetPrivateProfileInt(L"Script" , L"ErrOutput", 0 , szGLOBAL_PROFILE);

    UINT  nSize;
    LPBYTE pData;
	// Font initialization
	memset ( &m_lfText, 0 , sizeof(m_lfText) );
    if ( !::GetPrivateProfileBinary (L"Font" , L"LOGFONT" ,&pData, &nSize, szGLOBAL_PROFILE) ) {
	    m_lfText.lfHeight = -13;
	    m_lfText.lfWeight = FW_NORMAL;
	    m_lfText.lfCharSet = ANSI_CHARSET;
	    m_lfText.lfPitchAndFamily = FIXED_PITCH;
	    wcscpy(m_lfText.lfFaceName, L"Fixedsys");
    }
    else {
        ASSERT(nSize == sizeof(m_lfText));
        memcpy( &m_lfText, pData , nSize);
        delete pData;
    }
	m_fntText.CreateFontIndirect(&m_lfText);


    RecalcCharSize();

    // Load color settings 
    if ( ::GetPrivateProfileBinary (L"Colors" , L"Foreground" ,&pData, &nSize, szGLOBAL_PROFILE) ) {
        ASSERT(nSize==sizeof(m_ForeColors));
        memcpy(m_ForeColors, pData, nSize);
        delete pData;
        pData = NULL;
    }
    if ( ::GetPrivateProfileBinary (L"Colors" , L"Background" ,&pData, &nSize, szGLOBAL_PROFILE) ) {
        ASSERT(nSize==sizeof(m_BackColors));
        memcpy(m_BackColors, pData, nSize);
        delete pData;
        pData = NULL;
    }
    m_bDarkOnly = ::GetPrivateProfileInt(L"Colors" , L"DarkOnly" ,0 , szGLOBAL_PROFILE) ;

    bAllowDebug = ::GetPrivateProfileInt(L"Script" , L"AllowDebug" ,0 , szGLOBAL_PROFILE) ;

    pDoc = this;

    // Load keywords list 
    CString strTabwordsFile = szBASE_DIR;
    strTabwordsFile += L"\\tabwords.txt";
	int len = read_file_contents(strTabwordsFile, NULL, 0);
	if (len > 0) {
		wchar_t * buff = new wchar_t[len];
		read_file_contents(strTabwordsFile, buff, len);
		FillTabWords(buff);
		delete[] buff;
	}
	FillTabWords(NULL);
}

CSmcDoc::~CSmcDoc()
{
    ::WritePrivateProfileInt(L"Options" , L"Scroll" , nScrollSize, szGLOBAL_PROFILE);
    ::WritePrivateProfileInt(L"Options" , L"CommandChar" , cCommandChar, szGLOBAL_PROFILE);
    ::WritePrivateProfileInt(L"Options" , L"ConnectBeep" , bConnectBeep , szGLOBAL_PROFILE);
    ::WritePrivateProfileInt(L"Options" , L"AutoReconnect" , bAutoReconnect , szGLOBAL_PROFILE);
    ::WritePrivateProfileInt(L"Options" , L"SplitOnBackscroll" , m_bSplitOnBackscroll, szGLOBAL_PROFILE);
    ::WritePrivateProfileInt(L"Script" , L"AllowDebug" ,bAllowDebug  , szGLOBAL_PROFILE) ;

	::WritePrivateProfileInt(L"Options" , L"BroadCastUdpPort" , wBCastUdpPort , szGLOBAL_PROFILE);
	::WritePrivateProfileInt(L"Options" , L"BroadCastFilterIP" , bBCastFilterIP , szGLOBAL_PROFILE);
	::WritePrivateProfileInt(L"Options" , L"BroadCastFilterPort" , bBCastFilterPort , szGLOBAL_PROFILE);

	::WritePrivateProfileInt(L"Options" , L"RectangleSelection" , m_bRectangleSelection , szGLOBAL_PROFILE);
	::WritePrivateProfileInt(L"Options" , L"RemoveESCSelection" , m_bRemoveESCSelection , szGLOBAL_PROFILE);
	::WritePrivateProfileInt(L"Options" , L"LineWrap" , m_bLineWrap , szGLOBAL_PROFILE);
	::WritePrivateProfileInt(L"Options" , L"LineTimeStamps" , m_bShowTimestamps , szGLOBAL_PROFILE);
	::WritePrivateProfileInt(L"Options" , L"ShowHiddenText", m_bShowHiddenText , szGLOBAL_PROFILE);

    ::WritePrivateProfileInt(L"Script" , L"ErrOutput", nScripterrorOutput , szGLOBAL_PROFILE);

    bExit = TRUE;
    SetEvent(hInputDoneEvent);
    WaitForSingleObject ( hStateClosedEvent, INFINITE);
    CloseHandle(hInputDoneEvent);
    CloseHandle(hStateClosedEvent);

	CString strTabwordsFile = szBASE_DIR;
	strTabwordsFile += L"\\tabwords.txt";

	if (m_lstTabWords.IsEmpty()) {
		// delete keywords empty file
		DeleteFile(strTabwordsFile);
	} else {
		//save tabwords
		CString strWords;
		POSITION pos = m_lstTabWords.GetHeadPosition ();
		while ( pos ) {
			CString word = m_lstTabWords.GetNext (pos);
			if ( word[0] != cCommandChar )
				strWords += word + L"\r\n";
		}

		HANDLE hFile;
		hFile = CreateFile(strTabwordsFile, GENERIC_READ| GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, NULL, NULL );
		if ( hFile!=INVALID_HANDLE_VALUE ) { 
			DWORD Written;
			WriteFile(hFile , strWords, strWords.GetLength() , &Written, NULL );
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
    strProfileFile += L"\\" + m_strProfileName + L".opt";
//vls-end//
    
    pApp->m_pszProfileName = _tcsdup(strProfileFile);
    

//vls-begin// base dir
//    m_strDefSetFile = AfxGetApp()->GetProfileString("Options" , "AutoLoadFile" , m_strProfileName + ".set");
//    m_strDefSaveFile = AfxGetApp()->GetProfileString("Options" , "AutoSaveFile" , m_strDefSetFile);
    wchar_t *p = m_strDefSetFile.GetBuffer(MAX_PATH+2);
    MakeAbsolutePath(p, AfxGetApp()->GetProfileString(L"Options" , L"AutoLoadFile" , m_strProfileName + L".set"), szBASE_DIR);
    m_strDefSetFile.ReleaseBuffer();

    p = m_strDefSaveFile.GetBuffer(MAX_PATH+2);
    MakeAbsolutePath(p, AfxGetApp()->GetProfileString(L"Options" , L"AutoSaveFile" , m_strDefSetFile), szBASE_DIR);
    m_strDefSaveFile.ReleaseBuffer();
//vls-end//
	m_strSaveCommand = AfxGetApp()->GetProfileString(L"Options" , L"AutoSaveCommand" , L"");
	CString Delimiter = AfxGetApp()->GetProfileString(L"Options" , L"CommandDelimiter" , L";");
	cCommandDelimiter = Delimiter[0];

    // Load ANSi settings 
	bRMASupport = AfxGetApp()->GetProfileInt(L"ANSI" , L"RMAsupport" , 0);
	bAppendLogTitle = AfxGetApp()->GetProfileInt(L"ANSI" , L"AppendLogTitle" , 0);
	bANSILog = AfxGetApp()->GetProfileInt(L"ANSI" , L"ANSILog" , 0);
	bDefaultLogMode = AfxGetApp()->GetProfileInt(L"ANSI" , L"AppendMode" , 0);
	bHTML = AfxGetApp()->GetProfileInt(L"ANSI" , L"HTMLLog" , 0);
	bHTMLTimestamps = AfxGetApp()->GetProfileInt(L"ANSI" , L"HTMLLogTimestamps" , 0);
	bLogAsUserSeen = AfxGetApp()->GetProfileInt(L"ANSI" , L"LogAsUserSeen" , 0);
	LogCodePage = AfxGetApp()->GetProfileInt(L"ANSI" , L"LogCodePage" , 0);

	bIACSendSingle = AfxGetApp()->GetProfileInt(L"Substitution" , L"IACSendSingle" , 0);
	bIACReciveSingle = AfxGetApp()->GetProfileInt(L"Substitution" , L"IACReciveSingle" , 0);


    // Chars substitutions restore
    UINT  nSize;
    LPBYTE pData;
    bSubstitution = AfxGetApp()->GetProfileInt(L"Options" , L"bSubstitution" , 0);
    if ( AfxGetApp()->GetProfileBinary( L"Options" , L"charsSubstitution" , &pData, &nSize) ) {
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
    strInput += L"{" + m_strDefSetFile + L"}";
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
    AfxGetApp()->WriteProfileInt(L"Options" , L"bSubstitution" , bSubstitution);
    AfxGetApp()->WriteProfileBinary( L"Options" , L"charsSubstitution" , (LPBYTE)substChars, SUBST_ARRAY_SIZE);
    // End characters substitutions save

	 AfxGetApp()->WriteProfileInt(L"Substitution" , L"IACSendSingle" , bIACSendSingle);
	 AfxGetApp()->WriteProfileInt(L"Substitution" , L"IACReciveSingle" , bIACReciveSingle);

//vls-begin// base dir
//    AfxGetApp()->WriteProfileString("Options" , "AutoLoadFile" , m_strDefSetFile);
//	AfxGetApp()->WriteProfileString("Options" , "AutoSaveFile" , m_strDefSaveFile);
    wchar_t loc[MAX_PATH+2];
    MakeLocalPath(loc, m_strDefSetFile, szBASE_DIR);
    AfxGetApp()->WriteProfileString(L"Options" , L"AutoLoadFile" , loc);
    MakeLocalPath(loc, m_strDefSaveFile, szBASE_DIR);
    AfxGetApp()->WriteProfileString(L"Options" , L"AutoSaveFile" , loc);
//vls-end//
	AfxGetApp()->WriteProfileString(L"Options" , L"AutoSaveCommand" , m_strSaveCommand);

	CString Delimiter(cCommandDelimiter);
	AfxGetApp()->WriteProfileString(L"Options" , L"CommandDelimiter" , Delimiter);

    // Save editbar settings
    CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
    ASSERT_KINDOF(CMainFrame , pFrm);
    AfxGetApp()->WriteProfileInt(L"Main" , L"CursorWileList" , pFrm->m_editBar.m_nCursorPosWhileListing);
    AfxGetApp()->WriteProfileInt(L"Main", L"MinStrLen" , pFrm->m_editBar.m_nMinStrLen);

    // Save ANSI settings
	AfxGetApp()->WriteProfileInt(L"ANSI" , L"RMAsupport" , bRMASupport);
	AfxGetApp()->WriteProfileInt(L"ANSI" , L"AppendLogTitle" , bAppendLogTitle);
	AfxGetApp()->WriteProfileInt(L"ANSI" , L"ANSILog" , bANSILog);
	AfxGetApp()->WriteProfileInt(L"ANSI" , L"AppendMode" , bDefaultLogMode);
	AfxGetApp()->WriteProfileInt(L"ANSI" , L"HTMLLog" , bHTML);
	AfxGetApp()->WriteProfileInt(L"ANSI" , L"HTMLLogTimestamps" , bHTMLTimestamps);
	AfxGetApp()->WriteProfileInt(L"ANSI" , L"LogAsUserSeen" , bLogAsUserSeen);
	AfxGetApp()->WriteProfileInt(L"ANSI" , L"LogCodePage" , LogCodePage);

    ::WritePrivateProfileBinary(L"Colors" , L"Foreground", (LPBYTE)m_ForeColors, sizeof(m_ForeColors), szGLOBAL_PROFILE);
    ::WritePrivateProfileBinary(L"Colors" , L"Background", (LPBYTE)m_BackColors, sizeof(m_BackColors), szGLOBAL_PROFILE);
    ::WritePrivateProfileInt(L"Colors" , L"DarkOnly" ,m_bDarkOnly , szGLOBAL_PROFILE);


    // Save scripts here 
    // saves all scriptlets here in one file 
    int size;
    GetScriptFileList(&size);
    if ( m_ParseDlg.m_lstScriptlets.GetCount () > 2 + size) {
        CString strAllScripts;
        for ( POSITION pos = m_ParseDlg.m_lstScriptlets.GetHeadPosition () ; pos ; ) {
            CString strScriptlet = m_ParseDlg.m_lstScriptlets.GetNext(pos);
            strAllScripts += strScriptlet;
            strAllScripts += L"\r\n";
        }
        // now save it to file ".\\settings\\union.scr"
        HANDLE hFile;
        CString strFile = szSETTINGS_DIR;
        strFile += L"\\union.scr";
        hFile = CreateFile(strFile, GENERIC_READ| GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, NULL, NULL );
        if ( hFile!=INVALID_HANDLE_VALUE ) { 
            DWORD Written;
            WriteFile(hFile , strAllScripts, strAllScripts.GetLength() , &Written, NULL );
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
    dc.CreateDC(L"display" , NULL , NULL, NULL);
    CFont* pOldFont = dc.SelectObject(&m_fntText);

    CSize size = dc.GetTextExtent(L"A");
    
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
        ::WritePrivateProfileBinary(L"Font" , L"LOGFONT" ,(LPBYTE)&m_lfText, sizeof(m_lfText), szGLOBAL_PROFILE);

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

static DWORD lastDrawTickCount = 0;
void CSmcDoc::OnDrawTextAdded()
{
    if ( m_bFrozen ) 
        return;

	if ( !m_nUpdateCount && !m_bClearContents )
		return;
	
	DWORD now = GetTickCount();
	if( now - lastDrawTickCount < 20)
		return;
	lastDrawTickCount = now;

    m_UpdateSection.Lock();
    UpdateAllViews(NULL, TEXT_ARRIVED, NULL );
    m_nUpdateCount = 0;
	m_bClearContents = FALSE;
    CString str = m_strTempList.GetTail();
    m_strTempList.RemoveAll ();
    m_strTempList.AddTail(str);
    m_UpdateSection.Unlock();
}

static DWORD lastOutputDrawTickCount = 0;
void CSmcDoc::OnOutputTextAdded()
{
    if ( m_bFrozen ) 
        return;

	DWORD now = GetTickCount();
	if( now - lastOutputDrawTickCount < 20)
		return;
	lastOutputDrawTickCount = now;

    for (int i = 0; i < MAX_OUTPUT; i++) {
        if (m_bClearOutputContents[i] || m_nOutputUpdateCount[i] > 0) {
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

void CSmcDoc::DrawSome(const wchar_t* str)
{
    OutTextFrom(str, 0);
}



void CSmcDoc::OnOptionsScrollbuffer() 
{
	CScrollOptionDlg dlg(AfxGetMainWnd());

    dlg.m_nCount = nScrollSize;
    if ( dlg.DoModal() == IDOK ) {
        nScrollSize = dlg.m_nCount;

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

	DWORD dwSize;

    CString ScriptFileName = szSETTINGS_DIR;
    ScriptFileName += L"\\commonlib.scr";

    if ( (dwSize = read_file_contents(ScriptFileName, NULL, 0)) > 0 ) { 
        wchar_t *buff = strScriptText.GetBuffer(dwSize * sizeof(wchar_t));
		read_file_contents(ScriptFileName, buff, dwSize);
		strScriptText.ReleaseBuffer();

        m_ParseDlg.m_lstScriptlets.AddTail (strScriptText);
    } else {
        strScriptText.Empty ();
    }
    m_ParseDlg.m_strText = strScriptText;
    
    // Load scripts text
    wchar_t p[MAX_PATH+2];
    MakeAbsolutePath(p, szPROFILESCRIPT, szSETTINGS_DIR);
    ScriptFileName = p;
    CString strText;

	if ( (dwSize = read_file_contents(ScriptFileName, NULL, 0)) > 0 ) { 
        wchar_t *buff = strText.GetBuffer(dwSize * sizeof(wchar_t));
		read_file_contents(ScriptFileName, buff, dwSize);
		strText.ReleaseBuffer();

        m_ParseDlg.m_lstScriptlets.AddTail (strText);
        strScriptText += strText;
    }

    int size;
    GetScriptFileList(&size);
    for (int pos = 0; pos < size; pos++) {
        PCScriptFile pScr = GetScriptFile(pos);
        wchar_t p[MAX_PATH+2];
        MakeAbsolutePath(p, pScr->m_strName.data(), szSETTINGS_DIR);

		if ( (dwSize = read_file_contents(p, NULL, 0)) > 0 ) { 
			wchar_t *buff = strText.GetBuffer(dwSize * sizeof(wchar_t));
			read_file_contents(p, buff, dwSize);
			strText.ReleaseBuffer();

			m_ParseDlg.m_lstScriptlets.AddTail (strText);
			strScriptText += strText;
		}

        m_ParseDlg.m_strText = strText;
    }

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

void CSmcDoc::FillTabWords(const wchar_t* strWords)
{
    const wchar_t* ptr  = strWords;
	wchar_t *commands = NULL;

	if (!ptr) {
		int len = GetCommandsList(NULL);
		commands = new wchar_t[len + 1];
		GetCommandsList(commands);
		ptr = (const wchar_t*)commands;
	}
    
	while ( *ptr ) {
        while ( *ptr && iswspace(*ptr) ) 
            ptr++;
        if ( !*ptr ) 
            break;

        const wchar_t* ptrWord = ptr++;
        while ( *ptr && !iswspace(*ptr) ) 
            ptr++;

        int size = ptr-ptrWord;
        wchar_t* buff = new wchar_t[size+1];
        wcsncpy(buff, ptrWord, size);
        buff[size] = 0;
        m_lstTabWords.AddTail(buff);
        delete[] buff;
    }

	if (commands)
		delete[] commands;
}

void CSmcDoc::OnOptionsKeywords() 
{
	CKeyWordsDlg dlg(AfxGetMainWnd() );

    POSITION pos = m_lstTabWords.GetHeadPosition ();
    while ( pos ) {
		CString str = m_lstTabWords.GetNext (pos);
		if (str[0] != cCommandChar)
			dlg.m_strKeys += str + L"\r\n";
    }

    if ( dlg.DoModal() ) {
		m_lstTabWords.RemoveAll ();
        FillTabWords(dlg.m_strKeys);
		FillTabWords(NULL);
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
