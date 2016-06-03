// JmcHlightPage.cpp : implementation file
//

#include "stdafx.h"
#include "smc.h"
#include "JmcHlightPage.h"
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
// CJmcHlightPage property page

IMPLEMENT_DYNCREATE(CJmcHlightPage, CPropertyPage)

CJmcHlightPage::CJmcHlightPage() : CGroupedPage(CJmcHlightPage::IDD, IDS_HLIGHT_PAGE_CAPTION)
{
	//{{AFX_DATA_INIT(CJmcHlightPage)
	m_strName = _T("");
	m_strGroup = _T("");
	m_nBackColor = -1;
	m_nForeColor = -1;
	//}}AFX_DATA_INIT
}

CJmcHlightPage::~CJmcHlightPage()
{
}

void CJmcHlightPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CJmcHlightPage)
	DDX_Control(pDX, IDC_FORE_COLOR, m_cForeColor);
	DDX_Control(pDX, IDC_BACK_COLOR, m_cBackColor);
	DDX_Control(pDX, IDC_GRP, m_cGroup);
	DDX_Control(pDX, IDC_GROUPS, m_cGroupList);
	DDX_Control(pDX, IDC_ALIAS, m_cHlightList);
	DDX_Text(pDX, IDC_NAME, m_strName);
	DDX_CBString(pDX, IDC_GRP, m_strGroup);
	DDX_CBIndex(pDX, IDC_BACK_COLOR, m_nBackColor);
	DDX_CBIndex(pDX, IDC_FORE_COLOR, m_nForeColor);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CJmcHlightPage, CPropertyPage)
	//{{AFX_MSG_MAP(CJmcHlightPage)
	ON_CBN_SELCHANGE(IDC_GROUPS, OnSelchangeGroups)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_ALIAS, OnItemchangedHlight)
	ON_CBN_SELCHANGE(IDC_GRP, OnSelchangeGrp)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_EN_KILLFOCUS(IDC_NAME, OnKillfocusName)
	ON_WM_DRAWITEM()
	ON_CBN_SELCHANGE(IDC_BACK_COLOR, OnSelchangeColor)
	ON_CBN_SELCHANGE(IDC_FORE_COLOR, OnSelchangeColor)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CJmcHlightPage message handlers

BOOL CJmcHlightPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
    AddPage(L"hlight", this);

    m_ImageList.Create(IDB_GROUP_ICONS, 16 , 2, (COLORREF)0xFFFFFF);

    DWORD Exstyle = ListView_GetExtendedListViewStyle(m_cHlightList.GetSafeHwnd());
    Exstyle += LVS_EX_FULLROWSELECT;
    ListView_SetExtendedListViewStyleEx(m_cHlightList.GetSafeHwnd () , 0 , Exstyle);

    // init list here 
    CString t;
    t.LoadString(IDS_TP_PATTERN_COL);
    m_cHlightList.InsertColumn (0 , t , LVCFMT_LEFT , 300 );
    t.LoadString(IDS_AP_GROUP_COL);
    m_cHlightList.InsertColumn (1 , t , LVCFMT_LEFT , 100 );

    m_cHlightList.SetImageList(&m_ImageList, LVSIL_SMALL);
    

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

void CJmcHlightPage::ResetList()
{
    m_cHlightList.DeleteAllItems ();

    int ind = m_cGroupList.GetCurSel ();
    ASSERT(ind >= 0 );
    PCGROUP pGrp = (PCGROUP)m_cGroupList.GetItemData (ind);

    int Count ;
    PPHLIGHT ppHlight = GetHlightList (&Count);
    for ( int i = 0 ; i < Count ; i++ ) {
        if ( !pGrp || (ppHlight[i]->m_pGroup == pGrp) ){
            AddItem(ppHlight[i]);
        }
    }
    SetControls();
}

