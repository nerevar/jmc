// JmcGroupPage.cpp : implementation file
//

#include "stdafx.h"
#include "smc.h"
#include "JmcGroupPage.h"
#include "ttcoreex/tintinx.h"
#include "ttcoreex/ttobjects.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CJmcGroupPage property page

CMap<wchar_t*, wchar_t*, CGroupedPage*, CGroupedPage*> CGroupedPage::m_Pages;
CGroupedPage::CGroupedPage(UINT nIDTemplate, UINT nIDCaption ) : CPropertyPage(nIDTemplate, nIDCaption )
{

}

void CGroupedPage::AddPage(wchar_t* name, CGroupedPage* pg) 
{
    m_Pages.SetAt(name, pg);
}

void CGroupedPage::NotifyAll()
{
    POSITION pos = m_Pages.GetStartPosition();
    while (pos ) {
        CGroupedPage* pg;
        wchar_t* key;
        m_Pages.GetNextAssoc(pos, key, pg);
        pg->GroupListChanged();
    }
}


IMPLEMENT_DYNCREATE(CJmcGroupPage, CPropertyPage)

CJmcGroupPage::CJmcGroupPage() : CPropertyPage(CJmcGroupPage::IDD, IDS_GROUPS_PAGE_CAPTION)
{
	//{{AFX_DATA_INIT(CJmcGroupPage)
	m_bEnabled = FALSE;
	m_bGlobal = FALSE;
	//}}AFX_DATA_INIT
}

CJmcGroupPage::~CJmcGroupPage()
{
}

void CJmcGroupPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CJmcGroupPage)
	DDX_Control(pDX, IDC_GROUPS, m_cGroups);
	DDX_Control(pDX, IDC_GLOBAL, m_cGlobal);
	DDX_Control(pDX, IDC_ENABLED, m_cEnabled);
	DDX_Check(pDX, IDC_ENABLED, m_bEnabled);
	DDX_Check(pDX, IDC_GLOBAL, m_bGlobal);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CJmcGroupPage, CPropertyPage)
	//{{AFX_MSG_MAP(CJmcGroupPage)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_ENABLED, OnEnabledGlobal)
	ON_BN_CLICKED(IDC_GLOBAL, OnEnabledGlobal)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_GROUPS, OnItemchangedGroups)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CJmcGroupPage message handlers

int CJmcGroupPage::AddItem(CGROUP* pGrp)
{
    int i = m_cGroups.GetItemCount ();
    LV_ITEM lvi;
    ZeroMemory(&lvi , sizeof(lvi));
    lvi.mask = LVIF_IMAGE;
    lvi.iItem = i;
    lvi.iImage = pGrp->m_bGlobal ? 1 : 0 ;
    int ind = m_cGroups.InsertItem(&lvi);

    lvi.iItem = ind;
    lvi.iSubItem = 1;
    lvi.mask = LVIF_TEXT ;
    lvi.pszText  = (LPWSTR)pGrp->m_strName.c_str();
    m_cGroups.SetItem (&lvi);

    return ind;
}

void CJmcGroupPage::OnRemove() 
{
    CString strName;
    int i = m_cGroups.GetNextItem(-1, LVNI_SELECTED);
    ASSERT(i>=0);
    strName = m_cGroups.GetItemText(i, 1);

    CString strQuest;
    strQuest.Format(IDS_GP_DEL_QUES, strName);
    if ( MessageBox(strQuest , ::AfxGetAppName(), MB_YESNO | MB_ICONQUESTION ) != IDYES ) 
        return;

    RemoveGroup(strName);
    m_cGroups.DeleteItem (i);
    m_cGroups.SetItemState(min(i, m_cGroups.GetItemCount () -1),
            LVNI_SELECTED | LVNI_FOCUSED ,  LVNI_SELECTED | LVNI_FOCUSED );
    EnableControls();
    CGroupedPage::NotifyAll();
}

void CJmcGroupPage::OnAdd() 
{
    CAddGroupDlg dlg(this);
    if ( dlg.DoModal () == IDOK ) {
        PCGROUP pGrp = ::SetGroup(dlg.m_strName , TRUE, FALSE);
        int i = AddItem(pGrp);
        m_cGroups.SetItemState(i,LVNI_SELECTED | LVNI_FOCUSED ,  LVNI_SELECTED | LVNI_FOCUSED );
        EnableControls();
        CGroupedPage::NotifyAll();
    }
}

