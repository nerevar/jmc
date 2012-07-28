#if !defined(AFX_JMCHLIGHTPAGE_H__C886D24A_EC5C_11D4_92C5_0001027ED904__INCLUDED_)
#define AFX_JMCHLIGHTPAGE_H__C886D24A_EC5C_11D4_92C5_0001027ED904__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// JmcHlightPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CJmcHlightPage dialog
#include "GroupCombo.h"
#include "groupedpage.h"

class CJmcHlightPage : public CGroupedPage
{
	DECLARE_DYNCREATE(CJmcHlightPage)

    void ResetList();
    void SetControls();
    CImageList m_ImageList;
    int AddItem(void*);

    BOOL m_bNewItem;

// Construction
public:
	CJmcHlightPage();
	~CJmcHlightPage();
    virtual void GroupListChanged();

// Dialog Data
	//{{AFX_DATA(CJmcHlightPage)
	enum { IDD = IDD_JMC_HLIGHT };
	CComboBox	m_cForeColor;
	CComboBox	m_cBackColor;
	CGroupCombo	m_cGroup;
	CGroupCombo	m_cGroupList;
	CListCtrl	m_cHlightList;
	CString	m_strName;
	CString	m_strGroup;
	int		m_nBackColor;
	int		m_nForeColor;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CJmcHlightPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CJmcHlightPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeGroups();
	afx_msg void OnItemchangedHlight(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeGrp();
	afx_msg void OnAdd();
	afx_msg void OnRemove();
	afx_msg void OnKillfocusName();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnSelchangeColor();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JMCHLIGHTPAGE_H__C886D24A_EC5C_11D4_92C5_0001027ED904__INCLUDED_)
