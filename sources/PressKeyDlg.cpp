// PressKeyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "smc.h"
#include "PressKeyDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPressKeyDlg dialog


CPressKeyDlg::CPressKeyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPressKeyDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPressKeyDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
    m_nKeyCode = m_nAltState =0;
}


void CPressKeyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPressKeyDlg)
	DDX_Control(pDX, IDC_KEY, m_HotKey);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPressKeyDlg, CDialog)
	//{{AFX_MSG_MAP(CPressKeyDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPressKeyDlg message handlers

BOOL CPressKeyDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_HotKey.SetHotKey(m_nKeyCode, m_nAltState);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPressKeyDlg::OnOK() 
{
	m_HotKey.GetHotKey(m_nKeyCode, m_nAltState);
	
	CDialog::OnOK();
}
