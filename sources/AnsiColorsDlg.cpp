// AnsiColorsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "smc.h"
#include "smcDoc.h"
#include "AnsiColorsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAnsiColorsDlg dialog


CAnsiColorsDlg::CAnsiColorsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAnsiColorsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAnsiColorsDlg)
	m_bDarkOnly = FALSE;
	//}}AFX_DATA_INIT
}


void CAnsiColorsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAnsiColorsDlg)
	DDX_Check(pDX, IDC_DARK_ONLY, m_bDarkOnly);
	//}}AFX_DATA_MAP
    for ( int i = 0 ; i < 16 ; i++ ) {
        DDX_Control(pDX, 1000+i, m_Btn[i]);
    }
}


BEGIN_MESSAGE_MAP(CAnsiColorsDlg, CDialog)
	//{{AFX_MSG_MAP(CAnsiColorsDlg)
	ON_BN_CLICKED(IDC_RESTORE_DEFAULT, OnRestoreDefault)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAnsiColorsDlg message handlers

BOOL CAnsiColorsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
    CenterWindow();	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAnsiColorsDlg::OnRestoreDefault() 
{
	// TODO: Add your control notification handler code here
	    
    int i;
    for ( i = 0 ; i < 16 ; i++ ) 
	{
        m_Btn[i].SetColor(DefColors[i]);
        m_Btn[i].InvalidateRect(NULL);
    }
}
