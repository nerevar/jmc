// AliasPage.cpp : implementation file
//

#include "stdafx.h"
#include "smc.h"
#include "AliasPage.h"
#include "ttcoreex/tintinx.h"
#include "ttcoreex/ttobjects.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAliasPage property page

IMPLEMENT_DYNCREATE(CAliasPage, CPropertyPage  )

CAliasPage::CAliasPage() : CGroupedPage(CAliasPage::IDD, IDS_ALIAS_PAGE_CAPTION)
{
	//{{AFX_DATA_INIT(CAliasPage)
	m_strName = _T("");
	m_strText = _T("");
	m_strGroup = _T("");
	//}}AFX_DATA_INIT
    m_bNewItem = FALSE;
}

CAliasPage::~CAliasPage()
{
}

void CAliasPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAliasPage)
	DDX_Control(pDX, IDC_GRP, m_cGroup);
	DDX_Control(pDX, IDC_GROUPS, m_cGroupList);
	DDX_Control(pDX, IDC_ALIAS, m_cAliasList);
	DDX_Text(pDX, IDC_NAME, m_strName);
	DDX_Text(pDX, IDC_TEXT, m_strText);
	DDX_CBString(pDX, IDC_GRP, m_strGroup);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAliasPage, CPropertyPage)
	//{{AFX_MSG_MAP(CAliasPage)
	ON_CBN_SELCHANGE(IDC_GROUPS, OnSelchangeGroups)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_ALIAS, OnItemchangedAlias)
	ON_EN_CHANGE(IDC_TEXT, OnChangeText)
	ON_CBN_SELCHANGE(IDC_GRP, OnSelchangeGrp)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_EN_KILLFOCUS(IDC_NAME, OnKillfocusName)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAliasPage message handlers

BOOL CAliasPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
    AddPage(L"alias", this);

    m_ImageList.Create(IDB_GROUP_ICONS, 16 , 2, (COLORREF)0xFFFFFF);

    DWORD Exstyle = ListView_GetExtendedListViewStyle(m_cAliasList.GetSafeHwnd());
    Exstyle += LVS_EX_FULLROWSELECT;
    ListView_SetExtendedListViewStyleEx(m_cAliasList.GetSafeHwnd () , 0 , Exstyle);

    // init list here 

    CString temp;
    temp.LoadString(IDS_AP_ALIAS_COL);
    m_cAliasList.InsertColumn (0 , temp , LVCFMT_LEFT , 100 );
    temp.LoadString(IDS_AP_TEXT_COL);
    m_cAliasList.InsertColumn (1 , temp , LVCFMT_LEFT , 200 );
    temp.LoadString(IDS_AP_GROUP_COL);
    m_cAliasList.InsertColumn (2 , temp , LVCFMT_LEFT , 100 );

    m_cAliasList.SetImageList(&m_ImageList, LVSIL_SMALL);
    

	// init All controls here 
    // init group list 
    m_cGroupList.Reinit();
    m_cGroup.Reinit (FALSE);

    // init columns
	ResetList();
    SetControls();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAliasPage::ResetList()
{
    m_cAliasList.DeleteAllItems ();

    int ind = m_cGroupList.GetCurSel ();
    ASSERT(ind >= 0 );
    PCGROUP pGrp = (PCGROUP)m_cGroupList .GetItemData (ind);

    int Count ;
    PPALIAS ppAlias = GetAliasList (&Count);
    for ( int i = 0 ; i < Count ; i++ ) {
        if ( !pGrp || (ppAlias[i]->m_pGroup == pGrp) ){
            AddItem(ppAlias[i]);
        }
    }
    SetControls();
}

void CAliasPage::SetControls()
{
    int pos = m_cAliasList.GetNextItem(-1, LVNI_SELECTED);
    if ( pos < 0 ) {
        m_strName = L"";
        m_strText = L"";
        GetDlgItem(IDC_NAME)->EnableWindow(FALSE);
        GetDlgItem(IDC_TEXT)->EnableWindow(FALSE);
        GetDlgItem(IDC_REMOVE)->EnableWindow(FALSE);
        m_cGroup.EnableWindow (FALSE);
    } else {
        GetDlgItem(IDC_NAME)->EnableWindow(FALSE);
        GetDlgItem(IDC_TEXT)->EnableWindow(TRUE);
        GetDlgItem(IDC_REMOVE)->EnableWindow(TRUE);
        m_cGroup.EnableWindow (TRUE);
        PALIAS pAlia = (PALIAS)m_cAliasList.GetItemData(pos);
        m_strName = pAlia->m_strLeft.data();
        m_strText = pAlia->m_strRight.data();
        m_cGroup.SelectGroup (pAlia->m_pGroup );
    }
    UpdateData(FALSE);
}

