#if !defined(AFX_KEYWORDSDLG_H__6F621076_984A_11D4_92B2_0001027ED904__INCLUDED_)
#define AFX_KEYWORDSDLG_H__6F621076_984A_11D4_92B2_0001027ED904__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// KeyWordsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CKeyWordsDlg dialog

class CKeyWordsDlg : public CDialog
{
// Construction
public:
	CKeyWordsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CKeyWordsDlg)
	enum { IDD = IDD_KEYWORDS_DLG };
	CString	m_strKeys;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKeyWordsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CKeyWordsDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_KEYWORDSDLG_H__6F621076_984A_11D4_92B2_0001027ED904__INCLUDED_)