BOOL CJmcGroupPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

    m_ImageList.Create(IDB_GROUP_ICONS, 16 , 2, (COLORREF)0xFFFFFF);

    DWORD Exstyle = ListView_GetExtendedListViewStyle(m_cGroups.GetSafeHwnd());
    Exstyle += LVS_EX_FULLROWSELECT;
    ListView_SetExtendedListViewStyleEx(m_cGroups.GetSafeHwnd () , 0 , Exstyle);

    // init list here 
    CString t;
    t.LoadString(IDS_GP_ENABLED_COL);
    m_cGroups.InsertColumn (0 , t , LVCFMT_LEFT , 20 );
    t.LoadString(IDS_GP_NAME_COL);
    m_cGroups.InsertColumn (1 , t , LVCFMT_LEFT , 200 );

    m_cGroups.SetImageList(&m_ImageList, LVSIL_SMALL);


	int count ;
    PPCGROUP pGrpList = GetGroupsList (&count);
    for ( int i = 0 ; i < count ; i++ ) {
        AddItem(pGrpList[i]);
    }
    m_cGroups.SetItemState(0,LVNI_SELECTED | LVNI_FOCUSED ,  LVNI_SELECTED | LVNI_FOCUSED );

    EnableControls();

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CJmcGroupPage::EnableControls()
{
    UpdateData();
    int i = m_cGroups.GetNextItem(-1, LVNI_SELECTED);
    if ( i < 0 ) {
        m_cEnabled.EnableWindow (FALSE);
        m_cGlobal.EnableWindow (FALSE);
        GetDlgItem(IDC_REMOVE)->EnableWindow(FALSE);
        return;
    } 
    CString strName;
    strName = m_cGroups.GetItemText(i, 1);
    PCGROUP pGrp = GetGroup (strName);
    ASSERT(pGrp);
    
    m_cEnabled.EnableWindow (TRUE);
    m_cGlobal.EnableWindow (TRUE);
    GetDlgItem(IDC_REMOVE)->EnableWindow(TRUE);

    m_bGlobal = pGrp->m_bGlobal;
    m_bEnabled = pGrp->m_bEnabled ;
    UpdateData(FALSE);
}


void CJmcGroupPage::OnEnabledGlobal() 
{
	UpdateData();
    CString strName;
    int i = m_cGroups.GetNextItem(-1, LVNI_SELECTED);
    strName = m_cGroups.GetItemText(i, 1);
    SetGroup (strName, m_bEnabled, m_bGlobal);
    LV_ITEM lvi;
    ZeroMemory(&lvi , sizeof(lvi));
    lvi.mask = LVIF_IMAGE;
    lvi.iItem = i;
    lvi.iImage = m_bGlobal ? 1 : 0 ;
    m_cGroups.SetItem(&lvi);
}

/////////////////////////////////////////////////////////////////////////////
// CAddGroupDlg dialog


CAddGroupDlg::CAddGroupDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAddGroupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAddGroupDlg)
	m_strName = _T("");
	//}}AFX_DATA_INIT
}


void CAddGroupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddGroupDlg)
	DDX_Text(pDX, IDC_NAME, m_strName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddGroupDlg, CDialog)
	//{{AFX_MSG_MAP(CAddGroupDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddGroupDlg message handlers

void CAddGroupDlg::OnOK() 
{
	UpdateData();

    if ( m_strName.GetLength () == 0 ) {
        CString t;
        t.LoadString(IDS_GP_ERR_EMPTY);
        MessageBox(t, ::AfxGetAppName(), MB_OK | MB_ICONSTOP);
        GetDlgItem(IDC_NAME)->SetFocus();
        return;
    }

    CGROUP* pGrp = GetGroup(m_strName);
    if ( pGrp ) {
        CString t;
        t.LoadString(IDS_GP_ERR_EXIST);
        MessageBox(t, ::AfxGetAppName(), MB_OK | MB_ICONSTOP);
        GetDlgItem(IDC_NAME)->SetFocus();
        return;
    }
	CDialog::OnOK();
}

void CJmcGroupPage::OnItemchangedGroups(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here

    if ( !(LVIF_STATE & pNMListView->uChanged) /*|| !(pNMListView->uNewState & LVNI_SELECTED)*/ )
        return;
    
    EnableControls();	
	*pResult = 0;
}
