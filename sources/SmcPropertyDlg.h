#if !defined(AFX_SMCPROPERTYDLG_H__16273183_091B_11D2_8A74_444553540000__INCLUDED_)
#define AFX_SMCPROPERTYDLG_H__16273183_091B_11D2_8A74_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SmcPropertyDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSmcPropertyDlg

class CSmcPropertyDlg : public CPropertySheet
{
	DECLARE_DYNAMIC(CSmcPropertyDlg)

// Construction
public:
	CSmcPropertyDlg(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CSmcPropertyDlg(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSmcPropertyDlg)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSmcPropertyDlg();

	// Generated message map functions
protected:
	//{{AFX_MSG(CSmcPropertyDlg)
	afx_msg LRESULT HandleInitDialog(WPARAM, LPARAM);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SMCPROPERTYDLG_H__16273183_091B_11D2_8A74_444553540000__INCLUDED_)
