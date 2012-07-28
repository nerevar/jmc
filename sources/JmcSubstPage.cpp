//vls-begin// subst page
// JmcSubstPage.cpp : implementation file
//

#include "stdafx.h"
#include "smc.h"
#include "JmcSubstPage.h"
#include "ttcoreex/tintinx.h"
#include "ttcoreex/ttobjects.h"
#include "smcdoc.h"
#include "smcview.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CJmcSubstPage property page

IMPLEMENT_DYNCREATE(CJmcSubstPage, CPropertyPage)

CJmcSubstPage::CJmcSubstPage() : CPropertyPage(CJmcSubstPage::IDD, IDS_SUBST_PAGE_CAPTION)
{
	//{{AFX_DATA_INIT(CJmcSubstPage)
	m_strName = _T("");
	m_strText = _T("");
	//}}AFX_DATA_INIT

    m_bNewItem = FALSE;
}

CJmcSubstPage::~CJmcSubstPage()
{
}

void CJmcSubstPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CJmcSubstPage)
	DDX_Control(pDX, IDC_ALIAS, m_cSubstList);
	DDX_Text(pDX, IDC_NAME, m_strName);
	DDX_Text(pDX, IDC_TEXT, m_strText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CJmcSubstPage, CPropertyPage)
	//{{AFX_MSG_MAP(CJmcSubstPage)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_EN_CHANGE(IDC_TEXT, OnChangeText)
	ON_EN_KILLFOCUS(IDC_NAME, OnKillfocusName)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_ALIAS, OnItemchangedAlias)
	ON_EN_CHANGE(IDC_NAME, OnChangeName)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CJmcSubstPage message handlers

BOOL CJmcSubstPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
 
    DWORD Exstyle = ListView_GetExtendedListViewStyle(m_cSubstList.GetSafeHwnd());
    Exstyle += LVS_EX_FULLROWSELECT;
    ListView_SetExtendedListViewStyleEx(m_cSubstList.GetSafeHwnd () , 0 , Exstyle);

    // init list here 
    CString t;
    t.LoadString(IDS_TP_PATTERN_COL);
    m_cSubstList.InsertColumn (0 , t , LVCFMT_LEFT , 200 );
    t.LoadString(IDS_AP_TEXT_COL);
    m_cSubstList.InsertColumn (1 , t , LVCFMT_LEFT , 200 );

    int size;
    GetSubstList(&size);
//* en: bugfix    
//	for (int i = 0; i < size; i++)
	for (int i = 0; i <=size; i++)
//*/en
        AddItem(GetSubst(i));

    // init columns
    SetControls();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CJmcSubstPage::SetControls()
{
    int pos = m_cSubstList.GetNextItem(-1, LVNI_SELECTED);
    if ( pos < 0 ) {
        m_strName = "";
        m_strText = "";
        GetDlgItem(IDC_NAME)->EnableWindow(FALSE);
        GetDlgItem(IDC_TEXT)->EnableWindow(FALSE);
        GetDlgItem(IDC_REMOVE)->EnableWindow(FALSE);
    } else {
        GetDlgItem(IDC_NAME)->EnableWindow(TRUE);
        GetDlgItem(IDC_TEXT)->EnableWindow(TRUE);
        GetDlgItem(IDC_REMOVE)->EnableWindow(TRUE);
        LPSTR pSubst = (LPSTR)m_cSubstList.GetItemData(pos);
        if ( pSubst ) {
            m_strName = pSubst;
            m_strText = GetSubstText(pSubst);
        } else {
            m_strName.Empty ();
            m_strText.Empty ();
        }
    }
    UpdateData(FALSE);
}

int CJmcSubstPage::AddItem(void* p)
{
    int ind;
    int i = m_cSubstList.GetItemCount ();
    LPSTR pSubst = (LPSTR)p;
    LV_ITEM lvi;
    ZeroMemory(&lvi , sizeof(lvi));
    lvi.mask = LVIF_TEXT;
    lvi.iItem = i;
    lvi.pszText  = pSubst;
    ind = m_cSubstList.InsertItem(&lvi);

    lvi.iItem = ind;
    lvi.iSubItem = 1;
    lvi.mask = LVIF_TEXT ;
    lvi.pszText  = (LPSTR)GetSubstText(pSubst);
    m_cSubstList.SetItem (&lvi);
    m_cSubstList.SetItemData(ind, (DWORD)pSubst);
    return ind;
}

