// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "afxpriv.h"
#include "smc.h"
#include "MainFrm.h"
#include "smcDoc.h"
#include "smcView.h"

#include "CommonParamsPage.h"
#include "CharSubstPage.h"
#include "SmcPropertyDlg.h"
#include "ProfilePage.h"
#include "LogParamsPage.h"
#include "ScriptPage.h"
#include "JmcObjectsDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//vls-begin// multiple output
//vls-end//

static UINT indicators[] =
{
	    ID_SEPARATOR,           // status line indicator
        ID_TICKER,
        ID_INDICATOR_INFO1,
        ID_INDICATOR_INFO2,
        ID_INDICATOR_INFO3,
        ID_INDICATOR_INFO4,
        ID_INDICATOR_INFO5,
        ID_INDICATOR_CONNECTED,
	    ID_INDICATOR_LOGGED,
        ID_PATH_WRITING
};

enum INDICATOR_NUM{
        NUM_INDICATOR_INFO1 = 2,
        NUM_INDICATOR_INFO2,
        NUM_INDICATOR_INFO3,
        NUM_INDICATOR_INFO4,
        NUM_INDICATOR_INFO5,
        NUM_INDICATOR_CONNECTED,
	    NUM_INDICATOR_LOGGED,
        NUM_PATH_WRITING, 
        NUM_TICKER=1
};


CJMCStatus::CJMCStatus()
{
    m_bmpConnected.LoadBitmap (IDB_CONNECTED);
    m_bmpLogged.LoadBitmap (IDB_LOGGED);
    m_bmpMarked.LoadBitmap (IDB_MARKED);
}

//vls-begin// multiple output
static void __stdcall GetOutputName(int wnd, char *name, int maxlen)
{
    if (wnd >= 0 && wnd < MAX_OUTPUT && name)
    {
        int len;
        CString cs;
        CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();
        cs = pMainFrm->m_coolBar[wnd].m_sTitle;
        len = min(maxlen-1, cs.GetLength());
        strncpy(name, cs, len);
        name[len] = '\0';
    }
}
//vls-end//

static void DrawColoredText(LPDRAWITEMSTRUCT lpDrawItemStruct, LPCSTR strText)
{
    CSmcDoc* pDoc = (CSmcDoc*) (((CMainFrame*)AfxGetMainWnd())->GetActiveDocument());
    if ( !pDoc) 
        return;
    ASSERT( pDoc->IsKindOf( RUNTIME_CLASS( CSmcDoc ) ) );

    int Bg = 0, Fg = 7, bold = 0;

    
    // parse ANSI colors here 
    char* ptr = (char*)strText;
    if ( *ptr == 0x1B ) {
        ptr += 2; // skip [ symbol
        while ( *ptr && *ptr != 'm' ) { 
            char col[32];
            char* dest = col;
            while ( isdigit(*ptr) ) 
                *dest++ = *ptr++;
            // now set up color 
            *dest = 0;
            int value = atoi(col);
            if ( !value ) {
                Bg = 0;
                Fg = 7;
                bold = 0;
            }
            if ( value == 1 ) 
                bold = 1;
            if ( value <= 37 && value >= 30) {
                Fg = value-30;
            }
            if ( value <= 47 && value >= 40) {
                Bg = value-40;
            }
            if ( *ptr == ';' ) 
                ptr++;
        }
        if ( *ptr ) 
            ptr++;

    }
    // now set up colors 
    SetTextColor(lpDrawItemStruct->hDC , pDoc->m_ForeColors[Fg+bold*8]);
    SetBkColor(lpDrawItemStruct->hDC , pDoc->m_BackColors[Bg+bold*8]);

    SelectObject(lpDrawItemStruct->hDC ,pDoc->m_fntText.GetSafeHandle ());

    ExtTextOut(lpDrawItemStruct->hDC , 
        lpDrawItemStruct->rcItem.left , lpDrawItemStruct->rcItem.top, 
        ETO_OPAQUE, &lpDrawItemStruct->rcItem, ptr, strlen(ptr), NULL);
}

void CJMCStatus::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    static int count = 0;
    HBITMAP hb;
    switch ( lpDrawItemStruct->itemID  ) {
    case NUM_INDICATOR_INFO1:
        {
            DrawColoredText(lpDrawItemStruct, ((CMainFrame*)AfxGetMainWnd())->m_strInfo1);
        }
        return;
    case NUM_INDICATOR_INFO2:
        {
            DrawColoredText(lpDrawItemStruct, ((CMainFrame*)AfxGetMainWnd())->m_strInfo2);
        }
        return;
    case NUM_INDICATOR_INFO3:
        {
            DrawColoredText(lpDrawItemStruct, ((CMainFrame*)AfxGetMainWnd())->m_strInfo3);
        }
        return;
    case NUM_INDICATOR_INFO4:
        {
            DrawColoredText(lpDrawItemStruct, ((CMainFrame*)AfxGetMainWnd())->m_strInfo4);
        }
        return;
    case NUM_INDICATOR_INFO5:
        {
            DrawColoredText(lpDrawItemStruct, ((CMainFrame*)AfxGetMainWnd())->m_strInfo5);
        }
        return;
    case NUM_INDICATOR_CONNECTED:
        {
            if ( IsConnected() ) {
                hb = (HBITMAP)m_bmpConnected;
                break;
            }
        }
        return;
    case NUM_INDICATOR_LOGGED:
        {
            if ( IsLogging() ) {
                hb = (HBITMAP)m_bmpLogged;
                break;
            }
        }
        return;
    case NUM_PATH_WRITING:
        {
            if ( IsPathing () ) {
                hb = (HBITMAP)m_bmpMarked;
                break;
            }
        }
        return;
    default:
        return;
    }
    HDC dc = CreateCompatibleDC (lpDrawItemStruct->hDC );
    SelectObject(dc, hb);
    BitBlt(lpDrawItemStruct->hDC, lpDrawItemStruct->rcItem.left, 
        lpDrawItemStruct->rcItem.top, 
        lpDrawItemStruct->rcItem.right - lpDrawItemStruct->rcItem.left, 
        lpDrawItemStruct->rcItem.bottom- lpDrawItemStruct->rcItem.top, 
        dc, 0, 0 , SRCCOPY);

    DeleteDC(dc);
    return;
}


