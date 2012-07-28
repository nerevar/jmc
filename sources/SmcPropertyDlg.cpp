// SmcPropertyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "smc.h"
#include "SmcPropertyDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSmcPropertyDlg

IMPLEMENT_DYNAMIC(CSmcPropertyDlg, CPropertySheet)

CSmcPropertyDlg::CSmcPropertyDlg(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
}

CSmcPropertyDlg::CSmcPropertyDlg(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
}

CSmcPropertyDlg::~CSmcPropertyDlg()
{
}


BEGIN_MESSAGE_MAP(CSmcPropertyDlg, CPropertySheet)
	//{{AFX_MSG_MAP(CSmcPropertyDlg)
	ON_MESSAGE(WM_INITDIALOG, HandleInitDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSmcPropertyDlg message handlers

LRESULT CSmcPropertyDlg::HandleInitDialog(WPARAM, LPARAM)
{
	LRESULT lResult = CPropertySheet::OnInitDialog();
    HWND hwdnApplyBnt = ::GetDlgItem(m_hWnd, ID_APPLY_NOW);
    
    ASSERT(hwdnApplyBnt);
    if ( !hwdnApplyBnt ) 
        return lResult;

    GetDlgItem(ID_APPLY_NOW)->ShowWindow(SW_HIDE);
	GetDlgItem(ID_APPLY_NOW)->EnableWindow(FALSE);

    // Move buttons to left 

    CRect ApplyRect;
    static int Btns[2] = {IDOK, IDCANCEL};


    ::GetWindowRect(hwdnApplyBnt, &ApplyRect);
	// ScreenToClient(ApplyRect);

	for (int i = 0; i < 2; i++)
	{
		HWND hWnd = ::GetDlgItem(m_hWnd, Btns[i]);
		if (hWnd != NULL)
		{
            CRect rectOld;
			::GetWindowRect(hWnd, &rectOld);
			ScreenToClient(&rectOld);
			::SetWindowPos(hWnd, NULL,
				rectOld.left+ApplyRect.Width(), rectOld.top,
				0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}

	return lResult;
}
