#if !defined(AFX_SCROLLOPTIONDLG_H__7D06E514_8CCA_11D1_8590_0060977E8CAC__INCLUDED_)
#define AFX_SCROLLOPTIONDLG_H__7D06E514_8CCA_11D1_8590_0060977E8CAC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ScrollOptionDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CScrollOptionDlg dialog

class CScrollOptionDlg : public CDialog
{
// Construction
public:
	CScrollOptionDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CScrollOptionDlg)
	enum { IDD = IDD_SETSCROLL_SIZE };
	UINT	m_nCount;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScrollOptionDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CScrollOptionDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCROLLOPTIONDLG_H__7D06E514_8CCA_11D1_8590_0060977E8CAC__INCLUDED_)