BEGIN_MESSAGE_MAP(CInvertSplit , CSplitterWnd)
	//{{AFX_MSG_MAP(CInvertSplit)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)


BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_OPTIONS_OPTIONS, OnOptionsOptions)
	ON_WM_DESTROY()
	ON_COMMAND(ID_UNSPLIT, OnUnsplit)
	ON_WM_SIZE()
	ON_COMMAND(ID_EDIT_JMCOBJECTS, OnEditJmcobjects)
	ON_WM_MOUSEWHEEL()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
    ON_UPDATE_COMMAND_UI(ID_INDICATOR_CONNECTED, OnUpdateConnected)
    ON_UPDATE_COMMAND_UI(ID_INDICATOR_LOGGED, OnUpdateLogged)
    ON_UPDATE_COMMAND_UI(ID_PATH_WRITING, OnUpdatePath)
    ON_UPDATE_COMMAND_UI(ID_TICKER, OnUpdateTicker)
	ON_COMMAND_EX(ID_VIEW_OUTPUTWINDOW, OnBarCheckEx)
//vls-begin// multiple output
	ON_COMMAND_EX(ID_VIEW_OUTPUTWINDOW1, OnBarCheckEx)
	ON_COMMAND_EX(ID_VIEW_OUTPUTWINDOW2, OnBarCheckEx)
	ON_COMMAND_EX(ID_VIEW_OUTPUTWINDOW3, OnBarCheckEx)
	ON_COMMAND_EX(ID_VIEW_OUTPUTWINDOW4, OnBarCheckEx)
	ON_COMMAND_EX(ID_VIEW_OUTPUTWINDOW5, OnBarCheckEx)
	ON_COMMAND_EX(ID_VIEW_OUTPUTWINDOW6, OnBarCheckEx)
	ON_COMMAND_EX(ID_VIEW_OUTPUTWINDOW7, OnBarCheckEx)
	ON_COMMAND_EX(ID_VIEW_OUTPUTWINDOW8, OnBarCheckEx)
	ON_COMMAND_EX(ID_VIEW_OUTPUTWINDOW9, OnBarCheckEx)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OUTPUTWINDOW1, OnUpdateControlBarMenu)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OUTPUTWINDOW2, OnUpdateControlBarMenu)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OUTPUTWINDOW3, OnUpdateControlBarMenu)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OUTPUTWINDOW4, OnUpdateControlBarMenu)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OUTPUTWINDOW5, OnUpdateControlBarMenu)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OUTPUTWINDOW6, OnUpdateControlBarMenu)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OUTPUTWINDOW7, OnUpdateControlBarMenu)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OUTPUTWINDOW8, OnUpdateControlBarMenu)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OUTPUTWINDOW9, OnUpdateControlBarMenu)
//vls-end// multiple output
	ON_UPDATE_COMMAND_UI(ID_VIEW_OUTPUTWINDOW, OnUpdateControlBarMenu)

	ON_UPDATE_COMMAND_UI(ID_INDICATOR_INFO1, OnUpdateInfo1)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_INFO2, OnUpdateInfo2)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_INFO3, OnUpdateInfo3)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_INFO4, OnUpdateInfo4)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_INFO5, OnUpdateInfo5)

    ON_MESSAGE(WM_USER+200, OnTabAdded)
    ON_MESSAGE(WM_USER+201, OnTabDeleted)

//vls-begin// #reloadscripts
    ON_MESSAGE(WM_USER+300, OnReloadScripts)
//vls-end//

//vls-begin// #quit
    ON_MESSAGE(WM_USER+400, OnQuitCommand)
//vls-end//

//vls-begin// multiple output
    ON_MESSAGE(WM_USER+500, OnShowOutput)
    ON_MESSAGE(WM_USER+501, OnNameOutput)
    ON_MESSAGE(WM_USER+502, OnDockOutput)
    ON_MESSAGE(WM_USER+505, OnPosWOutput)
//vls-end//

    ON_MESSAGE(WM_USER+600, OnCleanInput)
//* en status refreshing
    ON_MESSAGE(WM_USER+651, OnUpdStat1)
    ON_MESSAGE(WM_USER+652, OnUpdStat2)
    ON_MESSAGE(WM_USER+653, OnUpdStat3)
    ON_MESSAGE(WM_USER+654, OnUpdStat4)
    ON_MESSAGE(WM_USER+655, OnUpdStat5)
//*/en

END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
    m_wndSplitter.m_bInited = FALSE;
	
    m_wndSplitter.m_nUpSize = ::GetPrivateProfileInt("Main" , "UpSize" , 300, szGLOBAL_PROFILE);
    m_wndSplitter.m_nDownSize = ::GetPrivateProfileInt("Main" , "DownSize" , 100, szGLOBAL_PROFILE);
    bDisplayCommands  = ::GetPrivateProfileInt("Options" , "DisplayCommands" , 0, szGLOBAL_PROFILE);
    bDisplayInput  = ::GetPrivateProfileInt("Options" , "DisplayInput" , 1, szGLOBAL_PROFILE);
    MoreComingDelay  = ::GetPrivateProfileInt("Options" , "MoreComingDelay" , 100, szGLOBAL_PROFILE);
}

