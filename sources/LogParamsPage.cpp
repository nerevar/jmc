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
	m_bRMASupport = FALSE;
	m_bHTMLTimestamps = FALSE;
	m_bAppendLogTitle = TRUE;
	m_nAppendMode = 0;
	m_nLogAs = 0;
	m_LogType = -1;
	//}}AFX_DATA_INIT
}

CLogParamsPage::~CLogParamsPage()
{
}

void CLogParamsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLogParamsPage)
	DDX_Control(pDX, IDC_LOGTYPE_TEXT, m_LogTypeControl);
	DDX_Control(pDX, IDC_RMA_SUPPORT, m_RmaSupportControl);
	DDX_Control(pDX, IDC_HTML_TIMESTAMPS, m_HtmlTimestampsControl);
	DDX_Check(pDX, IDC_RMA_SUPPORT, m_bRMASupport);
	DDX_Check(pDX, IDC_HTML_TIMESTAMPS, m_bHTMLTimestamps);
	DDX_Check(pDX, IDC_LOG_TITLE, m_bAppendLogTitle);
	DDX_Radio(pDX, IDC_OVERWRITE_LOG_MODE, m_nAppendMode);
	DDX_Radio(pDX, IDC_WRITE_LOG_AS_SHOWN_BY_SERVER, m_nLogAs);
	DDX_Radio(pDX, IDC_LOGTYPE_TEXT, m_LogType);
	DDX_Control(pDX, IDC_LOG_CODEPAGE, m_cCodePage);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLogParamsPage, CPropertyPage)
	//{{AFX_MSG_MAP(CLogParamsPage)
	ON_BN_CLICKED(IDC_LOGTYPE_TEXT, OnChangeLogType)
	ON_BN_CLICKED(IDC_LOGTYPE_HTML, OnChangeLogType)
	ON_BN_CLICKED(IDC_LOGTYPE_ANSI, OnChangeLogType)
	ON_WM_SHOWWINDOW()
	ON_CBN_SELCHANGE(IDC_LOG_CODEPAGE, OnSelchangeCodePage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLogParamsPage message handlers

void CLogParamsPage::OnChangeLogType() 
{
	UpdateData();

	m_RmaSupportControl.EnableWindow(m_LogType == 2);
	m_HtmlTimestampsControl.EnableWindow(m_LogType == 1);
}

void CLogParamsPage::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CPropertyPage::OnShowWindow(bShow, nStatus);
	
	m_RmaSupportControl.EnableWindow(m_LogType == 2);
	m_HtmlTimestampsControl.EnableWindow(m_LogType == 1);
}

BOOL CLogParamsPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	wchar_t cpname[BUFFER_SIZE];
	int cpid;
	int ind, i = 0;
	m_vIndexToCPID.clear();

	cpid = 0;
	wcscpy(cpname, L"Same as main CodePage");
	ind = m_cCodePage.AddString (cpname);
	if ( cpid == m_nLogCodePage )
		m_cCodePage.SetCurSel(ind);
	m_vIndexToCPID.push_back(cpid);

	while ( (cpid = enumerate_codepage(i++, cpname, sizeof(cpname))) >= 0) {
		ind = m_cCodePage.AddString (cpname);
		if ( cpid == m_nLogCodePage )
			m_cCodePage.SetCurSel(ind);
		m_vIndexToCPID.push_back(cpid);
	}
    
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CLogParamsPage::OnSelchangeCodePage() 
{
	int ind = m_cCodePage.GetCurSel();
    if ( ind < 0 || ind >= m_vIndexToCPID.size() ) 
        return;
	m_nLogCodePage = m_vIndexToCPID[ind];
}