int CAliasPage::AddItem(void* p)
{
    int i = m_cAliasList.GetItemCount ();
    PALIAS pAlia = (PALIAS)p;
    LV_ITEM lvi;
    ZeroMemory(&lvi , sizeof(lvi));
    lvi.mask = LVIF_IMAGE | LVIF_TEXT;
    lvi.iItem = i;
    lvi.pszText  = (wchar_t*)pAlia->m_strLeft.c_str();
    lvi.iImage = pAlia->m_pGroup->m_bGlobal ? 1 : 0 ;
    int ind = m_cAliasList.InsertItem(&lvi);

    lvi.iItem = ind;
    lvi.iSubItem = 1;
    lvi.mask = LVIF_TEXT ;
    lvi.pszText  = (wchar_t*)pAlia->m_strRight.c_str();
    m_cAliasList.SetItem (&lvi);

    lvi.iSubItem = 2;
    lvi.mask = LVIF_TEXT ;
    lvi.pszText  = (wchar_t*)pAlia->m_pGroup->m_strName.c_str();
    m_cAliasList.SetItem (&lvi);
    m_cAliasList.SetItemData(ind, (DWORD)p);
    
    return ind;
}


void CAliasPage::OnSelchangeGroups() 
{
    ResetList();
}

void CAliasPage::OnItemchangedAlias(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
    if ( !(LVIF_STATE & pNMListView->uChanged) /*|| !(pNMListView->uNewState & LVNI_SELECTED)*/ )
        return;
    SetControls();
	
	*pResult = 0;
}


void CAliasPage::OnChangeText() 
{
    UpdateData();
    int pos = m_cAliasList.GetNextItem(-1, LVNI_SELECTED);
    ASSERT(pos >= 0 );
    if ( pos < 0 ) 
        return;
    PALIAS pAl = (PALIAS)m_cAliasList.GetItemData(pos);
    ASSERT(pAl);
    SetAlias((const wchar_t*)pAl->m_strLeft.c_str(), (const wchar_t*)m_strText, NULL);

    LV_ITEM lvi;
    ZeroMemory(&lvi , sizeof(lvi));
    lvi.iItem = pos;
    lvi.iSubItem = 1;
    lvi.mask = LVIF_TEXT ;
    lvi.pszText  = (LPWSTR)(const wchar_t*)m_strText;
    m_cAliasList.SetItem (&lvi);

}

void CAliasPage::OnSelchangeGrp() 
{
	PCGROUP pG = (PCGROUP)m_cGroup.GetItemData (m_cGroup.GetCurSel() ) ;
    ASSERT(pG);

    int pos = m_cAliasList.GetNextItem(-1, LVNI_SELECTED);
    ASSERT(pos >= 0 );
    PALIAS pAl = (PALIAS)m_cAliasList.GetItemData(pos);
    ASSERT(pAl);

    pAl->m_pGroup = pG;

    LV_ITEM lvi;
    ZeroMemory(&lvi , sizeof(lvi));
    lvi.iItem = pos;
    lvi.iSubItem = 2;
    lvi.mask = LVIF_TEXT ;
    lvi.pszText  = (LPWSTR)pG->m_strName.c_str();
    m_cAliasList.SetItem (&lvi);

    lvi.iSubItem = 0;
    lvi.mask = LVIF_IMAGE;
    lvi.iImage = pAl->m_pGroup->m_bGlobal ? 1 : 0;
    m_cAliasList.SetItem (&lvi);
}

void CAliasPage::OnAdd() 
{
	GetDlgItem(IDC_NAME)->EnableWindow(TRUE);
	GetDlgItem(IDC_TEXT)->EnableWindow(TRUE);
	GetDlgItem(IDC_GRP)->EnableWindow(TRUE);
    m_strName.Empty ();
    m_strText.Empty();
    PCGROUP pGrp = GetGroup (L"default");
    m_cGroup.SelectGroup (pGrp);
    UpdateData(FALSE);
    m_bNewItem = TRUE;
    GetDlgItem(IDC_NAME)->SetFocus();
}

void CAliasPage::OnRemove() 
{
    int pos = m_cAliasList.GetNextItem(-1, LVNI_SELECTED);
    ASSERT(pos >= 0 );
    PALIAS pAl = (PALIAS)m_cAliasList.GetItemData(pos);
    ASSERT(pAl);
	
    RemoveAlias((LPWSTR)pAl->m_strLeft.c_str());
    m_cAliasList.DeleteItem (pos);
    m_cAliasList.SetItemState(min(pos, m_cAliasList.GetItemCount () -1),
            LVNI_SELECTED | LVNI_FOCUSED ,  LVNI_SELECTED | LVNI_FOCUSED);
    SetControls();
}

void CAliasPage::OnKillfocusName() 
{
    if ( m_bNewItem ) {
        UpdateData();
        if ( m_strName.GetLength () == 0 ) {
            MessageBeep(MB_OK);
            SetControls();
            return;
        }
        if ( GetAlias((LPWSTR)(const wchar_t*)m_strName) ) {
            CString t1, t2;
            t1.LoadString(IDS_AP_ERR_EXISTS);
            MessageBox(t1 , ::AfxGetAppName() , MB_OK | MB_ICONSTOP);
            SetControls();
            return;
        }
        PALIAS pAl = SetAlias((const wchar_t*)m_strName, L"", NULL );
        int i = AddItem(pAl);
        int sel = m_cAliasList.GetNextItem(-1, LVNI_SELECTED);
        m_cAliasList.SetItemState(i,LVNI_SELECTED | LVNI_FOCUSED ,  LVNI_SELECTED | LVNI_FOCUSED  );
        m_cAliasList.EnsureVisible(i, TRUE);
        SetControls();
    }
}

void CAliasPage::GroupListChanged()
{
    m_cGroupList.Reinit();
    m_cGroup.Reinit (FALSE);

    // init columns
	ResetList();
    SetControls();
}