void CJmcHlightPage::SetControls()
{
    int pos = m_cHlightList.GetNextItem(-1, LVNI_SELECTED);
    if ( pos < 0 ) {
        m_strName = "";
        GetDlgItem(IDC_NAME)->EnableWindow(FALSE);
        GetDlgItem(IDC_REMOVE)->EnableWindow(FALSE);
        GetDlgItem(IDC_FORE_COLOR)->EnableWindow(FALSE);
        GetDlgItem(IDC_BACK_COLOR)->EnableWindow(FALSE);
        m_cGroup.EnableWindow (FALSE);
    } else {
        GetDlgItem(IDC_NAME)->EnableWindow(FALSE);
        GetDlgItem(IDC_REMOVE)->EnableWindow(TRUE);
        GetDlgItem(IDC_FORE_COLOR)->EnableWindow(TRUE);
        GetDlgItem(IDC_BACK_COLOR)->EnableWindow(TRUE);
        m_cGroup.EnableWindow (TRUE);
        PHLIGHT pHlight = (PHLIGHT)m_cHlightList.GetItemData(pos);
        m_strName = pHlight->m_strPattern.data();
        m_cGroup.SelectGroup (pHlight->m_pGroup );
        // now set up colors. scan ANSI string for it
        const wchar_t* p = pHlight->m_strAnsi.c_str();
        p+=2;
        BOOL bBold = FALSE;
        m_nForeColor  = 7;
        m_nBackColor= 0;

        while ( *p && *p != L'm' ) {
            wchar_t buff[16] = L"";
            wchar_t* p1 = buff;
            while ( iswdigit(*p ) ) {
                *p1++ = *p++;
            }
            *p1 = 0;
            if ( *buff ) {
                int val = _wtoi(buff);
                if ( val == 0 ) 
                    bBold = FALSE;
                   else if ( val == 1 ) 
                       bBold = TRUE;
                   else if ( val <=37 && val >= 30 ) 
                       m_nForeColor = val-30;
                   else if ( val <= 47 && val >= 40 ) 
                       m_nBackColor = val-40;
            }
            if ( *p ) 
                p++;
        }
        if ( bBold ) {
            m_nForeColor += 8;
            m_nBackColor += 8;
        }



    }
    UpdateData(FALSE);
}

int CJmcHlightPage::AddItem(void* p)
{
    int i = m_cHlightList.GetItemCount ();
    PHLIGHT pHlight = (PHLIGHT)p;
    LV_ITEM lvi;
    ZeroMemory(&lvi , sizeof(lvi));
    lvi.mask = LVIF_IMAGE | LVIF_TEXT;
    lvi.iItem = i;
    lvi.pszText  = (LPWSTR)pHlight->m_strPattern.c_str();
    lvi.iImage = pHlight->m_pGroup->m_bGlobal ? 1 : 0 ;
    int ind = m_cHlightList.InsertItem(&lvi);

    lvi.iItem = ind;
    lvi.iSubItem = 1;
    lvi.mask = LVIF_TEXT ;
    lvi.pszText  = (LPWSTR)pHlight->m_pGroup->m_strName.c_str();
    m_cHlightList.SetItem (&lvi);
    m_cHlightList.SetItemData(ind, (DWORD)p);
    
    return ind;
}


void CJmcHlightPage::OnSelchangeGroups() 
{
    ResetList();
}

void CJmcHlightPage::OnItemchangedHlight(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
    if ( !(LVIF_STATE & pNMListView->uChanged) /*|| !(pNMListView->uNewState & LVNI_SELECTED) */)
        return;
    SetControls();
	
	*pResult = 0;
}


void CJmcHlightPage::OnSelchangeGrp() 
{
	PCGROUP pG = (PCGROUP)m_cGroup.GetItemData (m_cGroup.GetCurSel() ) ;
    ASSERT(pG);

    int pos = m_cHlightList.GetNextItem(-1, LVNI_SELECTED);
    ASSERT(pos >= 0 );
    PHLIGHT pAl = (PHLIGHT)m_cHlightList.GetItemData(pos);
    ASSERT(pAl);

    pAl->m_pGroup = pG;

    LV_ITEM lvi;
    ZeroMemory(&lvi , sizeof(lvi));
    lvi.iItem = pos;
    lvi.iSubItem = 1;
    lvi.mask = LVIF_TEXT ;
    lvi.pszText  = (LPWSTR)pG->m_strName.c_str();
    m_cHlightList.SetItem (&lvi);

    lvi.iSubItem = 0;
    lvi.mask = LVIF_IMAGE;
    lvi.iImage = pAl->m_pGroup->m_bGlobal ? 1 : 0;
    m_cHlightList.SetItem (&lvi);
}

void CJmcHlightPage::OnAdd() 
{
	GetDlgItem(IDC_NAME)->EnableWindow(TRUE);
	GetDlgItem(IDC_GRP)->EnableWindow(TRUE);
    GetDlgItem(IDC_FORE_COLOR)->EnableWindow(TRUE);
    GetDlgItem(IDC_BACK_COLOR)->EnableWindow(TRUE);
    m_strName.Empty ();
    PCGROUP pGrp = GetGroup (L"default");
    m_cGroup.SelectGroup (pGrp);

    m_nBackColor = 0;
    m_nForeColor = 7;
    UpdateData(FALSE);
    m_bNewItem = TRUE;
    GetDlgItem(IDC_NAME)->SetFocus();
}

