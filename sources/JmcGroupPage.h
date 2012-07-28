#if !defined(AFX_JMCGROUPPAGE_H__DF5602C4_EC03_11D4_87F4_A021CAE36F24__INCLUDED_)
#define AFX_JMCGROUPPAGE_H__DF5602C4_EC03_11D4_87F4_A021CAE36F24__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// JmcGroupPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CJmcGroupPage dialog
class CGROUP;
#include "groupedpage.h"

class CJmcGroupPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CJmcGroupPage)

    CImageList m_ImageList;
// Construction
public:
	CJmcGroupPage();
	~CJmcGroupPage();

    void EnableControls();
    int AddItem(CGROUP* pGrp);

// Dialog Data
	//{{AFX_DATA(CJmcGroupPage)
	enum { IDD = IDD_JMC_GROUPS };
	CListCtrl	m_cGroups;
	CButton	m_cGlobal;
	CButton	m_cEnabled;
	BOOL	m_bEnabled;
	BOOL	m_bGlobal;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CJmcGroupPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CJmcGroupPage)
	afx_msg void OnRemove();
	afx_msg void OnAdd();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeGroups();
	afx_msg void OnEnabledGlobal();
	afx_msg void OnItemchangedGroups(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////
// CAddGroupDlg dialog

class CAddGroupDlg : public CDialog
{
// Construction
public:
	CAddGroupDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAddGroupDlg)
	enum { IDD = IDD_ADD_GROUP };
	CString	m_strName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddGroupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAddGroupDlg)
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JMCGROUPPAGE_H__DF5602C4_EC03_11D4_87F4_A021CAE36F24__INCLUDED_)