void CJmcSubstPage::OnAdd() 
{
	GetDlgItem(IDC_NAME)->EnableWindow(TRUE);
	GetDlgItem(IDC_TEXT)->EnableWindow(TRUE);
    m_strName.Empty();
    m_strText.Empty();

    UpdateData(FALSE);
    m_bNewItem = TRUE;
    GetDlgItem(IDC_NAME)->SetFocus();
}

void CJmcSubstPage::OnRemove() 
{
    int pos = m_cSubstList.GetNextItem(-1, LVNI_SELECTED);
    ASSERT(pos >= 0 );
    LPSTR pSubst = (LPSTR)m_cSubstList.GetItemData(pos);
    ASSERT(pSubst);
	
    RemoveSubst(pSubst);
    m_cSubstList.DeleteItem (pos);
    m_cSubstList.SetItemState(min(pos, m_cSubstList.GetItemCount () -1),
            LVNI_SELECTED | LVNI_FOCUSED ,  LVNI_SELECTED | LVNI_FOCUSED);
    SetControls();
}

void CJmcSubstPage::OnChangeText() 
{
	UpdateData();
    int pos = m_cSubstList.GetNextItem(-1, LVNI_SELECTED);
    ASSERT(pos >= 0 );
    LPSTR pSubst = (LPSTR)m_cSubstList.GetItemData(pos);
    ASSERT(pSubst);

    SetSubst((LPSTR)(LPCSTR)m_strText, (LPSTR)(LPCSTR)m_strName);

    LV_ITEM lvi;
    ZeroMemory(&lvi , sizeof(lvi));
    lvi.iItem = pos;
    lvi.iSubItem = 1;
    lvi.mask = LVIF_TEXT ;
    lvi.pszText = (LPSTR)(LPCSTR)m_strText;
    m_cSubstList.SetItem (&lvi);
}

void CJmcSubstPage::OnKillfocusName() 
{
    if ( m_bNewItem ) {
        UpdateData();
        m_bNewItem = FALSE;
        if ( m_strName.GetLength () == 0 ) {
            MessageBeep(MB_OK);
            SetControls();
            return;
        }
        if ( GetSubstText((LPSTR)(LPCSTR)m_strName) ) {
            CString t;
            t.LoadString(IDS_SP_ERR_EXIST);
            MessageBox(t, ::AfxGetAppName() , MB_OK | MB_ICONSTOP);
            SetControls();
            return;
        }
        LPSTR pSubst = SetSubst("",(LPSTR)(LPCSTR)m_strName);
        if ( !pSubst ) 
            return;
        int i = AddItem(pSubst);
        int sel = m_cSubstList.GetNextItem(-1, LVNI_SELECTED);
        m_cSubstList.SetItemState(i,LVNI_SELECTED | LVNI_FOCUSED ,  LVNI_SELECTED | LVNI_FOCUSED  );
        m_cSubstList.EnsureVisible(i, TRUE);
        SetControls();
    } 
}

void CJmcSubstPage::OnItemchangedAlias(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
    if ( !(LVIF_STATE & pNMListView->uChanged) /*|| !(pNMListView->uNewState & LVNI_SELECTED)*/ )
        return;
    SetControls();
	
	*pResult = 0;
}

void CJmcSubstPage::OnChangeName() 
{
    if ( m_bNewItem ) 
        return;

    UpdateData();
    int pos = m_cSubstList.GetNextItem(-1, LVNI_SELECTED);
    ASSERT(pos >= 0 );
    if ( pos < 0 ) 
        return;
    LPSTR pSubst = (LPSTR)m_cSubstList.GetItemData(pos);
    ASSERT(pSubst);

    SetSubstPattern(pSubst, (LPSTR)(LPCSTR)m_strName);

    LV_ITEM lvi;
    ZeroMemory(&lvi , sizeof(lvi));
    lvi.iItem = pos;
    lvi.iSubItem = 0;
    lvi.mask = LVIF_TEXT ;
    lvi.pszText  = (LPSTR)(LPCSTR)m_strName;
    m_cSubstList.SetItem (&lvi);
}

//vls-end//
