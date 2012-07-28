// NewProfileDlg.cpp : implementation file
//

#include "stdafx.h"
#include "smc.h"
#include "NewProfileDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewProfileDlg dialog


CNewProfileDlg::CNewProfileDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNewProfileDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewProfileDlg)
	m_strCommand = _T("");
	m_strName = _T("");
	m_strSaveFile = _T("");
	m_strStartFile = _T("");
	//}}AFX_DATA_INIT
}


void CNewProfileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewProfileDlg)
	DDX_Text(pDX, IDC_COMMAND, m_strCommand);
	DDX_Text(pDX, IDC_NAME, m_strName);
	DDX_Text(pDX, IDC_SAVE_FILE, m_strSaveFile);
	DDX_Text(pDX, IDC_START_FILE, m_strStartFile);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewProfileDlg, CDialog)
	//{{AFX_MSG_MAP(CNewProfileDlg)
	ON_EN_CHANGE(IDC_NAME, OnChangeName)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewProfileDlg message handlers

BOOL CNewProfileDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CenterWindow();
	
	GetDlgItem(IDOK)->EnableWindow(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNewProfileDlg::OnChangeName() 
{
	UpdateData();
	if ( m_strName.GetLength() ) {
//vls-begin// base dir
//        m_strStartFile = m_strName + ".set";
		m_strStartFile = szBASE_DIR;
        m_strStartFile += "\\" + m_strName + ".set";
//vls-end//
		m_strSaveFile = m_strStartFile;
		GetDlgItem(IDOK)->EnableWindow(TRUE);
	}
	else{
		m_strStartFile = "";
		m_strSaveFile = "";
		GetDlgItem(IDOK)->EnableWindow(FALSE);
	}
	UpdateData(FALSE);
}
