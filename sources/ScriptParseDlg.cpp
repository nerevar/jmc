// ScriptParseDlg.cpp : implementation file
//

#include "stdafx.h"
#include "smc.h"
#include "ScriptParseDlg.h"
#include "smcdoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CScriptParseDlg dialog


CScriptParseDlg::CScriptParseDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScriptParseDlg::IDD, pParent)
{
    m_nBtnCX =  m_nCXShift = m_nCYShift = m_nBtnCY = 0;

    m_bDoParseScriptlet = FALSE;
    m_nScriptletIndex = 0;
    m_hParseDoneEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	//{{AFX_DATA_INIT(CScriptParseDlg)
	m_strText = _T("");
	//}}AFX_DATA_INIT
}


void CScriptParseDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CScriptParseDlg)
	DDX_Control(pDX, IDOK, m_cParse);
	DDX_Control(pDX, IDCANCEL, m_cClose);
	DDX_Control(pDX, IDC_NEXT, m_cNext);
	DDX_Control(pDX, IDC_PREV, m_cPrev);
	DDX_Control(pDX, IDC_TEXT, m_cText);
	DDX_Text(pDX, IDC_TEXT, m_strText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CScriptParseDlg, CDialog)
	//{{AFX_MSG_MAP(CScriptParseDlg)
	ON_WM_TIMER()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_NEXT, OnNext)
	ON_BN_CLICKED(IDC_PREV, OnPrev)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScriptParseDlg message handlers

void CScriptParseDlg::OnCancel() 
{
    ShowWindow (SW_HIDE);
}

void CScriptParseDlg::OnOK() 
{
	UpdateData();
    // Do parsing here 
    
    if ( m_lstScriptlets.GetCount() == 0 || m_strText.Compare (m_lstScriptlets.GetTail () )  ){
        m_lstScriptlets.AddTail (m_strText);
        m_nScriptletIndex = m_lstScriptlets.GetCount()-1;
        SetPNKeys ();
    }
    
    m_bDoParseScriptlet = TRUE;

    // WaitForSingleObject (m_hParseDoneEvent, INFINITE);
    EnableWindow(FALSE);
    SetTimer(1, 100, NULL );
}

void CScriptParseDlg::OnTimer(UINT nIDEvent) 
{
    if ( !m_bDoParseScriptlet ) {
        KillTimer(1 ) ;
        EnableWindow(TRUE);
        m_cText.SetSel (0, -1);
        m_cText.SetFocus ();
        UpdateData (FALSE);
    }
}

int CScriptParseDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
    int x = ::GetPrivateProfileInt("View" , "ScriptDlgX" ,0 , szGLOBAL_PROFILE) ;
    int y = ::GetPrivateProfileInt("View" , "ScriptDlgY" ,0 , szGLOBAL_PROFILE) ;
    SetWindowPos(NULL, x, y , 0 , 0 , SWP_NOZORDER | SWP_NOSIZE);
    // --CHANGED by Anton Likhtarov
	SetIcon(LoadIcon(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDD_SCRIPT_PARSE)), TRUE);
    // --END
	return 0;
}

void CScriptParseDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	WINDOWPLACEMENT wp;
    GetWindowPlacement (&wp);
    ::WritePrivateProfileInt("View" , "ScriptDlgX" ,wp.rcNormalPosition.left , szGLOBAL_PROFILE);
    ::WritePrivateProfileInt("View" , "ScriptDlgY" ,wp.rcNormalPosition.top , szGLOBAL_PROFILE);
    ::WritePrivateProfileInt("View" , "ScriptDlgCX" ,wp.rcNormalPosition.right - wp.rcNormalPosition.left , szGLOBAL_PROFILE);
    ::WritePrivateProfileInt("View" , "ScriptDlgCY" ,wp.rcNormalPosition.bottom - wp.rcNormalPosition.top , szGLOBAL_PROFILE);
	
}

