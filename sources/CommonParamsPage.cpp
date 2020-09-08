// CommonParamsPage.cpp : implementation file
//

#include "stdafx.h"
#include "smc.h"
#include "CommonParamsPage.h"

extern DLLEXPORT UINT MudCodePage;
extern DLLEXPORT UINT MudCodePageUsed;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCommonParamsPage property page

IMPLEMENT_DYNCREATE(CCommonParamsPage, CPropertyPage)

CCommonParamsPage::CCommonParamsPage() : CPropertyPage(CCommonParamsPage::IDD, IDS_COMMON_PARAMS_CAPTION)
{
	//{{AFX_DATA_INIT(CCommonParamsPage)
	m_nStartLine = -1;
	m_nMinLen = 0;
	m_strCommandChar = _T("");
	m_strCommandDelimiter = _T("");
	m_nHistorySize = 0;
	m_bDisplayCommands = FALSE;
	m_bClearInput = FALSE;
	m_bTokenInput = FALSE;
	m_bScrollEnd = FALSE;
	m_bKillOneToken = FALSE;
	m_bConnectBeep = FALSE;
	m_bAutoReconnect = FALSE;
	m_bSplitOnBackscroll = FALSE;
	m_nTrigDelay = 0;
	m_bMinimizeToTray = FALSE;
	m_bLineWrap = TRUE;
	m_bShowTimestamps = FALSE;
	m_bSelectRect = FALSE;
	m_bRemoveESC = TRUE;
	m_bShowHidden = TRUE;
	m_bShowPing = TRUE;
	m_nUserInputHide = 1;
	//}}AFX_DATA_INIT
}

CCommonParamsPage::~CCommonParamsPage()
{
}

void CCommonParamsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCommonParamsPage)
	DDX_Control(pDX, IDC_COMMAND_DELIMITER, m_cCommandDelimiter);
	DDX_Control(pDX, IDC_COMMAND_CHAR, m_cCommandChar);
	DDX_Radio(pDX, IDC_START_LINE, m_nStartLine);
	DDX_Text(pDX, IDC_MIN_LEN, m_nMinLen);
	DDX_Text(pDX, IDC_COMMAND_CHAR, m_strCommandChar);
	DDX_Text(pDX, IDC_COMMAND_DELIMITER, m_strCommandDelimiter);
	DDX_Text(pDX, IDC_HISTORY_SIZE, m_nHistorySize);
	DDV_MinMaxUInt(pDX, m_nHistorySize, 1, 10000);
	DDX_Check(pDX, IDC_DISPLAY_COMMANDS, m_bDisplayCommands);
	DDX_Radio(pDX, IDC_INPUT_DONT_DISPLAY, m_nUserInputHide);
	DDX_Check(pDX, IDC_CLEAR_INPUT, m_bClearInput);
	DDX_Check(pDX, IDC_TOKEN_INPUT, m_bTokenInput);
	DDX_Check(pDX, IDC_AUTO_END, m_bScrollEnd);
	DDX_Check(pDX, IDC_TOKEN_LAST, m_bKillOneToken);
	DDX_Check(pDX, IDC_CONNECT_BEEP, m_bConnectBeep);
	DDX_Check(pDX, IDC_AUTO_RECONNECT, m_bAutoReconnect);
	DDX_Check(pDX, IDC_SPLIT_ONBACKSCROLL, m_bSplitOnBackscroll);
	DDX_Check(pDX, IDC_MINIMIZE_TO_TRAY, m_bMinimizeToTray);
	DDX_Text(pDX, IDC_TRIG_DELAY, m_nTrigDelay);
	DDX_Control(pDX, IDC_MUD_CODEPAGE, m_cCodePage);
	DDX_Check(pDX, IDC_CHK_LINEWRAP, m_bLineWrap);
	DDX_Check(pDX, IDC_DISPLAY_TIMESTAMPS, m_bShowTimestamps);
	DDX_Check(pDX, IDC_CHK_SELECTRECT, m_bSelectRect);
	DDX_Check(pDX, IDC_CHK_REMOVEESC, m_bRemoveESC);
	DDX_Check(pDX, IDC_CHK_SHOWHIDDEN, m_bShowHidden);
	DDX_Check(pDX, IDC_DISPLAY_PING, m_bShowPing);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCommonParamsPage, CPropertyPage)
	//{{AFX_MSG_MAP(CCommonParamsPage)
	ON_CBN_SELCHANGE(IDC_MUD_CODEPAGE, OnSelchangeCodePage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCommonParamsPage message handlers

BOOL CCommonParamsPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	wchar_t cpname[BUFFER_SIZE];
	int cpid;
	int i = 0;
	m_vIndexToCPID.clear();
	while ( (cpid = enumerate_codepage(i++, cpname, sizeof(cpname))) >= 0) {
		int ind = m_cCodePage.AddString (cpname);
		if ( cpid == m_nMudCodePage )
			m_cCodePage.SetCurSel(ind);
		m_vIndexToCPID.push_back(cpid);
	}
    
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CCommonParamsPage::OnSelchangeCodePage() 
{
	int ind = m_cCodePage.GetCurSel();
    if ( ind < 0 || ind >= m_vIndexToCPID.size() ) 
        return;
	m_nMudCodePage = m_vIndexToCPID[ind];
}

