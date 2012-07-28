#if !defined(AFX_ALIASPAGE_H__F9383E42_EA6B_11D4_87F4_F6F03C29B215__INCLUDED_)
#define AFX_ALIASPAGE_H__F9383E42_EA6B_11D4_87F4_F6F03C29B215__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AliasPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAliasPage dialog
#include "GroupCombo.h"
#include "groupedpage.h"


class CAliasPage : public CGroupedPage
{
	DECLARE_DYNCREATE(CAliasPage)

    void ResetList();
    void SetControls();
    CImageList m_ImageList;
    int AddItem(void*);

    BOOL m_bNewItem;

// Construction
public:
	CAliasPage();
	~CAliasPage();

    virtual void GroupListChanged();

// Dialog Data
	//{{AFX_DATA(CAliasPage)
	enum { IDD = IDD_JMC_ALIAS };
	CGroupCombo	m_cGroup;
	CGroupCombo	m_cGroupList;
	CListCtrl	m_cAliasList;
	CString	m_strName;
	CString	m_strText;
	CString	m_strGroup;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CAliasPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CAliasPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeGroups();
	afx_msg void OnItemchangedAlias(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeText();
	afx_msg void OnSelchangeGrp();
	afx_msg void OnAdd();
	afx_msg void OnRemove();
	afx_msg void OnKillfocusName();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ALIASPAGE_H__F9383E42_EA6B_11D4_87F4_F6F03C29B215__INCLUDED_)
