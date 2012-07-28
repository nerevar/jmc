#if !defined(AFX_PRESSKEYDLG_H__DEEA4E04_93D1_11D1_8592_0060977E8CAC__INCLUDED_)
#define AFX_PRESSKEYDLG_H__DEEA4E04_93D1_11D1_8592_0060977E8CAC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// PressKeyDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPressKeyDlg dialog

class CPressKeyDlg : public CDialog
{
// Construction
public:
	CPressKeyDlg(CWnd* pParent = NULL);   // standard constructor

    WORD m_nKeyCode, m_nAltState;

// Dialog Data
	//{{AFX_DATA(CPressKeyDlg)
	enum { IDD = IDD_NEW_HOTKEY };
	CHotKeyCtrl	m_HotKey;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPressKeyDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPressKeyDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRESSKEYDLG_H__DEEA4E04_93D1_11D1_8592_0060977E8CAC__INCLUDED_)
