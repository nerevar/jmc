// JMCActionsPage.cpp : implementation file
//

#include "stdafx.h"
#include "smc.h"
#include "JMCActionsPage.h"
#include "ttcoreex/tintinx.h"
#include "ttcoreex/ttobjects.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CJMCActionsPage property page

IMPLEMENT_DYNCREATE(CJMCActionsPage, CPropertyPage)

CJMCActionsPage::CJMCActionsPage() : CGroupedPage(CJMCActionsPage::IDD, IDS_ACTION_PAGE_CAPTION)
{
	//{{AFX_DATA_INIT(CJMCActionsPage)
	m_strName = _T("");
	m_strText = _T("");
	m_strGroup = _T("");
	m_nPriority = -1;
	m_nInputType = -1;
	//}}AFX_DATA_INIT
    m_bNewItem = FALSE;
}

CJMCActionsPage::~CJMCActionsPage()
{
}

void CJMCActionsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CJMCActionsPage)
	DDX_Control(pDX, IDC_GRP, m_cGroup);
	DDX_Control(pDX, IDC_GROUPS, m_cGroupList);
	DDX_Control(pDX, IDC_ACTIONS, m_cActionsList);
	DDX_Text(pDX, IDC_NAME, m_strName);
	DDX_Text(pDX, IDC_TEXT, m_strText);
	DDX_CBString(pDX, IDC_GRP, m_strGroup);
	DDX_CBIndex(pDX, IDC_PRIORITY, m_nPriority);
	DDX_CBIndex(pDX, IDC_ACTION_TYPE, m_nInputType);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CJMCActionsPage, CPropertyPage)
	//{{AFX_MSG_MAP(CJMCActionsPage)
	ON_CBN_SELCHANGE(IDC_GROUPS, OnSelchangeGroups)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_ACTIONS, OnItemchangedActions)
	ON_EN_CHANGE(IDC_TEXT, OnChangeText)
	ON_CBN_SELCHANGE(IDC_GRP, OnSelchangeGrp)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_EN_KILLFOCUS(IDC_NAME, OnKillfocusName)
	ON_CBN_SELCHANGE(IDC_PRIORITY, OnSelchangePriority)
	ON_CBN_SELCHANGE(IDC_ACTION_TYPE, OnSelchangeInputType)
	ON_EN_CHANGE(IDC_NAME, OnChangeName)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CJMCActionsPage message handlers
/////////////////////////////////////////////////////////////////////////////
// CJMCActionsPage message handlers

BOOL CJMCActionsPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
    AddPage("action", this);

    m_ImageList.Create(IDB_GROUP_ICONS, 16 , 2, (COLORREF)0xFFFFFF);

    DWORD Exstyle = ListView_GetExtendedListViewStyle(m_cActionsList.GetSafeHwnd());
    Exstyle += LVS_EX_FULLROWSELECT;
    ListView_SetExtendedListViewStyleEx(m_cActionsList.GetSafeHwnd () , 0 , Exstyle);

    // init list here 
    CString t;
    t.LoadString(IDS_TP_PATTERN_COL);
    m_cActionsList.InsertColumn (0 , t , LVCFMT_LEFT , 250 );
    t.LoadString(IDS_TP_COMMAND_COL);
    m_cActionsList.InsertColumn (1 , t , LVCFMT_LEFT , 150 );
    t.LoadString(IDS_AP_GROUP_COL);
    m_cActionsList.InsertColumn (2 , t , LVCFMT_LEFT , 100 );
    t.LoadString(IDS_TP_PRIORITY_COL);
    m_cActionsList.InsertColumn (3 , t , LVCFMT_LEFT , 30 );
	t.LoadString(IDS_TP_INPUTTYPE_COL);
    m_cActionsList.InsertColumn (4 , t , LVCFMT_LEFT , 70 );
	
    m_cActionsList.SetImageList(&m_ImageList, LVSIL_SMALL);
    

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

