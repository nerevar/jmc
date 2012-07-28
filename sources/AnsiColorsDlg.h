#if !defined(AFX_ANSICOLORSDLG_H__E7DDE422_0E35_11D2_8A74_444553540000__INCLUDED_)
#define AFX_ANSICOLORSDLG_H__E7DDE422_0E35_11D2_8A74_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// AnsiColorsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAnsiColorsDlg dialog

#include "ColorButton.h"
class CAnsiColorsDlg : public CDialog
{
// Construction
public:
	CAnsiColorsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAnsiColorsDlg)
	enum { IDD = IDD_ANSI_COLOR };
	BOOL	m_bDarkOnly;
	//}}AFX_DATA
    CColourSetButton m_Btn[16];


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAnsiColorsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAnsiColorsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnRestoreDefault();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANSICOLORSDLG_H__E7DDE422_0E35_11D2_8A74_444553540000__INCLUDED_)
