// KeyWordsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "smc.h"
#include "KeyWordsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CKeyWordsDlg dialog


CKeyWordsDlg::CKeyWordsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CKeyWordsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CKeyWordsDlg)
	m_strKeys = _T("");
	//}}AFX_DATA_INIT
}


void CKeyWordsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CKeyWordsDlg)
	DDX_Text(pDX, IDC_KEYS, m_strKeys);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CKeyWordsDlg, CDialog)
	//{{AFX_MSG_MAP(CKeyWordsDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKeyWordsDlg message handlers
