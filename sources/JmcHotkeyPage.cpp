// JmcHotkeyPage.cpp : implementation file
//

#include "stdafx.h"
#include "smc.h"
#include "JmcHotkeyPage.h"
#include "ttcoreex/tintinx.h"
#include "ttcoreex/ttobjects.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static int CALLBACK HkeyCompareFunc(LPARAM lParam1, LPARAM lParam2, 
   LPARAM lParamSort)
{
    CHotKey* pHot1 = (CHotKey*)lParam1 ;
    CHotKey* pHot2 = (CHotKey*)lParam2 ;

    return pHot1->m_nScanCode - pHot2->m_nScanCode;
}

/////////////////////////////////////////////////////////////////////////////
// CJmcHotkeyPage property page

IMPLEMENT_DYNCREATE(CJmcHotkeyPage, CPropertyPage)

//vls-begin// grouped hotkeys
//CJmcHotkeyPage::CJmcHotkeyPage() : CPropertyPage(CJmcHotkeyPage::IDD, IDS_HKEY_PAGE_CAPTION)
CJmcHotkeyPage::CJmcHotkeyPage() : CGroupedPage(CJmcHotkeyPage::IDD, IDS_HKEY_PAGE_CAPTION)
//vls-end//
{
	//{{AFX_DATA_INIT(CJmcHotkeyPage)
	m_strText = _T("");
	m_strKey = _T("");
//vls-begin// grouped hotkeys
	m_strGroup = _T("");
//vls-end//
	//}}AFX_DATA_INIT
    m_bNewItem = FALSE;
}

CJmcHotkeyPage::~CJmcHotkeyPage()
{
}

void CJmcHotkeyPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CJmcHotkeyPage)
	DDX_Control(pDX, IDC_ALIAS, m_cHotkeysList);
//vls-begin// grouped hotkeys
	DDX_Control(pDX, IDC_GRP, m_cGroup);
	DDX_Control(pDX, IDC_GROUPS, m_cGroupList);
	DDX_CBString(pDX, IDC_GRP, m_strGroup);
