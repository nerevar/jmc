#if !defined(AFX_SCRIPTPARSEDLG_H__DDF7822C_8F80_11D4_92AE_0001027ED904__INCLUDED_)
#define AFX_SCRIPTPARSEDLG_H__DDF7822C_8F80_11D4_92AE_0001027ED904__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ScriptParseDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CScriptParseDlg dialog

class CScriptParseDlg : public CDialog
{
// Construction
public:
	CScriptParseDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CScriptParseDlg)
	enum { IDD = IDD_SCRIPT_PARSE };
	CButton	m_cParse;
	CButton	m_cClose;
	CButton	m_cNext;
	CButton	m_cPrev;
	CEdit	m_cText;
	CString	m_strText;
	//}}AFX_DATA

    BOOL m_bDoParseScriptlet;
    HANDLE m_hParseDoneEvent;

    CStringList m_lstScriptlets;
    int m_nScriptletIndex;
    void SetPNKeys();


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScriptParseDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

    int m_nBtnCX,m_nBtnCY,  m_nCXShift, m_nCYShift;


	// Generated message map functions
	//{{AFX_MSG(CScriptParseDlg)
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnNext();
	afx_msg void OnPrev();
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCRIPTPARSEDLG_H__DDF7822C_8F80_11D4_92AE_0001027ED904__INCLUDED_)
