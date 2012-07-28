// ProfilePage.cpp : implementation file
//

#include "stdafx.h"
#include "smc.h"
#include "ProfilePage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProfilePage property page

IMPLEMENT_DYNCREATE(CProfilePage, CPropertyPage)

CProfilePage::CProfilePage() : CPropertyPage(CProfilePage::IDD, IDS_PROFILE_PARAMS_CAPTION)
{
	//{{AFX_DATA_INIT(CProfilePage)
	m_strCommand = _T("");
	m_strLangFile = _T("");
	m_strLangSect = _T("");
	m_strSaveName = _T("");
	m_strStartFileName = _T("");
	//}}AFX_DATA_INIT
}

CProfilePage::~CProfilePage()
{
}

void CProfilePage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProfilePage)
	DDX_Text(pDX, IDC_COMMAND, m_strCommand);
	DDX_Text(pDX, IDC_SAVE_FILE, m_strSaveName);
	DDX_Text(pDX, IDC_LANG_FILE, m_strLangFile);
	DDX_Text(pDX, IDC_LANG_SECT, m_strLangSect);
	DDX_Text(pDX, IDC_START_FILE, m_strStartFileName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProfilePage, CPropertyPage)
	//{{AFX_MSG_MAP(CProfilePage)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProfilePage message handlers
