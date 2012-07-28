// JmcObjectsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "smc.h"
#include "JmcObjectsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CJmcObjectsDlg



IMPLEMENT_DYNAMIC(CJmcObjectsDlg, CPropertySheet)

CJmcObjectsDlg::CJmcObjectsDlg(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
}

CJmcObjectsDlg::CJmcObjectsDlg(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
}

CJmcObjectsDlg::~CJmcObjectsDlg()
{
}


BEGIN_MESSAGE_MAP(CJmcObjectsDlg, CPropertySheet)
	//{{AFX_MSG_MAP(CJmcObjectsDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CJmcObjectsDlg message handlers

BOOL CJmcObjectsDlg::OnInitDialog() 
{
	LRESULT lResult = CPropertySheet::OnInitDialog();
    HWND hwdnApplyBnt = ::GetDlgItem(m_hWnd, ID_APPLY_NOW);
    
    ASSERT(hwdnApplyBnt);
    if ( !hwdnApplyBnt ) 
        return lResult;

    GetDlgItem(ID_APPLY_NOW)->ShowWindow(SW_HIDE);
	GetDlgItem(ID_APPLY_NOW)->EnableWindow(FALSE);

    GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);
	GetDlgItem(IDCANCEL)->EnableWindow(FALSE);


    // Move buttons to left 

    CRect ApplyRect;
    static int Btns[2] = {IDOK, IDCANCEL};


    ::GetWindowRect(hwdnApplyBnt, &ApplyRect);
	ScreenToClient(ApplyRect);
    GetDlgItem(IDOK)->SetWindowPos(NULL,  
        ApplyRect.left , ApplyRect.top , 0 , 0 , SWP_NOSIZE | SWP_NOZORDER);

    CString t;
    t.LoadString(IDS_CLOSE);
    GetDlgItem(IDOK)->SetWindowText(t);

	return lResult;
}
