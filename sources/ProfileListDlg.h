#if !defined(AFX_PROFILELISTDLG_H__21606002_0955_11D2_8A74_444553540000__INCLUDED_)
#define AFX_PROFILELISTDLG_H__21606002_0955_11D2_8A74_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ProfileListDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProfileListDlg dialog

class CProfileListDlg : public CDialog
{
// Construction
public:
	CProfileListDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CProfileListDlg)
	enum { IDD = IDD_PROFILE_LIST };
	CListBox	m_cList;
	CString	m_strProfile;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProfileListDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CProfileListDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROFILELISTDLG_H__21606002_0955_11D2_8A74_444553540000__INCLUDED_)