void CJmcHlightPage::OnRemove() 
{
    int pos = m_cHlightList.GetNextItem(-1, LVNI_SELECTED);
    ASSERT(pos >= 0 );
    PHLIGHT pAl = (PHLIGHT)m_cHlightList.GetItemData(pos);
    ASSERT(pAl);
	
    RemoveHlight(pAl->m_strPattern.c_str());
    m_cHlightList.DeleteItem (pos);
    m_cHlightList.SetItemState(min(pos, m_cHlightList.GetItemCount () -1),
            LVNI_SELECTED | LVNI_FOCUSED ,  LVNI_SELECTED | LVNI_FOCUSED);
    SetControls();
}

void CJmcHlightPage::OnKillfocusName() 
{
    if ( m_bNewItem ) {
        UpdateData();
        if ( m_strName.GetLength () == 0 ) {
            MessageBeep(MB_OK);
            SetControls();
            return;
        }
        if ( GetHlight(m_strName) ) {
            CString t;
            t.LoadString(IDS_HP_ERR_EXIST);
            MessageBox(t, ::AfxGetAppName() , MB_OK | MB_ICONSTOP);
            SetControls();
            return;
        }
        PHLIGHT pAl = SetHlight(L"",m_strName,  NULL );
        int i = AddItem(pAl);
        int sel = m_cHlightList.GetNextItem(-1, LVNI_SELECTED);
        m_cHlightList.SetItemState(i,LVNI_SELECTED | LVNI_FOCUSED ,  LVNI_SELECTED | LVNI_FOCUSED  );
        m_cHlightList.EnsureVisible(i, TRUE);
        SetControls();
    }
}

void CJmcHlightPage::GroupListChanged()
{
    m_cGroupList.Reinit();
    m_cGroup.Reinit (FALSE);

    // init columns
	ResetList();
    SetControls();
}

void CJmcHlightPage::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	// CPropertyPage::OnDrawItem(nIDCtl, lpDrawItemStruct);

    if ( nIDCtl == IDC_ALIAS) {
        CDC dc;
        dc.Attach (lpDrawItemStruct->hDC );

        COLORREF OldFg = dc.GetTextColor();
        COLORREF OldBg = dc.GetBkColor ();

        CRect rect=lpDrawItemStruct->rcItem;
        if ( lpDrawItemStruct->itemState & ODS_SELECTED ) {
            CPen ps(PS_SOLID, 1,RGB(0, 0, 255)) ;
            CPen* pOldPen = (CPen*)dc.SelectObject(&ps);
            dc.Rectangle (&rect);
            dc.SelectObject (pOldPen);
            rect.top += 1;
            rect.bottom -= 1;
            rect.left +=1;
            rect.right -= 1;
        } 

        
        CFrameWnd* pFrm = (CFrameWnd*)AfxGetMainWnd();
        CSmcView* pView = (CSmcView*)pFrm->GetActiveView();
        PHLIGHT pHigh = (PHLIGHT)lpDrawItemStruct->itemData ;

        CString str = pHigh->m_strAnsi.data ();
        str += pHigh->m_strPattern.data();

        pView->DrawWithANSI (&dc, rect, &str);

        rect.left = rect.left + m_cHlightList.GetColumnWidth (0)-3;

        str = "\x1B[0m";
        str += pHigh->m_pGroup->m_strName.data();
        pView->DrawWithANSI (&dc, rect, &str);
        dc.SetTextColor (OldFg);
        dc.SetBkColor (OldBg);

        dc.Detach ();
    }

    if ( nIDCtl == IDC_FORE_COLOR ) {
        CDC dc;
        dc.Attach (lpDrawItemStruct->hDC );
        
        dc.Detach ();
    }
}

static char* colorNames[] = {
    "black", 
    "red",
    "green",
    "brown",
    "blue",
    "magenta",
    "cyan",
    "grey",
    "charcoal",
    "light red",
    "light green",
    "yellow",
    "light blue",
    "light magenta",
    "light cyan",
    "white"};

void CJmcHlightPage::OnSelchangeColor() 
{
	UpdateData();
    int pos = m_cHlightList.GetNextItem(-1, LVNI_SELECTED);
    ASSERT(pos >= 0 );
    PHLIGHT pAl = (PHLIGHT)m_cHlightList.GetItemData(pos);
    ASSERT(pAl);

    CString str = colorNames[m_nForeColor];
    str += ",b ";
    str += colorNames[m_nBackColor];
    SetHlight(str, pAl->m_strPattern.c_str(), NULL);

    m_cHlightList.RedrawItems (pos, pos);
}