void CScriptParseDlg::OnNext() 
{
	if ( m_nScriptletIndex > m_lstScriptlets.GetCount() -1 ) 
        return;

	m_nScriptletIndex++;
    if ( m_nScriptletIndex < m_lstScriptlets.GetCount() ) {
        POSITION pos = m_lstScriptlets.FindIndex (m_nScriptletIndex);
        m_strText = m_lstScriptlets.GetAt(pos);
    } else {
        m_strText.Empty();
    }
    UpdateData(FALSE);
    SetPNKeys();
}

void CScriptParseDlg::OnPrev() 
{
	if ( m_nScriptletIndex <= 0 ) 
        return;
	m_nScriptletIndex--;
    POSITION pos = m_lstScriptlets.FindIndex (m_nScriptletIndex);

    m_strText = m_lstScriptlets.GetAt(pos);
    UpdateData(FALSE);
    SetPNKeys();
}

void CScriptParseDlg::SetPNKeys()
{
    if ( m_nScriptletIndex > 0 ) 
        m_cPrev.EnableWindow (TRUE);
    else 
        m_cPrev.EnableWindow (FALSE);

    if ( m_nScriptletIndex <= m_lstScriptlets.GetCount() -1 ) 
        m_cNext.EnableWindow (TRUE);
    else 
        m_cNext.EnableWindow (FALSE);

}


BOOL CScriptParseDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
    SetPNKeys ();

    CRect rect, clirect;;
    m_cPrev.GetWindowRect(&rect);
    
    GetClientRect(&clirect);

    ScreenToClient (&rect);
    m_nBtnCX = rect.Width ();
    m_nBtnCY = rect.Height ();
    m_nCXShift = clirect.right - rect.right;

    m_cText.GetWindowRect (&rect);
    ScreenToClient (&rect);

    m_nCYShift = clirect.bottom - rect.bottom ;

    m_cText.SetFont (&pDoc->m_fntText );

    int x = ::GetPrivateProfileInt("View" , "ScriptDlgCX" ,0 , szGLOBAL_PROFILE) ;
    int y = ::GetPrivateProfileInt("View" , "ScriptDlgCY" ,0 , szGLOBAL_PROFILE) ;
    
    if ( x!= 0  && y != 0 ) {
        SetWindowPos(NULL, 0,0 ,x, y, SWP_NOZORDER | SWP_NOMOVE);
    }
    
    return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CScriptParseDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	if ( !m_cText.m_hWnd ) 
        return;
    
    CRect rect;
    m_cText.GetWindowRect (&rect);
    ScreenToClient (&rect);

    
    HDWP wp = BeginDeferWindowPos(5);
    DeferWindowPos(wp, m_cText.GetSafeHwnd () , NULL, 0, 0 , 
        cx - 3*m_nCXShift - m_nBtnCX , cy - rect.top - m_nCYShift, SWP_NOZORDER | SWP_NOMOVE );

    DeferWindowPos(wp, m_cPrev.GetSafeHwnd () , NULL, cx-m_nCXShift - m_nBtnCX , rect.top, 
        0, 0, SWP_NOZORDER | SWP_NOSIZE );
    DeferWindowPos(wp, m_cNext.GetSafeHwnd () , NULL, 
        cx-m_nCXShift - m_nBtnCX , rect.top + m_nBtnCY + m_nCYShift/3, 
        0, 0, SWP_NOZORDER | SWP_NOSIZE );



    DeferWindowPos(wp, m_cParse.GetSafeHwnd () , NULL, 
        cx-m_nCXShift - m_nBtnCX , cy- 4*m_nCYShift/3- 2*m_nBtnCY , 
        0, 0, SWP_NOZORDER | SWP_NOSIZE );


    DeferWindowPos(wp, m_cClose.GetSafeHwnd () , NULL, 
        cx-m_nCXShift - m_nBtnCX , cy- m_nCYShift- m_nBtnCY , 
        0, 0, SWP_NOZORDER | SWP_NOSIZE );
    EndDeferWindowPos (wp);
}

void CScriptParseDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{

	CDialog::OnGetMinMaxInfo(lpMMI);


    lpMMI->ptMinTrackSize.x = 250;
    lpMMI->ptMinTrackSize.y = 200;
}
