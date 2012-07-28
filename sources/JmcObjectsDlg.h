#if !defined(AFX_JMCOBJECTSDLG_H__F9383E41_EA6B_11D4_87F4_F6F03C29B215__INCLUDED_)
#define AFX_JMCOBJECTSDLG_H__F9383E41_EA6B_11D4_87F4_F6F03C29B215__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// JmcObjectsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CJmcObjectsDlg

class CJmcObjectsDlg : public CPropertySheet
{
	DECLARE_DYNAMIC(CJmcObjectsDlg)

// Construction
public:
	CJmcObjectsDlg(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CJmcObjectsDlg(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJmcObjectsDlg)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CJmcObjectsDlg();

	// Generated message map functions
protected:
	//{{AFX_MSG(CJmcObjectsDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JMCOBJECTSDLG_H__F9383E41_EA6B_11D4_87F4_F6F03C29B215__INCLUDED_)
