// ScriptPage.cpp : implementation file
//

#include "stdafx.h"
#include "smc.h"
#include "ScriptPage.h"
#include <atlconv.h>
#include <initguid.h>
#include <comcat.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CScriptPage property page

IMPLEMENT_DYNCREATE(CScriptPage, CPropertyPage)

CScriptPage::CScriptPage() : CPropertyPage(CScriptPage::IDD, IDS_SCRIPT_PAGE__CAPTION)
{
	//{{AFX_DATA_INIT(CScriptPage)
	m_bAllowDebug = FALSE;
	m_nErrorOutput = -1;
	//}}AFX_DATA_INIT
}

CScriptPage::~CScriptPage()
{
}

void CScriptPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CScriptPage)
	DDX_Control(pDX, IDC_LANG, m_cLang);
	DDX_Check(pDX, IDC_ALLOW_DEBUG, m_bAllowDebug);
	DDX_Radio(pDX, IDC_ERROR_MSGBOX, m_nErrorOutput);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CScriptPage, CPropertyPage)
	//{{AFX_MSG_MAP(CScriptPage)
	ON_CBN_SELCHANGE(IDC_LANG, OnSelchangeLang)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScriptPage message handlers
static const GUID CATID_ActiveScript = {0xf0b7a1a1, 0x9847, 0x11cf, {0x8f, 0x20, 0x0, 0x80, 0x5f, 0x2c, 0xd0, 0x64}};

BOOL CScriptPage::OnInitDialog() 
{
    USES_CONVERSION;
	CPropertyPage::OnInitDialog();
    // lets enum script engines 

    ICatRegister *	m_lpv = NULL;

	HRESULT hr = ::CoCreateInstance(CLSID_StdComponentCategoriesMgr, NULL, CLSCTX_ALL,
		IID_ICatRegister, (LPVOID *)&m_lpv);
    ASSERT(SUCCEEDED(hr));

	ICatInformation *lpic;
	hr = m_lpv->QueryInterface(IID_ICatInformation, (LPVOID *)&lpic);
    ASSERT(SUCCEEDED(hr));

	CATID ids[1];
	ids[0] = CATID_ActiveScript;
	IEnumCLSID *lpie;
	hr = lpic->EnumClassesOfCategories(1, ids, 0, NULL, &lpie);
	lpic->Release();
	ASSERT(SUCCEEDED(hr));


	ULONG uModules = 10;
	lpie->Reset();
	lpie->Next(uModules, m_clsIDs, &uModules);
	lpie->Release();

    for ( ULONG i = 0; i < uModules ; i++ ) {
        LPOLESTR ProgId;
        ProgIDFromCLSID (m_clsIDs[i], &ProgId);
        int ind = m_cLang.AddString (W2A(ProgId));
        if ( !memcmp(&m_clsIDs[i], &m_guidLang, sizeof(m_guidLang) ) )
            m_cLang.SetCurSel(ind);
    }

    m_lpv->Release ();
	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CScriptPage::OnSelchangeLang() 
{
	int ind = m_cLang.GetCurSel();
    if ( ind < 0 ) 
        return;
    memcpy(&m_guidLang , &m_clsIDs[ind], sizeof(m_guidLang) );
}