//vls-end//
	DDX_Text(pDX, IDC_TEXT, m_strText);
	DDX_Text(pDX, IDC_KEY, m_strKey);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CJmcHotkeyPage, CPropertyPage)
	//{{AFX_MSG_MAP(CJmcHotkeyPage)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_ALIAS, OnItemchangedAlias)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_EN_CHANGE(IDC_TEXT, OnChangeText)
	ON_EN_KILLFOCUS(IDC_KEY, OnKillfocusKey)
	ON_EN_CHANGE(IDC_KEY, OnChangeKey)
	ON_CBN_SELCHANGE(IDC_GROUPS, OnSelchangeGroups)
	ON_CBN_SELCHANGE(IDC_GRP, OnSelchangeGrp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CJmcHotkeyPage message handlers

BOOL CJmcHotkeyPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
//vls-begin// grouped hotkeys
    AddPage(L"hotkey", this);
	
    m_ImageList.Create(IDB_GROUP_ICONS, 16 , 2, (COLORREF)0xFFFFFF);
//vls-end//

    DWORD Exstyle = ListView_GetExtendedListViewStyle(m_cHotkeysList.GetSafeHwnd());
    Exstyle += LVS_EX_FULLROWSELECT;
    ListView_SetExtendedListViewStyleEx(m_cHotkeysList.GetSafeHwnd () , 0 , Exstyle);

    CString t;
    t.LoadString(IDS_KP_KEY_COL);
    m_cHotkeysList.InsertColumn (0 , t , LVCFMT_LEFT , 100 );
    t.LoadString(IDS_AP_TEXT_COL);
    m_cHotkeysList.InsertColumn (1 , t , LVCFMT_LEFT , 200 );
//vls-begin// grouped hotkeys
    t.LoadString(IDS_AP_GROUP_COL);
    m_cHotkeysList.InsertColumn (2 , t , LVCFMT_LEFT , 100 );
    m_cHotkeysList.SetImageList(&m_ImageList, LVSIL_SMALL);
//vls-end//

    // fill list 
    int size;
    PPCHotKey ppHot = GetHotList(&size);
    for ( int i = 0 ; i < size ; i++ ) {
        AddItem(ppHot[i]);
    }

    m_cHotkeysList.SortItems(HkeyCompareFunc, 0);
    m_wndHotCtrl.SubclassDlgItem(IDC_KEY, this);

//vls-begin// grouped hotkeys
    // init group list 
    m_cGroupList.Reinit();
    m_cGroup.Reinit (FALSE);
	ResetList();
//vls-end//

    SetControls();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CJmcHotkeyPage::OnAdd() 
{
    GetDlgItem(IDC_TEXT)->EnableWindow(FALSE);
    GetDlgItem(IDC_REMOVE)->EnableWindow(FALSE);
//vls-begin// grouped hotkeys
    GetDlgItem(IDC_GRP)->EnableWindow(TRUE);
    PCGROUP pGrp = GetGroup (L"default");
    m_cGroup.SelectGroup (pGrp);
//vls-end//
    m_strKey.Empty ();
    m_strText.Empty();
    UpdateData(FALSE);

    AddItem(NULL);
    m_bNewItem = TRUE;
    m_wndHotCtrl.m_fKeyPressed = FALSE;

    GetDlgItem(IDC_KEY)->SetFocus();
}

void CJmcHotkeyPage::OnItemchangedAlias(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
    if ( !(LVIF_STATE & pNMListView->uChanged) /*|| !(pNMListView->uNewState & LVNI_SELECTED)*/ )
        return;
    SetControls();
	
	*pResult = 0;
}

void CJmcHotkeyPage::OnRemove() 
{
    int pos = m_cHotkeysList.GetNextItem(-1, LVNI_SELECTED);
    if ( pos < 0 ) 
        return;
    CHotKey* pHot = (CHotKey*)m_cHotkeysList.GetItemData (pos);

    RemoveHot(pHot);
    m_cHotkeysList.DeleteItem (pos);
    m_cHotkeysList.SetItemState(min(pos, m_cHotkeysList.GetItemCount () -1),
            LVNI_SELECTED | LVNI_FOCUSED ,  LVNI_SELECTED | LVNI_FOCUSED);
    SetControls();

}

void CJmcHotkeyPage::OnChangeText() 
{
    UpdateData();
    int pos = m_cHotkeysList.GetNextItem(-1, LVNI_SELECTED);
    if ( pos < 0 ) 
        return;
    CHotKey* pHot = (CHotKey*)m_cHotkeysList.GetItemData (pos);
    SetHotText(pHot, m_strText);
	//pHot->m_strAction = m_strText;

    LV_ITEM lvi;
    ZeroMemory(&lvi , sizeof(lvi));
    lvi.mask = LVIF_TEXT;
    lvi.iItem = pos;
    lvi.iSubItem = 1;
    lvi.pszText  = (LPWSTR)(const wchar_t*)m_strText;
    m_cHotkeysList.SetItem (&lvi);

}

void CJmcHotkeyPage::SetControls()
{
    int pos = m_cHotkeysList.GetNextItem(-1, LVNI_SELECTED);
    if ( pos < 0 ) {
        m_strKey = L"";
        m_strText = L"";
        GetDlgItem(IDC_TEXT)->EnableWindow(FALSE);
        GetDlgItem(IDC_REMOVE)->EnableWindow(FALSE);
        GetDlgItem(IDC_KEY)->EnableWindow(FALSE);
//vls-begin// grouped hotkeys
        m_cGroup.EnableWindow(FALSE);
//vls-end//
    } else {
        GetDlgItem(IDC_TEXT)->EnableWindow(TRUE);
        GetDlgItem(IDC_REMOVE)->EnableWindow(TRUE);
        GetDlgItem(IDC_KEY)->EnableWindow(TRUE);
//vls-begin// grouped hotkeys
        m_cGroup.EnableWindow(TRUE);
//vls-end//
        CHotKey* pHot= (CHotKey*)m_cHotkeysList.GetItemData(pos);
        if ( pHot ) {
//vls-begin// grouped hotkeys
            m_cGroup.SelectGroup (pHot->m_pGroup );
//vls-end//
            m_strKey = pHot->m_strKey.data();
            m_strText = pHot->m_strAction.data();
        } else {
            m_strKey.Empty ();
            m_strText.Empty ();
        }
    }
    UpdateData(FALSE);
}

int CJmcHotkeyPage::AddItem(void* p)
{
    int ind;
    if ( p ) { 
        int i = m_cHotkeysList.GetItemCount ();
        CHotKey* pHot = (CHotKey*)p;
        LV_ITEM lvi;
        ZeroMemory(&lvi , sizeof(lvi));
        lvi.mask = LVIF_TEXT;
        lvi.iItem = i;
        lvi.pszText  = (LPWSTR)pHot->m_strKey.c_str();
        ind = m_cHotkeysList.InsertItem(&lvi);

        lvi.iItem = ind;
        lvi.iSubItem = 1;
        lvi.mask = LVIF_TEXT ;
        lvi.pszText  = (LPWSTR)pHot->m_strAction.c_str();
        m_cHotkeysList.SetItem (&lvi);

//vls-begin// grouped hotkeys
        lvi.iItem = ind;
        lvi.iSubItem = 2;
        lvi.mask = LVIF_TEXT ;
        lvi.pszText  = (LPWSTR)pHot->m_pGroup->m_strName.c_str();
        m_cHotkeysList.SetItem (&lvi);
//vls-end//

        m_cHotkeysList.SetItemData(ind, (DWORD)pHot);
    } else {
        int i = m_cHotkeysList.GetItemCount ();
        LV_ITEM lvi;
        ZeroMemory(&lvi , sizeof(lvi));
        lvi.mask = LVIF_TEXT;
        lvi.iItem = i;
        CString t;
        t.LoadString(IDS_KP_NEW);
        lvi.pszText  = t.GetBuffer(256);
        ind = m_cHotkeysList.InsertItem(&lvi);
        m_cHotkeysList.SetItemData(ind, (DWORD)p);
        m_cHotkeysList.SetItemState(ind,
                LVNI_SELECTED | LVNI_FOCUSED ,  LVNI_SELECTED | LVNI_FOCUSED);
        m_cHotkeysList.EnsureVisible (ind , TRUE);
    }

    return ind;
}



void CJmcHotkeyPage::OnKillfocusKey() 
{
    if ( m_bNewItem ) {
        if ( !m_wndHotCtrl.m_fKeyPressed ) {
            int i = m_cHotkeysList.GetItemCount ();
            ASSERT(m_cHotkeysList.GetItemData(i-1) == NULL );
            m_cHotkeysList.DeleteItem (i -1);
            SetControls();
        }
        m_bNewItem = FALSE;
    } 
}

void CJmcHotkeyPage::OnChangeKey() 
{
    UpdateData();
    if ( m_bNewItem ) {
        m_bNewItem = FALSE;
        int i = m_cHotkeysList.GetItemCount ();
        ASSERT(m_cHotkeysList.GetItemData(i-1) == NULL );
        // now check for this hotkey already exist 
        CHotKey* pHot = GetHot (m_wndHotCtrl.m_key.scan , m_wndHotCtrl.m_key.uFlags );
        if ( !pHot ) {
            // change curtrent item to the new hotkey 
//vls-begin// grouped hotkeys
//            pHot = SetHot (m_wndHotCtrl.m_key.scan , m_wndHotCtrl.m_key.uFlags, 
//                (LPSTR)(LPCSTR)m_strKey , "" );
            pHot = SetHot (m_wndHotCtrl.m_key.scan , m_wndHotCtrl.m_key.uFlags, 
                m_strKey , L"", NULL);
//vls-end//
            
            LV_ITEM lvi;
            ZeroMemory(&lvi , sizeof(lvi));
            lvi.mask = LVIF_TEXT;
            lvi.iItem = i-1;
            lvi.pszText  =  (LPWSTR)(const wchar_t*)m_strKey ;
            int ind = m_cHotkeysList.SetItem(&lvi);
            m_cHotkeysList.SetItemData(i-1, (DWORD)pHot);
        } else {
            m_cHotkeysList.DeleteItem (i-1);
            // now find item with same key 
            for ( i = 0 ; i < m_cHotkeysList.GetItemCount () ; i++ ) {
                if ( (CHotKey*)m_cHotkeysList.GetItemData(i) == pHot ) {
                    m_cHotkeysList.SetItemState(i,
                            LVNI_SELECTED | LVNI_FOCUSED ,  LVNI_SELECTED | LVNI_FOCUSED);
                    m_cHotkeysList.EnsureVisible (i , TRUE);
                    break;
                }
            }
        }
    } else { // switch to the new key or 
        int pos = m_cHotkeysList.GetNextItem(-1, LVNI_SELECTED);
        if ( pos < 0 ) {
            ASSERT(0);
            return;
        }
        CHotKey* pHot = (CHotKey*)m_cHotkeysList.GetItemData (pos);
        for ( int i = 0 ; i < m_cHotkeysList.GetItemCount () ; i++ ) {
            CHotKey* pTmpHot = (CHotKey*)m_cHotkeysList.GetItemData (i);
            if ( i != pos && m_wndHotCtrl.m_key.scan == pTmpHot->m_nScanCode && 
                m_wndHotCtrl.m_key.uFlags == pTmpHot->m_nAltState ) { // ok,\, its exist 
                MessageBeep(MB_OK);
                m_strKey = pHot->m_strKey.data ();
                m_wndHotCtrl.m_fKeyPressed = FALSE;
                UpdateData(FALSE);
                return;
            }
        }

        CString strAct = pHot->m_strAction.data(); ;
        RemoveHot(pHot);
//vls-begin// grouped hotkeys
//        pHot = SetHot(m_wndHotCtrl.m_key.scan , m_wndHotCtrl.m_key.uFlags , 
//            (LPSTR)(LPCSTR)m_strKey, (LPSTR)(LPCSTR)strAct);
        pHot = SetHot(m_wndHotCtrl.m_key.scan , m_wndHotCtrl.m_key.uFlags , 
            m_strKey, strAct, NULL);
//vls-end//
        m_cHotkeysList.SetItemData (pos, (DWORD)pHot);
        LV_ITEM lvi;
        ZeroMemory(&lvi , sizeof(lvi));
        lvi.mask = LVIF_TEXT;
        lvi.iItem = pos;
        lvi.pszText  = (LPWSTR)(const wchar_t*)m_strKey ;
        m_cHotkeysList.SetItem(&lvi);
    }
}

//vls-begin// grouped hotkeys
void CJmcHotkeyPage::ResetList()
{
    m_cHotkeysList.DeleteAllItems ();

    int ind = m_cGroupList.GetCurSel ();
    ASSERT(ind >= 0 );
    PCGROUP pGrp = (PCGROUP)m_cGroupList.GetItemData (ind);

    int Count ;
    PPCHotKey ppHot = GetHotList (&Count);
    for ( int i = 0 ; i < Count ; i++ ) {
        if ( !pGrp || (ppHot[i]->m_pGroup == pGrp) ){
            AddItem(ppHot[i]);
        }
    }
    SetControls();
}

void CJmcHotkeyPage::GroupListChanged()
{
    m_cGroupList.Reinit();
    m_cGroup.Reinit (FALSE);

    // init columns
	ResetList();
    SetControls();
}

void CJmcHotkeyPage::OnSelchangeGroups() 
{
    ResetList();
}

void CJmcHotkeyPage::OnSelchangeGrp() 
{
	PCGROUP pG = (PCGROUP)m_cGroup.GetItemData (m_cGroup.GetCurSel() ) ;
    ASSERT(pG);

    int pos = m_cHotkeysList.GetNextItem(-1, LVNI_SELECTED);
    ASSERT(pos >= 0 );
    PCHotKey pHot = (PCHotKey)m_cHotkeysList.GetItemData(pos);
    ASSERT(pHot);

    pHot->m_pGroup = pG;

    LV_ITEM lvi;
    ZeroMemory(&lvi , sizeof(lvi));
    lvi.iItem = pos;
    lvi.iSubItem = 2;
    lvi.mask = LVIF_TEXT ;
    lvi.pszText  = (LPWSTR)pG->m_strName.c_str();
    m_cHotkeysList.SetItem (&lvi);

    lvi.iSubItem = 0;
    lvi.mask = LVIF_IMAGE;
    lvi.iImage = pHot->m_pGroup->m_bGlobal ? 1 : 0;
    m_cHotkeysList.SetItem (&lvi);
}

//vls-end//