void CJMCActionsPage::ResetList()
{
    m_cActionsList.DeleteAllItems ();

    int ind = m_cGroupList.GetCurSel ();
    ASSERT(ind >= 0 );
    PCGROUP pGrp = (PCGROUP)m_cGroupList .GetItemData (ind);

    int Count ;
    PPACTION ppAction = GetActionsList(&Count);
    for ( int i = 0 ; i < Count ; i++ ) {
        if ( !pGrp || (ppAction[i]->m_pGroup == pGrp) ){
            AddItem(ppAction[i]);
        }
    }
    SetControls();
}

void CJMCActionsPage::SetControls()
{
    int pos = m_cActionsList.GetNextItem(-1, LVNI_SELECTED);
    if ( pos < 0 ) {
        m_strName = "";
        m_strText = "";
        GetDlgItem(IDC_NAME)->EnableWindow(FALSE);
        GetDlgItem(IDC_TEXT)->EnableWindow(FALSE);
        GetDlgItem(IDC_REMOVE)->EnableWindow(FALSE);
        GetDlgItem(IDC_PRIORITY)->EnableWindow(FALSE);
		GetDlgItem(IDC_ACTION_TYPE)->EnableWindow(FALSE);
        m_cGroup.EnableWindow (FALSE);
    } else {
        GetDlgItem(IDC_NAME)->EnableWindow(TRUE);
        GetDlgItem(IDC_TEXT)->EnableWindow(TRUE);
        GetDlgItem(IDC_REMOVE)->EnableWindow(TRUE);
        GetDlgItem(IDC_PRIORITY)->EnableWindow(TRUE);
		GetDlgItem(IDC_ACTION_TYPE)->EnableWindow(TRUE);
        m_cGroup.EnableWindow (TRUE);
        PACTION pAct = (PACTION)m_cActionsList.GetItemData(pos);
        m_strName = pAct->m_strLeft.data();
        m_strText = pAct->m_strRight.data();
        m_cGroup.SelectGroup (pAct->m_pGroup );
        m_nPriority = pAct->m_nPriority ;
		m_nInputType = (int)pAct->m_InputType;
    }
    UpdateData(FALSE);
}

int CJMCActionsPage::AddItem(void* p)
{
    int i = m_cActionsList.GetItemCount ();
    PACTION pAct = (PACTION )p;
    LV_ITEM lvi;
    ZeroMemory(&lvi , sizeof(lvi));
    lvi.mask = LVIF_IMAGE | LVIF_TEXT;
    lvi.iItem = i;
    lvi.pszText  = (LPSTR)pAct->m_strLeft.data();
    lvi.iImage = pAct->m_pGroup->m_bGlobal ? 1 : 0 ;
    int ind = m_cActionsList.InsertItem(&lvi);

    lvi.iItem = ind;
    lvi.iSubItem = 1;
    lvi.mask = LVIF_TEXT ;
    lvi.pszText  = (LPSTR)pAct->m_strRight.data();
    m_cActionsList.SetItem (&lvi);

    lvi.iSubItem = 2;
    lvi.mask = LVIF_TEXT ;
    lvi.pszText  = (LPSTR)pAct->m_pGroup->m_strName.data();
    m_cActionsList.SetItem (&lvi);
    m_cActionsList.SetItemData(ind, (DWORD)p);

    char buff[16];
    sprintf(buff, "%d", pAct->m_nPriority );
    lvi.iSubItem = 3;
    lvi.mask = LVIF_TEXT ;
    lvi.pszText  = buff;
    m_cActionsList.SetItem (&lvi);
    m_cActionsList.SetItemData(ind, (DWORD)p);

	lvi.iSubItem = 4;
    lvi.mask = LVIF_TEXT ;
    lvi.pszText  = (LPSTR)act_type_to_str((int)pAct->m_InputType);
    m_cActionsList.SetItem (&lvi);
    m_cActionsList.SetItemData(ind, (DWORD)p);

    return ind;
}


