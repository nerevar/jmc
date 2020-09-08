#if !defined(AFX_JMCACTIONSPAGE_H__C886D248_EC5C_11D4_92C5_0001027ED904__INCLUDED_)
#define AFX_JMCACTIONSPAGE_H__C886D248_EC5C_11D4_92C5_0001027ED904__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// JMCActionsPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CJMCActionsPage dialog
#include "GroupCombo.h"
#include "groupedpage.h"

class CJMCActionsPage : public CGroupedPage
{
    void ResetList();
    void SetControls();
    CImageList m_ImageList;
    int AddItem(void*);

    BOOL m_bNewItem;

    DECLARE_DYNCREATE(CJMCActionsPage)

// Construction
public:
	CJMCActionsPage();
	~CJMCActionsPage();

    virtual void GroupListChanged();
// Dialog Data
	//{{AFX_DATA(CJMCActionsPage)
	enum { IDD = IDD_JMC_ACTIONS };
	CGroupCombo	m_cGroup;
	CGroupCombo	m_cGroupList;
	CListCtrl	m_cActionsList;
	CString	m_strName;
	CString	m_strText;
	CString	m_strGroup;
	int		m_nPriority;
	int		m_nInputType;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CJMCActionsPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CJMCActionsPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeGroups();
	afx_msg void OnItemchangedActions(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeText();
	afx_msg void OnSelchangeGrp();
	afx_msg void OnAdd();
	afx_msg void OnRemove();
	afx_msg void OnKillfocusName();
	afx_msg void OnSelchangePriority();
	afx_msg void OnSelchangeInputType();
	afx_msg void OnChangeName();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JMCACTIONSPAGE_H__C886D248_EC5C_11D4_92C5_0001027ED904__INCLUDED_)
