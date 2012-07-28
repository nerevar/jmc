//vls-begin// script files
// JmcScriptFilesPage.cpp : implementation file
//

#include "stdafx.h"
#include "smc.h"
#include "smcDoc.h"
#include "JmcScriptFilesPage.h"
#include "ttcoreex/tintinx.h"
#include "ttcoreex/ttobjects.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CJmcScriptFilesPage property page

IMPLEMENT_DYNCREATE(CJmcScriptFilesPage, CPropertyPage)

CJmcScriptFilesPage::CJmcScriptFilesPage() : CPropertyPage(CJmcScriptFilesPage::IDD, IDS_SCRIPTFILES_PAGE_CAPTION)
{
	//{{AFX_DATA_INIT(CJmcScriptFilesPage)
	//}}AFX_DATA_INIT
}

CJmcScriptFilesPage::~CJmcScriptFilesPage()
{
}

void CJmcScriptFilesPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CJmcScriptFilesPage)
	DDX_Control(pDX, IDC_LIST, m_cFilesList);
	DDX_Control(pDX, IDC_UP, m_cUp);
	DDX_Control(pDX, IDC_REMOVE, m_cRemove);
	DDX_Control(pDX, IDC_DOWN, m_cDown);
	DDX_Control(pDX, IDC_ADD, m_cAdd);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CJmcScriptFilesPage, CPropertyPage)
	//{{AFX_MSG_MAP(CJmcScriptFilesPage)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_DOWN, OnDown)
	ON_LBN_SELCHANGE(IDC_LIST, OnSelchangeList)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_BN_CLICKED(IDC_UP, OnUp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CJmcScriptFilesPage message handlers

void CJmcScriptFilesPage::SetControls()
{
    int pos = m_cFilesList.GetCurSel();
    if (pos < 0) {
        m_cRemove.EnableWindow(FALSE);
        m_cUp.EnableWindow(FALSE);
        m_cDown.EnableWindow(FALSE);
    } else {
        m_cRemove.EnableWindow(TRUE);
        m_cUp.EnableWindow(pos > 0 ? TRUE : FALSE);
        m_cDown.EnableWindow(pos + 1 < m_cFilesList.GetCount() ? TRUE : FALSE);
    }

    CString str;
    CSize   sz;
    int     dx=0;
    CDC*    pDC = m_cFilesList.GetDC();
    for (int i = 0; i < m_cFilesList.GetCount(); i++)
    {
        m_cFilesList.GetText(i, str);
        sz = pDC->GetOutputTextExtent(str);
        
        if (sz.cx > dx)
            dx = sz.cx;
    }
    m_cFilesList.ReleaseDC(pDC);
    m_cFilesList.SetHorizontalExtent(dx);

    UpdateData(FALSE);
}

void CJmcScriptFilesPage::OnAdd() 
{
    CString strFile;
    char p[MAX_PATH+2];
    CString strCap;
    strCap.LoadString(IDS_SF_ADD_CAPTION);

    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = this->m_hWnd;
    ofn.lpstrFilter = "Script files (*.scr)\0*.scr\0All files (*.*)\0*.*\0";
    ofn.lpstrFile = p; *p = '\0';
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrInitialDir = szSETTINGS_DIR;
    ofn.lpstrTitle = (LPSTR)(LPCSTR)strCap;
    ofn.Flags = OFN_ENABLESIZING | OFN_HIDEREADONLY | OFN_NONETWORKBUTTON | OFN_FILEMUSTEXIST;

    char dir[MAX_PATH+2];
    GetCurrentDirectory(MAX_PATH, dir);
    BOOL bOk = GetOpenFileName(&ofn);
    SetCurrentDirectory(dir);

    strFile = p;
    MakeLocalPath(p, strFile, szSETTINGS_DIR);
    strFile = p;
    int i;
    if (bOk) {
        switch (FindScriptFile((LPSTR)(LPCSTR)strFile)) {
        case -2:
        case -3:
            AfxMessageBox(IDS_SF_ALREADY_USED_DEFAULT);
            m_cFilesList.SelectString(-1, strFile);
            break;
        case -1:
            AddScriptFile((LPSTR)(LPCSTR)strFile);
            i = m_cFilesList.AddString(strFile);
            m_cFilesList.SetCurSel(i);
            bScriptFileListChanged = TRUE;
            break;
        default:
            AfxMessageBox(IDS_SF_ALREADY_USED);
            m_cFilesList.SelectString(-1, strFile);
        }
    }
    SetControls();
}

void CJmcScriptFilesPage::OnDown() 
{
    int i = m_cFilesList.GetCurSel();
    if (i >= 0 && i + 1 < m_cFilesList.GetCount()) {
        CString strFile;
        m_cFilesList.GetText(i, strFile);
        DownScriptFile((LPSTR)(LPCSTR)strFile);
        m_cFilesList.DeleteString(i);
        m_cFilesList.InsertString(i + 1, strFile);
        m_cFilesList.SetCurSel(i + 1);
        bScriptFileListChanged = TRUE;
    }
    SetControls();
}

void CJmcScriptFilesPage::OnSelchangeList() 
{
    SetControls();
}

void CJmcScriptFilesPage::OnRemove() 
{
    int i = m_cFilesList.GetCurSel();
    if (i >= 0) {
        CString strFile;
        m_cFilesList.GetText(i, strFile);
        RemoveScriptFile((LPSTR)(LPCSTR)strFile);
        i = m_cFilesList.DeleteString(i);
        if (i > 0) i--;
        else i = 0;
        m_cFilesList.SetCurSel(i);
        bScriptFileListChanged = TRUE;
    }
    SetControls();
}

void CJmcScriptFilesPage::OnUp() 
{
    int i = m_cFilesList.GetCurSel();
    if (i > 0) {
        CString strFile;
        m_cFilesList.GetText(i, strFile);
        UpScriptFile((LPSTR)(LPCSTR)strFile);
        m_cFilesList.DeleteString(i);
        m_cFilesList.InsertString(i - 1, strFile);
        m_cFilesList.SetCurSel(i - 1);
        bScriptFileListChanged = TRUE;
    }
    SetControls();
}

BOOL CJmcScriptFilesPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

    bScriptFileListChanged = FALSE;

    int size;
    EnterCriticalSection(&secScriptFiles);
    GetScriptFileList(&size);
    for (int pos = 0; pos < size; pos++) {
        PCScriptFile pScr = GetScriptFile(pos);
        ASSERT(pScr);
        m_cFilesList.AddString(pScr->m_strName.data());
    }
    LeaveCriticalSection(&secScriptFiles);

    SetControls();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
//vls-end//