void CJMCActionsPage::OnSelchangeGroups() 
{
    ResetList();
}

void CJMCActionsPage::OnItemchangedActions(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
    if ( !(LVIF_STATE & pNMListView->uChanged) /*|| !(pNMListView->uNewState & LVNI_SELECTED)*/ )
        return;
    SetControls();
	
	*pResult = 0;
}


void CJMCActionsPage::OnChangeText() 
{
    UpdateData();
    int pos = m_cActionsList.GetNextItem(-1, LVNI_SELECTED);
    ASSERT(pos >= 0 );
    PACTION pAct = (PACTION )m_cActionsList.GetItemData(pos);
    ASSERT(pAct);
	SetActionText(pAct, (LPSTR)(LPCSTR)m_strText) ;
    //SetAction((LPSTR)pAct->m_strLeft.data(), (LPSTR)(LPCSTR)m_strText, pAct->m_nPriority , NULL);
    //pAct->m_strRight = m_strText;

	LV_ITEM lvi;
	ZeroMemory(&lvi, sizeof(lvi));
    lvi.iItem = pos;
    lvi.iSubItem = 1;
    lvi.mask = LVIF_TEXT ;
    lvi.pszText  = (LPSTR)(LPCSTR)m_strText;
    m_cActionsList.SetItem (&lvi);
}

void CJMCActionsPage::OnSelchangeGrp() 
{
	PCGROUP pG = (PCGROUP)m_cGroup.GetItemData (m_cGroup.GetCurSel() ) ;
    ASSERT(pG);

    int pos = m_cActionsList.GetNextItem(-1, LVNI_SELECTED);
    ASSERT(pos >= 0 );
    PACTION  pAct = (PACTION)m_cActionsList.GetItemData(pos);
    ASSERT(pAct);

    pAct->m_pGroup = pG;

    LV_ITEM lvi;
    ZeroMemory(&lvi , sizeof(lvi));
    lvi.iItem = pos;
    lvi.iSubItem = 2;
    lvi.mask = LVIF_TEXT ;
    lvi.pszText  = (LPSTR)pG->m_strName.data();
    m_cActionsList.SetItem (&lvi);

    lvi.iSubItem = 0;
    lvi.mask = LVIF_IMAGE;
    lvi.iImage = pAct->m_pGroup->m_bGlobal ? 1 : 0;
    m_cActionsList.SetItem (&lvi);
}

void CJMCActionsPage::OnAdd() 
{
	GetDlgItem(IDC_NAME)->EnableWindow(TRUE);
	GetDlgItem(IDC_TEXT)->EnableWindow(TRUE);
	GetDlgItem(IDC_GRP)->EnableWindow(TRUE);
    m_strName.Empty ();
    m_strText.Empty();
    PCGROUP pGrp = GetGroup ("default");
    m_cGroup.SelectGroup (pGrp);
    UpdateData(FALSE);
    m_bNewItem = TRUE;
    GetDlgItem(IDC_NAME)->SetFocus();
}

void CJMCActionsPage::OnRemove() 
{
    int pos = m_cActionsList.GetNextItem(-1, LVNI_SELECTED);
    ASSERT(pos >= 0 );
    PACTION pAct = (PACTION)m_cActionsList.GetItemData(pos);
    ASSERT(pAct);
	
    //RemoveAction((LPSTR)pAct->m_strLeft.data());
	if(RemoveAction(pAct)){
		m_cActionsList.DeleteItem (pos);
		m_cActionsList.SetItemState(min(pos, m_cActionsList.GetItemCount () -1),
            LVNI_SELECTED | LVNI_FOCUSED ,  LVNI_SELECTED | LVNI_FOCUSED);
	}else{
		MessageBox("Error deleting action!." , "JMC" , MB_OK | MB_ICONSTOP);
	}
	SetControls();
}

