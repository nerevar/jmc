// LogParamsPage.cpp : implementation file
//

#include "stdafx.h"
#include "smc.h"
#include "LogParamsPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLogParamsPage property page

IMPLEMENT_DYNCREATE(CLogParamsPage, CPropertyPage)

CLogParamsPage::CLogParamsPage() : CPropertyPage(CLogParamsPage::IDD, IDS_LOG_PARAMS_CAPTION)
{
	//{{AFX_DATA_INIT(CLogParamsPage)
	m_bLogANSI = FALSE;
	m_bRMASupport = FALSE;
	m_nAppendMode = 0;
	m_bHTML = FALSE;
	//}}AFX_DATA_INIT
}

CLogParamsPage::~CLogParamsPage()
{
}

void CLogParamsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLogParamsPage)
	DDX_Check(pDX, IDC_KEEP_ANSI_IN_LOG, m_bLogANSI);
	DDX_Check(pDX, IDC_RMA_SUPPORT, m_bRMASupport);
	DDX_Radio(pDX, IDC_OVERWRITE_LOG_MODE, m_nAppendMode);
	DDX_Check(pDX, IDC_HTML_LOG, m_bHTML);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLogParamsPage, CPropertyPage)
	//{{AFX_MSG_MAP(CLogParamsPage)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLogParamsPage message handlers
