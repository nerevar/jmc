// ScrollOptionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "smc.h"
#include "ScrollOptionDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CScrollOptionDlg dialog


CScrollOptionDlg::CScrollOptionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScrollOptionDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CScrollOptionDlg)
	m_nCount = 0;
	//}}AFX_DATA_INIT
}


void CScrollOptionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CScrollOptionDlg)
	DDX_Text(pDX, IDC_COUNT, m_nCount);
	DDV_MinMaxUInt(pDX, m_nCount, 1, 30000);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CScrollOptionDlg, CDialog)
	//{{AFX_MSG_MAP(CScrollOptionDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScrollOptionDlg message handlers