void CJMCActionsPage::OnKillfocusName() 
{
    if ( m_bNewItem ) {
        UpdateData();
        if ( m_strName.GetLength () == 0 ) {
            MessageBeep(MB_OK);
            SetControls();
            return;
        }
/*        if ( GetAction((LPSTR)(LPCSTR)m_strName) ) {
            MessageBox("Action with same name already exist." , "JMC" , MB_OK | MB_ICONSTOP);
            SetControls();
            return;
        }
*/
        PACTION pAct = SetAction((ACTION::ActionType)m_nInputType, (LPSTR)(LPCSTR)m_strName, "", 5, NULL );
        if ( !pAct ) 
            return;
        int i = AddItem(pAct);
        int sel = m_cActionsList.GetNextItem(-1, LVNI_SELECTED);
        m_cActionsList.SetItemState(i,LVNI_SELECTED | LVNI_FOCUSED ,  LVNI_SELECTED | LVNI_FOCUSED  );
        m_cActionsList.EnsureVisible(i, TRUE);
        SetControls();
    }
}

void CJMCActionsPage::GroupListChanged()
{
    m_cGroupList.Reinit();
    m_cGroup.Reinit (FALSE);

    // init columns
	ResetList();
    SetControls();
}

void CJMCActionsPage::OnSelchangePriority() 
{
    UpdateData();
    int pos = m_cActionsList.GetNextItem(-1, LVNI_SELECTED);
    ASSERT(pos >= 0 );
    PACTION pAct = (PACTION )m_cActionsList.GetItemData(pos);
    ASSERT(pAct);
    pAct->m_nPriority = m_nPriority;
    // SetAction((LPSTR)pAct->m_strLeft.data(), (LPSTR)(LPCSTR)m_strText, m_nPriority, NULL);

    char buff[32];
    LV_ITEM lvi;
    ZeroMemory(&lvi , sizeof(lvi));
    lvi.iItem = pos;
    lvi.iSubItem = 3;
    lvi.mask = LVIF_TEXT ;
    lvi.pszText  = itoa(m_nPriority, buff, 10);
    m_cActionsList.SetItem (&lvi);
}

void CJMCActionsPage::OnSelchangeInputType() 
{
    UpdateData();
    int pos = m_cActionsList.GetNextItem(-1, LVNI_SELECTED);
    ASSERT(pos >= 0 );
    PACTION pAct = (PACTION )m_cActionsList.GetItemData(pos);
    ASSERT(pAct);
	pAct->m_InputType = (ACTION::ActionType)m_nInputType;
    // SetAction((LPSTR)pAct->m_strLeft.data(), (LPSTR)(LPCSTR)m_strText, m_nPriority, NULL);

    LV_ITEM lvi;
    ZeroMemory(&lvi , sizeof(lvi));
    lvi.iItem = pos;
	lvi.iSubItem = 4;
    lvi.mask = LVIF_TEXT ;
    lvi.pszText  = (LPSTR)act_type_to_str((int)pAct->m_InputType);
    m_cActionsList.SetItem (&lvi);
}

void CJMCActionsPage::OnChangeName() 
{
    if ( m_bNewItem ) 
        return;

    UpdateData();
    int pos = m_cActionsList.GetNextItem(-1, LVNI_SELECTED);
    ASSERT(pos >= 0 );
    if ( pos < 0 ) 
        return;
    PACTION pAct = (PACTION )m_cActionsList.GetItemData(pos);
    ASSERT(pAct);
    
    SetActionPattern(pAct, m_strName);

    LV_ITEM lvi;
    ZeroMemory(&lvi , sizeof(lvi));
    lvi.iItem = pos;
    lvi.iSubItem = 0;
    lvi.mask = LVIF_TEXT ;
    lvi.pszText  = (LPSTR)(LPCSTR)m_strName;
    m_cActionsList.SetItem (&lvi);
}