CMainFrame::~CMainFrame()
{
    ::WritePrivateProfileInt("Main" , "UpSize" , m_wndSplitter.m_nUpSize, szGLOBAL_PROFILE);
    ::WritePrivateProfileInt("Main" , "DownSize" , m_wndSplitter.m_nDownSize, szGLOBAL_PROFILE);
    ::WritePrivateProfileInt("Options" , "DisplayCommands" , bDisplayCommands, szGLOBAL_PROFILE);
    ::WritePrivateProfileInt("Options" , "DisplayInput" , bDisplayInput , szGLOBAL_PROFILE);
    ::WritePrivateProfileInt("Options" , "MoreComingDelay" , MoreComingDelay , szGLOBAL_PROFILE);
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{

    if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
    
	if (!m_wndToolBar.CreateEx
		(this,
		TBSTYLE_FLAT,
		WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY
		| CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}


    UINT Style, ID;
    int Size;
    m_wndStatusBar.GetPaneInfo(0 , ID, Style, Size);
    m_wndStatusBar.SetPaneInfo(0 , ID, Style, 30);

    m_wndStatusBar.GetPaneInfo(NUM_INDICATOR_INFO1 , ID, Style, Size);
    m_wndStatusBar.SetPaneInfo(NUM_INDICATOR_INFO1, ID, Style, 150);
    m_wndStatusBar.GetStatusBarCtrl().SetText(0, NUM_INDICATOR_INFO1, SBT_OWNERDRAW );

    m_wndStatusBar.GetPaneInfo(NUM_INDICATOR_INFO2 , ID, Style, Size);
    m_wndStatusBar.SetPaneInfo(NUM_INDICATOR_INFO2, ID, Style, 150);
    m_wndStatusBar.GetStatusBarCtrl().SetText(0, NUM_INDICATOR_INFO2, SBT_OWNERDRAW );

    m_wndStatusBar.GetPaneInfo(NUM_INDICATOR_INFO3 , ID, Style, Size);
    m_wndStatusBar.SetPaneInfo(NUM_INDICATOR_INFO3, ID, Style, 150);
    m_wndStatusBar.GetStatusBarCtrl().SetText(0, NUM_INDICATOR_INFO3, SBT_OWNERDRAW );

    m_wndStatusBar.GetPaneInfo(NUM_INDICATOR_INFO4 , ID, Style, Size);
    m_wndStatusBar.SetPaneInfo(NUM_INDICATOR_INFO4, ID, Style, 150);
    m_wndStatusBar.GetStatusBarCtrl().SetText(0, NUM_INDICATOR_INFO4, SBT_OWNERDRAW );

    m_wndStatusBar.GetPaneInfo(NUM_INDICATOR_INFO5 , ID, Style, Size);
    m_wndStatusBar.SetPaneInfo(NUM_INDICATOR_INFO5, ID, Style, 150);
    m_wndStatusBar.GetStatusBarCtrl().SetText(0, NUM_INDICATOR_INFO5, SBT_OWNERDRAW );
    

    m_wndStatusBar.GetPaneInfo(NUM_INDICATOR_CONNECTED , ID, Style, Size);
    m_wndStatusBar.SetPaneInfo(NUM_INDICATOR_CONNECTED , ID, Style, 14);
    m_wndStatusBar.GetStatusBarCtrl ().SetText(0, NUM_INDICATOR_CONNECTED, SBT_OWNERDRAW );


    m_wndStatusBar.GetPaneInfo(NUM_INDICATOR_LOGGED , ID, Style, Size);
    m_wndStatusBar.SetPaneInfo(NUM_INDICATOR_LOGGED , ID, Style, 14);
    m_wndStatusBar.GetStatusBarCtrl ().SetText(0, NUM_INDICATOR_LOGGED, SBT_OWNERDRAW );


    m_wndStatusBar.GetPaneInfo(NUM_PATH_WRITING , ID, Style, Size);
    m_wndStatusBar.SetPaneInfo(NUM_PATH_WRITING , ID, Style, 14);
    m_wndStatusBar.GetStatusBarCtrl ().SetText(0, NUM_PATH_WRITING, SBT_OWNERDRAW );


    m_wndStatusBar.GetPaneInfo(NUM_TICKER , ID, Style, Size);
    m_wndStatusBar.SetPaneInfo(NUM_TICKER , ID, Style, 20);

    // TODO: Remove this if you don't want tool tips or a resizeable toolbar
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

    m_editBar.Create(this , CBRS_BOTTOM  , 100 );
	
//vls-begin// multiple output
//    CString t;
//    t.LoadString(IDS_OUTPUT);
//    LPCSTR strTitle = t;
//
//	if (!m_coolBar.Create(this, strTitle, CSize(200, 100), ID_VIEW_OUTPUTWINDOW,  WS_CHILD | WS_VISIBLE | CBRS_TOP) )
//	{
//		TRACE0("Failed to create output window\n");
//		return -1;      // fail to create
//	}
//
//    m_coolBar.SetBarStyle(m_coolBar.GetBarStyle()|CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_SIZE_DYNAMIC);
//	m_coolBar.EnableDocking(CBRS_ALIGN_ANY);
//	DockControlBar(&m_coolBar);
//	LoadBarState("JMC");
//    m_coolBar.Load();
    CString t;
    t.LoadString(IDS_OUTPUT);
    
    for (int i = 0; i < MAX_OUTPUT; i++) {
        CString str;
        str.Format(t, i);
        LPCSTR strTitle = str;

        m_coolBar[i].m_wndCode = i;
        m_coolBar[i].m_wndAnsi.m_wndCode = i;
        if (!m_coolBar[i].Create(this, strTitle, CSize(200, 100), outputwindows[i],  WS_CHILD | WS_VISIBLE | CBRS_TOP) )
        {
            TRACE0("Failed to create output window\n");
            return -1;      // fail to create
        }
        m_coolBar[i].SetBarStyle(m_coolBar[i].GetBarStyle()|CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_SIZE_DYNAMIC);
        m_coolBar[i].Load();
        DockControlBar(&m_coolBar[i]);
		m_coolBar[i].m_pDockContext->ToggleDocking();
		CControlBar* pBar = GetControlBar(outputwindows[i]);
		wposes[i][0] = m_coolBar[i].m_mX;
		wposes[i][1] = m_coolBar[i].m_mY;
		if(pBar->IsFloating())
		{
			FloatControlBar(pBar,CPoint(m_coolBar[i].m_mX,m_coolBar[i].m_mY),0);
		}

    }
    LoadBarState("JMC");
    InitOutputNameFunc(GetOutputName);
	
//vls-end//
    
    SetTimer(1, 1000, NULL);

    // Load history here 
    CFile histFile;
    if ( histFile.Open("history.dat", CFile::modeRead ) ) {
        CArchive ar(&histFile, CArchive::load );
        m_editBar.GetHistory().Serialize (ar);
        m_editBar.m_nCurrItem = m_editBar.GetHistory().GetCount();
    }
        
//	GetDlgItem(ID_VIEW_MUDEMULATOR)->SetWindowText("Emulation");
    return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
    // cs.style -= WS_VISIBLE;
	return CFrameWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::OnOptionsOptions() 
{
    
    CSmcPropertyDlg dlg(IDS_OPTIONS, this);
    CCommonParamsPage pg1;    // Common params
    CCharSubstPage pg2;
    CProfilePage pg3;
    CLogParamsPage pg4;
    CScriptPage pg5;

	CSmcDoc* pDoc = (CSmcDoc*)GetActiveDocument();
	ASSERT_KINDOF(CSmcDoc, pDoc);
    
    // Fill common pparams
    pg1.m_nStartLine = m_editBar.m_nCursorPosWhileListing;
    pg1.m_nMinLen = m_editBar.m_nMinStrLen;
    pg1.m_strCommandChar = pDoc->m_cCommandChar;
    pg1.m_strCommandDelimiter = cCommandDelimiter;
    pg1.m_nHistorySize = m_editBar.GetHistorySize();
    pg1.m_bDisplayCommands = bDisplayCommands;
    pg1.m_bDisplayInput = bDisplayInput;
    pg1.m_bClearInput = m_editBar.m_bClearInput;
    pg1.m_bTokenInput = m_editBar.m_bTokenInput;
    pg1.m_bScrollEnd = m_editBar.m_bScrollEnd;
    pg1.m_bKillOneToken = m_editBar.m_bKillOneToken;
    pg1.m_bConnectBeep = bConnectBeep;
    pg1.m_bAutoReconnect = bAutoReconnect;
    pg1.m_bSplitOnBackscroll = pDoc->m_bSplitOnBackscroll;
    pg1.m_nTrigDelay = MoreComingDelay;


    // Fill subst params
    pg2.m_bAllowSubst = bSubstitution;
    EnterCriticalSection(&secSubstSection);
        memcpy(pg2.m_charsSubst, substChars , SUBST_ARRAY_SIZE);
    LeaveCriticalSection(&secSubstSection);

    if ( bIACSendSingle ) 
        pg2.m_nSendSingle  = 0;
    else 
        pg2.m_nSendSingle  = 1;
    if ( bIACReciveSingle ) 
        pg2.m_nReciveSingle  = 0;
    else 
        pg2.m_nReciveSingle  = 1;

	pg3.m_strCommand		= pDoc->m_strSaveCommand;
	pg3.m_strSaveName		= pDoc->m_strDefSaveFile;
	pg3.m_strStartFileName	= pDoc->m_strDefSetFile;
    pg3.m_strLangFile       = langfile;
    pg3.m_strLangSect       = langsect;
    
    // Log options
    pg4.m_bLogANSI = bANSILog;
    pg4.m_bRMASupport = bRMASupport;
    pg4.m_nAppendMode = bDefaultLogMode ? 1 : 0 ;
    pg4.m_bHTML = bHTML ;

    memcpy(&pg5.m_guidLang ,  &theApp.m_guidScriptLang, sizeof(GUID));
    pg5.m_bAllowDebug = bAllowDebug;
    pg5.m_nErrorOutput = nScripterrorOutput;


    dlg.AddPage(&pg1);
    dlg.AddPage(&pg2);
    dlg.AddPage(&pg3);
    dlg.AddPage(&pg4);
    dlg.AddPage(&pg5);
    // End fill common pparams

    if ( dlg.DoModal() == IDOK ) {
        m_editBar.m_nCursorPosWhileListing = pg1.m_nStartLine ;
        m_editBar.m_nMinStrLen = pg1.m_nMinLen;
		m_editBar.SetHistorySize(pg1.m_nHistorySize);
        ASSERT(pg1.m_strCommandChar.GetLength());
        cCommandChar = pDoc->m_cCommandChar = pg1.m_strCommandChar[0];
        cCommandDelimiter = pg1.m_strCommandDelimiter[0];
        bDisplayCommands = pg1.m_bDisplayCommands;
        bDisplayInput = pg1.m_bDisplayInput;
        m_editBar.m_bClearInput = pg1.m_bClearInput;
        m_editBar.m_bTokenInput = pg1.m_bTokenInput;
        m_editBar.m_bScrollEnd = pg1.m_bScrollEnd;
        m_editBar.m_bKillOneToken = pg1.m_bKillOneToken;
        bConnectBeep = pg1.m_bConnectBeep;
        bAutoReconnect = pg1.m_bAutoReconnect;
        pDoc->m_bSplitOnBackscroll = pg1.m_bSplitOnBackscroll;
        if ( !pg1.m_bSplitOnBackscroll ) 
            OnUnsplit();

         MoreComingDelay =pg1.m_nTrigDelay;

        bSubstitution = pg2.m_bAllowSubst ;
        EnterCriticalSection(&secSubstSection);
            memcpy(substChars ,pg2.m_charsSubst,  SUBST_ARRAY_SIZE);
        LeaveCriticalSection(&secSubstSection);

        bIACSendSingle = (pg2.m_nSendSingle == 0);
        bIACReciveSingle = (pg2.m_nReciveSingle == 0);

        
        pDoc->m_strSaveCommand = pg3.m_strCommand;
		pDoc->m_strDefSaveFile = pg3.m_strSaveName;
		pDoc->m_strDefSetFile = pg3.m_strStartFileName;
		strcpy(langfile, pg3.m_strLangFile);
		strcpy(langsect, pg3.m_strLangSect);

        // Log settings save
        bANSILog = pg4.m_bLogANSI;
        bRMASupport = pg4.m_bRMASupport;
        bDefaultLogMode = pg4.m_nAppendMode ;
        bHTML = pg4.m_bHTML; 

        if ( memcmp(&theApp.m_guidScriptLang, &pg5.m_guidLang , sizeof(GUID) ) ) {
            memcpy(&theApp.m_guidScriptLang, &pg5.m_guidLang , sizeof(GUID) ) ;
            PostMessage(WM_COMMAND, ID_SCRIPTING_RELOADSCRIPT, 0 ); 
        }
        bAllowDebug =pg5.m_bAllowDebug;
        nScripterrorOutput = pg5.m_nErrorOutput;
    }
}

void CMainFrame::OnUpdateFrameTitle(BOOL)
{
    CString text;
    CSmcDoc* pDoc = (CSmcDoc*)GetActiveDocument();
    if ( pDoc ) {
        text.Format(IDS_JABA_TITLE, pDoc->m_strProfileName);
        SetWindowText(text);
    }
}

void CMainFrame::OnUpdateLogged(CCmdUI* pUI)
{
    char buff[32];
    int Data;
    int val  = m_wndStatusBar.GetStatusBarCtrl ().GetText(buff, NUM_INDICATOR_LOGGED, &Data);

    BOOL bLog = IsLogging();
    if (  (bLog && val ) || (!bLog && !val)  ) 
        return ;

    m_wndStatusBar.GetStatusBarCtrl ().SetText((char*)bLog, NUM_INDICATOR_LOGGED, SBT_OWNERDRAW );
}

void CMainFrame::OnUpdateConnected(CCmdUI* pUI)
{
    char buff[32];
    int Data;
    int val  = m_wndStatusBar.GetStatusBarCtrl ().GetText(buff, NUM_INDICATOR_CONNECTED, &Data);

    BOOL bLog = IsConnected ();
    if (  (bLog && val ) || (!bLog && !val)  ) 
        return ;

    m_wndStatusBar.GetStatusBarCtrl ().SetText((char*)bLog, NUM_INDICATOR_CONNECTED, SBT_OWNERDRAW );
}

void CMainFrame::OnUpdatePath(CCmdUI* pUI)
{
    char buff[32];
    int Data;
    int val  = m_wndStatusBar.GetStatusBarCtrl ().GetText(buff, NUM_PATH_WRITING, &Data);

    BOOL bLog = IsPathing ();
    if (  (bLog && val ) || (!bLog && !val)  ) 
        return ;

    m_wndStatusBar.GetStatusBarCtrl ().SetText((char*)bLog, NUM_PATH_WRITING, SBT_OWNERDRAW );
}

void CMainFrame::OnUpdateTicker(CCmdUI* pUI)
{
    BOOL bStatus = bTickStatus;
    int toTick = iSecToTick;

    if ( bStatus ) {
        CString str;
        str.Format("%d", toTick);
        pUI->SetText(str);
    }
    else
        pUI->SetText("OFF");
}

void CMainFrame::OnUpdateInfo1(CCmdUI* pUI)
{
    EnterCriticalSection(&secStatusSection);
    if ( m_strInfo1 != strInfo1  ) {
        m_strInfo1 = strInfo1;
        m_wndStatusBar.GetStatusBarCtrl ().SetText(strInfo1, NUM_INDICATOR_INFO1, SBT_OWNERDRAW );
    }
    LeaveCriticalSection(&secStatusSection);
}

void CMainFrame::OnUpdateInfo2(CCmdUI* pUI)
{
    EnterCriticalSection(&secStatusSection);
    if ( m_strInfo2 != strInfo2  ) {
        m_strInfo2 = strInfo2;
        m_wndStatusBar.GetStatusBarCtrl ().SetText(strInfo2, NUM_INDICATOR_INFO2, SBT_OWNERDRAW );
    }
    LeaveCriticalSection(&secStatusSection);
}

void CMainFrame::OnUpdateInfo3(CCmdUI* pUI)
{
    EnterCriticalSection(&secStatusSection);
    if ( m_strInfo3 != strInfo3  ) {
        m_strInfo3 = strInfo3;
        m_wndStatusBar.GetStatusBarCtrl ().SetText(strInfo3, NUM_INDICATOR_INFO3, SBT_OWNERDRAW );
    }
    LeaveCriticalSection(&secStatusSection);
}

void CMainFrame::OnUpdateInfo4(CCmdUI* pUI)
{
    EnterCriticalSection(&secStatusSection);
    if ( m_strInfo4 != strInfo4  ) {
        m_strInfo4 = strInfo4;
        m_wndStatusBar.GetStatusBarCtrl ().SetText(strInfo4, NUM_INDICATOR_INFO4, SBT_OWNERDRAW );
    }
    LeaveCriticalSection(&secStatusSection);
}

void CMainFrame::OnUpdateInfo5(CCmdUI* pUI)
{
    EnterCriticalSection(&secStatusSection);
    if ( m_strInfo5 != strInfo5  ) {
        m_strInfo5 = strInfo5;
        m_wndStatusBar.GetStatusBarCtrl ().SetText(strInfo5, NUM_INDICATOR_INFO5, SBT_OWNERDRAW );
    }
    LeaveCriticalSection(&secStatusSection);
}


void CMainFrame::OnDestroy() 
{
    
    WINDOWPLACEMENT wp;
	memset (&wp, 0 , sizeof(wp));
	wp.length = sizeof(wp);
	GetWindowPlacement(&wp);
	if ( wp.showCmd == SW_SHOWMINIMIZED ) 
		wp.showCmd = SW_SHOW;
    ::WritePrivateProfileBinary("View" , "WindowPlacement" ,(LPBYTE)&wp, sizeof(wp), szGLOBAL_PROFILE);

	CWinApp* pApp = AfxGetApp();
    const char* pProfSave= pApp->m_pszProfileName;
	pApp->m_pszProfileName = szGLOBAL_PROFILE;
	SaveBarState("View");
	pApp->m_pszProfileName = pProfSave;
	
    if ( m_wndSplitter.GetRowCount() > 1 ) {
        m_wndSplitter.SavePosition();
    }

    SaveBarState("JMC");
//vls-begin// multiple output
//    m_coolBar.Save();
    for (int i = 0; i < MAX_OUTPUT; i++)
        m_coolBar[i].Save();
//vls-end//

    // save history 
    CFile histFile;
//vls-begin// base dir
//    if ( histFile.Open("history.dat", CFile::modeCreate | CFile::modeWrite ) ) {
    CString strFile(szBASE_DIR);
    strFile += "\\history.dat";
    if ( histFile.Open(strFile, CFile::modeCreate | CFile::modeWrite ) ) {
//vls-end//
        CArchive ar(&histFile, CArchive::store);
        m_editBar.GetHistory().Serialize (ar);
    }

    pMainWnd = NULL;
    CFrameWnd::OnDestroy();
}

void CMainFrame::RestorePosition()
{
	// Loading state of control bars
	CWinApp* pApp = AfxGetApp();
    const char* pProfSave= pApp->m_pszProfileName;
	pApp->m_pszProfileName = szGLOBAL_PROFILE;
	LoadBarState("View");
	pApp->m_pszProfileName = pProfSave;
    UINT  nSize;
    LPBYTE pData;
    if ( ::GetPrivateProfileBinary ("View", "WindowPlacement", &pData, &nSize, szGLOBAL_PROFILE) ) {
		WINDOWPLACEMENT wp;
		memcpy(&wp, pData , nSize);
		delete pData;
		SetWindowPlacement(&wp);
	}
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
		if (!m_wndSplitter.Create(this,2,1, CSize(10, 10), pContext, 
            WS_CHILD | WS_VISIBLE | /*WS_VSCROLL | */SPLS_DYNAMIC_SPLIT))
		{
			TRACE0("Failed to create split bar ");
			return FALSE;    // failed to create
		}

        pMainWnd = this;
		m_wndSplitter.m_bInited = TRUE;
        return TRUE;
}


BOOL CInvertSplit::SplitRow()
{
    CRect rect;

    int TotalSize, MinSize;
    GetRowInfo(0, TotalSize, MinSize);

    TotalSize -=  m_cySplitter;
    int Before = TotalSize*m_nUpSize/(m_nDownSize+m_nUpSize) + m_cyBorder;

    BOOL bRet = CSplitterWnd::SplitRow(Before);

    if ( !bRet ) 
        return FALSE;

    CSmcView* pView = (CSmcView*)GetPane(0, 0 );
    CSmcView* pMainView = (CSmcView*)GetPane(1, 0 );
    CSmcDoc* pDoc = pView->GetDocument();

    pMainView->GetClientRect(&rect);
    int lineCount = rect.Height() / pDoc->m_nYsize;
    // now scroll this lines
    int minpos, maxpos;
    pView->GetScrollRange(SB_VERT, &minpos, &maxpos);
    pView->SetScrollPos(SB_VERT, maxpos - lineCount, TRUE);

    pView->InvalidateRect(NULL, FALSE);
    pView->UpdateWindow();

    // Copy contents of old view to the new view 
    pMainView->m_strList.RemoveAll();
    pMainView->m_strList.AddHead(&pView->m_strList);
    pMainView->m_nCurrentBg  = pView->m_nCurrentBg;
    pMainView->m_nCurrentFg  = pView->m_nCurrentFg;
    pMainView->m_bAnsiBold = pView->m_bAnsiBold;
    pMainView->InvalidateRect(NULL, FALSE);
    pMainView->UpdateWindow();

    return bRet;
}


BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) 
{
	if ( pMsg->message == WM_KEYDOWN ) {
        switch ( pMsg->wParam ) {
		case VK_PAUSE:
			{
				OnUnsplit();
			}
			return TRUE;
        case VK_ESCAPE:
            {
                if ( m_wndSplitter.GetRowCount() == 1 ) {
                    CEdit* pEdit = (CEdit*)m_editBar.GetDlgItem(IDC_EDIT);
                    pEdit->SetWindowText("");
                }
                else 
                    OnUnsplit();
            }
            break;
        case VK_PRIOR:
            {
                CSmcDoc* pDoc = (CSmcDoc*)GetActiveDocument();
                // first check second pane exist etc 
                if ( m_wndSplitter.GetRowCount() == 1 && pDoc->m_bSplitOnBackscroll ) {
                    m_wndSplitter.SplitRow();
                } else {
                    CWnd* pWnd = m_wndSplitter.GetPane(0, 0 );
                    if ( pWnd ) 
                        pWnd->PostMessage(WM_VSCROLL , MAKELONG(SB_PAGEUP, 0), 0);
                }
            }
            return TRUE;
        case VK_NEXT:
            {
                CWnd* pWnd = m_wndSplitter.GetPane(0, 0 );
                if ( pWnd ) {
                    pWnd->SendMessage(WM_VSCROLL , MAKELONG(SB_PAGEDOWN, 0), 0);
                    int minpos, maxpos, pos;
                    pWnd->GetScrollRange(SB_VERT, &minpos, &maxpos);
                    pos = pWnd->GetScrollPos(SB_VERT);
                    if ( pos == maxpos ) 
                        OnUnsplit() ;
                }
                
            }
            return TRUE;
        case VK_UP:
            if ( GetKeyState(VK_CONTROL) < 0 )  {
                CSmcDoc* pDoc = (CSmcDoc*)GetActiveDocument();
                // first check second pane exist etc 
                if ( m_wndSplitter.GetRowCount() == 1 && pDoc->m_bSplitOnBackscroll ) {
                    m_wndSplitter.SplitRow();
                } else {
                    CWnd* pWnd = m_wndSplitter.GetPane(0, 0 );
                    if ( pWnd ) 
                        pWnd->PostMessage(WM_VSCROLL , MAKELONG(SB_LINEUP, 0), 0);
                }
            }
            return TRUE;
        case VK_DOWN:
            if ( GetKeyState(VK_CONTROL) < 0 )  {
                CWnd* pWnd = m_wndSplitter.GetPane(0, 0 );
                if ( pWnd ) {
                    pWnd->SendMessage(WM_VSCROLL , MAKELONG(SB_LINEDOWN, 0), 0);
                    int minpos, maxpos, pos;
                    pWnd->GetScrollRange(SB_VERT, &minpos, &maxpos);
                    pos = pWnd->GetScrollPos(SB_VERT);
                    if ( pos == maxpos ) 
                        OnUnsplit() ;
                }
                
            }
            return TRUE;
        default:
            break;
        }                                                                           
    }
	return CFrameWnd::PreTranslateMessage(pMsg);
}

void CMainFrame::OnUnsplit() 
{
	if ( m_wndSplitter.GetRowCount() == 1 ) 
        return;

    m_wndSplitter.SavePosition();

    m_wndSplitter.DeleteRow(0);

    CWnd* pWnd = m_wndSplitter.GetPane(0, 0);
    pWnd->InvalidateRect(NULL, FALSE);
    pWnd->UpdateWindow();

}


void CInvertSplit::SavePosition()
{
	if ( GetRowCount() == 1 ) 
        return;

    int ideal;
    GetRowInfo(0, m_nUpSize, ideal);
    GetRowInfo(1, m_nDownSize, ideal);
}


void CMainFrame::OnSize(UINT nType, int cx, int cy) 
{
	CFrameWnd::OnSize(nType, cx, cy);
}

void CInvertSplit::OnSize(UINT nType, int cx, int cy) 
{
    
    CSplitterWnd::OnSize(nType, cx, cy);

    if ( ! m_bInited ) 
        return;

    int ideal, Up, Down;
    GetRowInfo(0, Up, ideal);
    GetRowInfo(1, Down, ideal);
    
    Up = m_nUpSize*(Up+Down)/(m_nDownSize+m_nUpSize);
    SetRowInfo(0, Up, 10);
    RecalcLayout();

}

/////////////////////////////////////////////////////////////////////////////
// COutputBar

COutputBar::COutputBar()
{
}

COutputBar::~COutputBar()
{
}

BEGIN_MESSAGE_MAP(COutputBar, CCoolDialogBar)
	//{{AFX_MSG_MAP(COutputBar)
	ON_WM_CREATE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// COutputBar message handlers

int COutputBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CCoolDialogBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CRect rect;
    GetClientRect(&rect);
    VERIFY(m_wndAnsi.Create(NULL, "", WS_CHILD | WS_VISIBLE, rect, this, 97));
	
	return 0;
}

void COutputBar::OnSize(UINT nType, int cx, int cy) 
{
	CCoolDialogBar::OnSize(nType, cx, cy);
	
    if ( m_wndAnsi.GetSafeHwnd() == NULL ) 
        return ;


    m_wndAnsi.SetWindowPos(NULL, 0 , 0 , cx, cy, SWP_NOZORDER | SWP_NOMOVE);

}

BOOL CMainFrame::OnBarCheckEx(UINT nID)
{
    int count = m_wndSplitter.GetRowCount ();
    for ( int i = 0 ; i < count ; i++ ) {
        (m_wndSplitter.GetPane(i, 0))->InvalidateRect(NULL);
    }

//vls-begin// multiple output
    CControlBar* pBar = GetControlBar(nID);

    if (pBar != NULL) {
        BOOL bVisible = (pBar->GetStyle() & WS_VISIBLE);
        for (int i = 0; i < MAX_OUTPUT; i++) {
            if (nID == outputwindows[i]) {
                m_coolBar[i].m_bFlag = !bVisible;
                break;
            }
        }
    }


//vls-end//

    return OnBarCheck(nID);
}

#include "AliasPage.h"
#include "JmcGroupPage.h"
#include "JMCActionsPage.h"
#include "JmcHlightPage.h"
#include "JmcHotkeyPage.h"
//vls-begin// subst page
#include "JmcSubstPage.h"
//vls-end//
//vls-begin// script files
#include "JmcScriptFilesPage.h"
//vls-end//

void CMainFrame::OnEditJmcobjects() 
{
	WaitForSingleObject (eventAllObjectEvent, INFINITE);
    CJmcObjectsDlg dlg(IDS_JMC_OBJECTS, this);
    CAliasPage pg1;
    CJMCActionsPage pg2;
    CJmcHlightPage pg3;
    CJmcHotkeyPage pg4;
    CJmcGroupPage pg5;

//vls-begin// subst page
    CJmcSubstPage pg6;
//vls-end//

//vls-begin// script files
    CJmcScriptFilesPage pg7;
//vls-end//

    dlg.AddPage(&pg1);
    dlg.AddPage(&pg2);
    dlg.AddPage(&pg3);
    dlg.AddPage(&pg4);
    dlg.AddPage(&pg5);
//vls-begin// subst page
    dlg.AddPage(&pg6);
//vls-end//
//vls-begin// script files
    dlg.AddPage(&pg7);
//vls-end//

    dlg.DoModal();

    SetEvent(eventAllObjectEvent );

//vls-begin// script files
    if (bScriptFileListChanged) {
        PostMessage(WM_COMMAND, ID_SCRIPTING_RELOADSCRIPT, 0);
    }
//vls-end//
}

LONG CMainFrame::OnTabAdded( UINT wParam, LONG lParam)
{
    HGLOBAL hg = (HGLOBAL)lParam;
    char* p = (char*)GlobalLock(hg);
    CSmcDoc* pDoc = (CSmcDoc*) (((CMainFrame*)AfxGetMainWnd())->GetActiveDocument());

    POSITION pos = pDoc->m_lstTabWords.GetHeadPosition ();
    while (pos ) {
        CString str = pDoc->m_lstTabWords.GetAt(pos);
        if ( !strcmpi(p, str) ){
            pDoc->m_lstTabWords.RemoveAt (pos);
            break;
        }
        pDoc->m_lstTabWords.GetNext(pos);
    }
    pDoc->m_lstTabWords.AddHead(p);

    GlobalUnlock (hg);
    GlobalFree(hg);

    return 0;
}

LONG CMainFrame::OnTabDeleted( UINT wParam, LONG lParam)
{
    HGLOBAL hg = (HGLOBAL)lParam;
    char* p = (char*)GlobalLock(hg);
    CSmcDoc* pDoc = (CSmcDoc*) (((CMainFrame*)AfxGetMainWnd())->GetActiveDocument());

    POSITION pos = pDoc->m_lstTabWords.GetHeadPosition ();
    while (pos ) {
        CString str = pDoc->m_lstTabWords.GetAt(pos);
        if ( !strcmpi(p, str) ){
            pDoc->m_lstTabWords.RemoveAt (pos);
            break;
        }
        pDoc->m_lstTabWords.GetNext(pos);
    }

    GlobalUnlock (hg);
    GlobalFree(hg);

    return 0;
}

//vls-begin// #reloadscripts
LONG CMainFrame::OnReloadScripts(UINT wParam, LONG lParam)
{
    // just emulating menu: scripts->reload
    return SendMessage(WM_COMMAND, ID_SCRIPTING_RELOADSCRIPT, 0);
}
//vls-end//

//vls-begin// #quit
LONG CMainFrame::OnQuitCommand(UINT wParam, LONG lParam)
{
    // just emulating menu: file->exit
    return SendMessage(WM_COMMAND, ID_APP_EXIT, 0);
}
//vls-end//

//vls-begin// multiple output
LONG CMainFrame::OnShowOutput(UINT wParam, LONG lParam)
{
    int wnd = (int)wParam;
    int opt = (int)lParam;
    UINT nId = outputwindows[wnd];
    CControlBar* pBar = GetControlBar(nId);

    if (pBar != NULL && opt > 0) {
        BOOL bVisible = (pBar->GetStyle() & WS_VISIBLE);
        if ( (opt>1 && bVisible) || (opt==1 && !bVisible) )
            return 0;
    }
    return SendMessage(WM_COMMAND, nId, 0);
}

LONG CMainFrame::OnNameOutput(UINT wParam, LONG lParam)
{
    int wnd = (int)wParam;
    HGLOBAL hg = (HGLOBAL)lParam;

    char* p = (char*)GlobalLock(hg);
    CString cs;
    if (p && p[0]) {
        cs = p;
    } else {
        CString t;
        t.LoadString(IDS_OUTPUT);
        cs.Format(t, wnd);
    }
    if (wnd >=0 && wnd < MAX_OUTPUT)
	{
        m_coolBar[wnd].SetTitle(cs);
		//* en
		if(m_coolBar[wnd].IsFloating())
		{
			CControlBar * pBar = GetControlBar(outputwindows[wnd]);
			FloatControlBar(pBar, CPoint(m_coolBar[wnd].m_mX,m_coolBar[wnd].m_mY),0);
		}
		//*/en
	}

    GlobalUnlock(hg);
    GlobalFree(hg);

    return 0;
}
//* en
LONG CMainFrame::OnCleanInput(UINT wParam, LONG lParam)
{
    return m_editBar.CleanLine()?1:0;
}

LONG CMainFrame::OnUpdStat1(UINT wParam, LONG lParam)
{
	char bf[BUFFER_SIZE];strcpy(bf,strInfo1);
	strcpy(strInfo1,"?");CMainFrame::OnUpdateInfo1(NULL);
	strcpy(strInfo1,bf); CMainFrame::OnUpdateInfo1(NULL);

	return 1;
}

LONG CMainFrame::OnUpdStat2(UINT wParam, LONG lParam)
{
	char bf[BUFFER_SIZE];strcpy(bf,strInfo2);
	strcpy(strInfo2,"?");CMainFrame::OnUpdateInfo2(NULL);
	strcpy(strInfo2,bf); CMainFrame::OnUpdateInfo2(NULL);
	return 1;
}

LONG CMainFrame::OnUpdStat3(UINT wParam, LONG lParam)
{
	char bf[BUFFER_SIZE];strcpy(bf,strInfo3);
	strcpy(strInfo3,"?");CMainFrame::OnUpdateInfo3(NULL);
	strcpy(strInfo3,bf); CMainFrame::OnUpdateInfo3(NULL);
	return 1;
}

LONG CMainFrame::OnUpdStat4(UINT wParam, LONG lParam)
{
	char bf[BUFFER_SIZE];strcpy(bf,strInfo4);
	strcpy(strInfo4,"?");CMainFrame::OnUpdateInfo4(NULL);
	strcpy(strInfo4,bf); CMainFrame::OnUpdateInfo4(NULL);
	return 1;
}

LONG CMainFrame::OnUpdStat5(UINT wParam, LONG lParam)
{
	char bf[BUFFER_SIZE];strcpy(bf,strInfo5);
	strcpy(strInfo5,"?");CMainFrame::OnUpdateInfo5(NULL);
	strcpy(strInfo5,bf); CMainFrame::OnUpdateInfo5(NULL);
	return 1;
}

LONG CMainFrame::OnDockOutput(UINT wParam, LONG lParam)
{
    int wnd = (int)wParam;

    DWORD cs = lParam ? CBRS_ALIGN_ANY : 0;
	
    if (wnd >=0 && wnd < MAX_OUTPUT)
	{
        m_coolBar[wnd].EnableDocking(cs);
	    m_coolBar[wnd].m_Dock = cs;
	}

    return 0;
}

LONG CMainFrame::OnPosWOutput(UINT wParam, LONG lParam)
{
    int p1,p2;
    int wnd = (int)wParam;

	p1 = lParam & 32767;
	p2 = lParam>>16;
    UINT nId = outputwindows[wnd];
    CControlBar* pBar = GetControlBar(nId);

    if ((pBar != NULL) && (m_coolBar[wnd])) {
        FloatControlBar(pBar,CPoint(p1,p2),0);
		m_coolBar[wnd].m_mX = p1;
		m_coolBar[wnd].m_mY = p2;
    }
  
    return 0;
}
//*/en

//vls-begin// mouse wheel
BOOL CMainFrame::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	if (!(GetKeyState(VK_SHIFT)&0x8000 || GetKeyState(VK_CONTROL)&0x8000 || GetKeyState(VK_MENU)&0x8000))
        return CFrameWnd::OnMouseWheel(nFlags, zDelta, pt);

    CWnd* pWnd = m_wndSplitter.GetPane(0, 0 );
    WPARAM wParam = MAKELONG(zDelta < 0 ? SB_PAGEDOWN : SB_PAGEUP, 0);
	if(GetKeyState(VK_SHIFT)&0x8000 || GetKeyState(VK_MENU)&0x8000)
		wParam = MAKELONG(zDelta < 0 ? SB_LINEDOWN : SB_LINEUP, 0);

    if (zDelta > 0) 
        if ( m_wndSplitter.GetRowCount() == 1 && pDoc->m_bSplitOnBackscroll )
            m_wndSplitter.SplitRow();

	if ( pWnd )
        pWnd->SendMessage(WM_VSCROLL , wParam, 0);

    if (zDelta < 0) 
	{
        int minpos, maxpos, pos;
        pWnd->GetScrollRange(SB_VERT, &minpos, &maxpos);
        pos = pWnd->GetScrollPos(SB_VERT);
        if ( pos == maxpos ) 
            OnUnsplit() ;
    }
	
	return CFrameWnd::OnMouseWheel(nFlags, zDelta, pt);
}
//vls-end//
