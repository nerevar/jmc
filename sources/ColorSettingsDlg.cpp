// ColorSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "smc.h"
#include "ColorSettingsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CColorSettingsDlg dialog


CColorSettingsDlg::CColorSettingsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CColorSettingsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CColorSettingsDlg)
	//}}AFX_DATA_INIT
}


void CColorSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CColorSettingsDlg)
	DDX_Control(pDX, IDC_FORE_COLOR, m_ForeColor);
	DDX_Control(pDX, IDC_BACK_COLOR, m_BackColor);
	DDX_Control(pDX, 1007, m_Color7);
	DDX_Control(pDX, 1006, m_Color6);
	DDX_Control(pDX, 1005, m_Color5);
	DDX_Control(pDX, 1004, m_Color4);
	DDX_Control(pDX, 1003, m_Color3);
	DDX_Control(pDX, 1002, m_Color2);
	DDX_Control(pDX, 1001, m_Color1);
	DDX_Control(pDX, 1000, m_Color0);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CColorSettingsDlg, CDialog)
	//{{AFX_MSG_MAP(CColorSettingsDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorSettingsDlg message handlers


