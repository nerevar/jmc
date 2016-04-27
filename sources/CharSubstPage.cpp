// CharSubstPage.cpp : implementation file
//

#include "stdafx.h"
#include "smc.h"
#include "CharSubstPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCharSubstPage dialog

IMPLEMENT_DYNCREATE(CCharSubstPage, CPropertyPage)

CCharSubstPage::CCharSubstPage()
	: CPropertyPage(CCharSubstPage::IDD, IDS_SUBST_PARAMS_CAPTION)
{

    memset (m_charsSubst, 0 , sizeof(m_charsSubst));
    //{{AFX_DATA_INIT(CCharSubstPage)
	m_bAllowSubst = FALSE;
	m_strNewChar = _T("");
	m_strOldChar = _T("");
	m_nReciveSingle = -1;
	m_nSendSingle = -1;
	//}}AFX_DATA_INIT
}

CCharSubstPage::~CCharSubstPage()
{
}

void CCharSubstPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCharSubstPage)
	DDX_Control(pDX, IDC_OLD, m_cOldChar);
	DDX_Control(pDX, IDC_NEW, m_cNewChar);
	DDX_Control(pDX, IDC_SUBST_LIST, m_cSubstList);
	DDX_Check(pDX, IDC_ALLOW_SUBST, m_bAllowSubst);
	DDX_Text(pDX, IDC_NEW, m_strNewChar);
	DDX_Text(pDX, IDC_OLD, m_strOldChar);
	DDX_Radio(pDX, IDC_RECIVE_SINGLE, m_nReciveSingle);
	DDX_Radio(pDX, IDC_SEND_SINGLE, m_nSendSingle);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCharSubstPage, CPropertyPage)
	//{{AFX_MSG_MAP(CCharSubstPage)
	ON_LBN_SELCHANGE(IDC_SUBST_LIST, OnSelchangeSubstList)
	ON_BN_CLICKED(IDC_ALLOW_SUBST, OnAllowSubst)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCharSubstPage message handlers

void CCharSubstPage::OnSelchangeSubstList() 
{
    InitControls();
}

void CCharSubstPage::OnAllowSubst() 
{
    EnableControls();	
}

void CCharSubstPage::OnOK() 
{
    UpdateData();
}


BOOL CCharSubstPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
    // Filling listbox
    for ( int i = 0 ; i <  *((int*)m_charsSubst) ; i++ ) {
        CString str;
        str.Format("'%c' -> '%c'" , m_charsSubst[sizeof(int) + i*2], m_charsSubst[sizeof(int) + i*2+1]);
        m_cSubstList.AddString((LPCSTR)str);
    }
    m_cSubstList.SetCurSel(0);
    InitControls();
    EnableControls();
    
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCharSubstPage::InitControls()
{
    int nPos = m_cSubstList.GetCurSel();
    if ( nPos < 0 ) {
        GetDlgItem(IDC_REMOVE)->EnableWindow(FALSE);
/*        m_strNewChar = '';
        m_strOldChar = '';
*/
        UpdateData(FALSE);
        return;
    }
    else {
        GetDlgItem(IDC_REMOVE)->EnableWindow(TRUE);
        m_strNewChar = m_charsSubst[sizeof(int) + nPos*2+1];
        m_strOldChar = m_charsSubst[sizeof(int) + nPos*2];
        UpdateData(FALSE);
    }
}

void CCharSubstPage::EnableControls()
{
    UpdateData(TRUE);
    if ( m_bAllowSubst ) {
        GetDlgItem(IDC_SUBST_LIST)->EnableWindow(TRUE);
        GetDlgItem(IDC_OLD)->EnableWindow(TRUE);
        GetDlgItem(IDC_NEW)->EnableWindow(TRUE);
        GetDlgItem(IDC_ADD)->EnableWindow(TRUE);
        GetDlgItem(IDC_REMOVE)->EnableWindow(TRUE);
    }
    else {
        GetDlgItem(IDC_SUBST_LIST)->EnableWindow(FALSE);
        GetDlgItem(IDC_OLD)->EnableWindow(FALSE);
        GetDlgItem(IDC_NEW)->EnableWindow(FALSE);
        GetDlgItem(IDC_ADD)->EnableWindow(FALSE);
        GetDlgItem(IDC_REMOVE)->EnableWindow(FALSE);
    }
}


void CCharSubstPage::OnRemove() 
{
    UpdateData();
    int nPos = m_cSubstList.GetCurSel();

    if ( nPos < *((int*)m_charsSubst) -1 ) { // move tail of array to left 
        for ( int i = nPos +1 ; i < *((int*)m_charsSubst) ; i++ ) {
            m_charsSubst[sizeof(int) + i*2-2] = m_charsSubst[sizeof(int) + i*2];
            m_charsSubst[sizeof(int) + i*2-1] = m_charsSubst[sizeof(int) + i*2+1];
        }
    }
    
    ASSERT(nPos >= 0 );
    m_cSubstList.DeleteString(nPos);
    int count = m_cSubstList.GetCount();

    if ( nPos < count ) 
        m_cSubstList.SetCurSel(nPos);
    else 
        m_cSubstList.SetCurSel(count-1);
    *((int*)m_charsSubst) = *((int*)m_charsSubst)-1;
    InitControls();
}

void CCharSubstPage::OnAdd() 
{
    UpdateData();
    if ( m_strOldChar.GetLength() < 1 ) {
        GetDlgItem(IDC_OLD)->SetFocus();
        return;
    }
    if ( m_strNewChar.GetLength() < 1 ) {
        GetDlgItem(IDC_NEW)->SetFocus();
        return;
    }

    int num = *((int*)m_charsSubst);
    if (num >= (SUBST_ARRAY_SIZE - sizeof(int))/2 ) {
        CString t1,t2;
        t1.LoadString(IDS_SP_ERR_MAX);
        t2.LoadString(IDS_SP_ERROR);
        MessageBox(t1 , t2 , MB_OK | MB_ICONSTOP);
        return;
    }

	int i;
    for ( i = 0 ; i <  *((int*)m_charsSubst) ; i++ ) {
        if ( m_charsSubst[sizeof(int) + i*2] == m_strOldChar[0] && 
            m_charsSubst[sizeof(int) + i*2+1] == m_strNewChar[0] ) {
            CString t1,t2;
            t1.LoadString(IDS_SP_ERR_EXIST);
            t2.LoadString(IDS_SP_ERROR);
            MessageBox(t1 , t2 , MB_OK | MB_ICONSTOP);
            m_cSubstList.SetCurSel(i);
            InitControls();
            return;
        }
    }
    
    m_charsSubst[sizeof(int) + i*2] = m_strOldChar[0] ;
    m_charsSubst[sizeof(int) + i*2+1] = m_strNewChar[0];
    CString str;
    str.Format("'%c' -> '%c'" , m_strOldChar[0], m_strNewChar[0]);
    m_cSubstList.AddString((LPCSTR)str);
    num++;
    *((int*)m_charsSubst) = num;
    m_cSubstList.SetCurSel(num-1);
    InitControls();
}
