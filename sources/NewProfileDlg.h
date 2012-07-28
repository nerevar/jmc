#if !defined(AFX_NEWPROFILEDLG_H__16273182_091B_11D2_8A74_444553540000__INCLUDED_)
#define AFX_NEWPROFILEDLG_H__16273182_091B_11D2_8A74_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// NewProfileDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNewProfileDlg dialog

class CNewProfileDlg : public CDialog
{
// Construction
public:
	CNewProfileDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNewProfileDlg)
	enum { IDD = IDD_NEW_PROFILE };
	CString	m_strCommand;
	CString	m_strName;
	CString	m_strSaveFile;
	CString	m_strStartFile;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewProfileDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNewProfileDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeName();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWPROFILEDLG_H__16273182_091B_11D2_8A74_444553540000__INCLUDED_)
