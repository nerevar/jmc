#if !defined(AFX_MUDEMUDLG_H__88E60A44_1D89_11D5_87F4_88DAEDAE6339__INCLUDED_)
#define AFX_MUDEMUDLG_H__88E60A44_1D89_11D5_87F4_88DAEDAE6339__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MudEmuDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMudEmuDlg dialog

class CMudEmuDlg : public CDialog
{
// Construction
public:
	CMudEmuDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMudEmuDlg)
	enum { IDD = IDD_TESTTEXT };
	CString	m_strText;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMudEmuDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

    void SendData();
    char* m_pBuff;
    int m_nBufSize, m_nOffset;

	// Generated message map functions
	//{{AFX_MSG(CMudEmuDlg)
	afx_msg void OnSend();
	afx_msg void OnSendLine();
	virtual void OnCancel();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MUDEMUDLG_H__88E60A44_1D89_11D5_87F4_88DAEDAE6339__INCLUDED_)
