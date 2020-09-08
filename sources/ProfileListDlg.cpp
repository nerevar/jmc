// ProfileListDlg.cpp : implementation file
//

#include "stdafx.h"
#include "smc.h"
#include "ProfileListDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProfileListDlg dialog


CProfileListDlg::CProfileListDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProfileListDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProfileListDlg)
	m_strProfile = _T("");
	//}}AFX_DATA_INIT
}


void CProfileListDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProfileListDlg)
	DDX_Control(pDX, IDC_LIST, m_cList);
	DDX_LBString(pDX, IDC_LIST, m_strProfile);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProfileListDlg, CDialog)
	//{{AFX_MSG_MAP(CProfileListDlg)
	ON_LBN_DBLCLK(IDC_LIST, OnDblclkList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProfileListDlg message handlers

BOOL CProfileListDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CenterWindow();
	// Fill listbox
    WIN32_FIND_DATA fd;
    HANDLE hFind;
    BOOL ret = TRUE;

//vls-begin// base dir
//    hFind = FindFirstFile(".\\settings\\*.opt" , &fd);
    CString strFileMask(szSETTINGS_DIR);
    strFileMask += "\\*.opt";
    hFind = FindFirstFile(strFileMask, &fd);
//vls-end//
    if ( hFind == INVALID_HANDLE_VALUE ) {
        return FALSE;
    }
    while ( ret ) {

        if ( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
            ret = FindNextFile(hFind, &fd);
            continue;
        }

		wchar_t Name[MAX_PATH];
		wchar_t* out = Name;
		wchar_t* ptr = fd.cFileName;

		while ( *ptr && *ptr != L'.' )
			*out++ = *ptr++;
		*out = 0;
		
        m_cList.InsertString(-1, Name);
        ret = FindNextFile(hFind, &fd);
    }
    FindClose(hFind);

	m_cList.SetCurSel(0);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CProfileListDlg::OnDblclkList() 
{
    OnOK();	
}
