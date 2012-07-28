#if !defined(AFX_JMCHOTKEYPAGE_H__C886D24E_EC5C_11D4_92C5_0001027ED904__INCLUDED_)
#define AFX_JMCHOTKEYPAGE_H__C886D24E_EC5C_11D4_92C5_0001027ED904__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// JmcHotkeyPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CJmcHotkeyPage dialog
#include "AcceptKeyEdit.h"

//vls-begin// grouped hotkeys
#include "GroupCombo.h"
#include "groupedpage.h"
//vls-end//

//vls-begin// grouped hotkeys
//class CJmcHotkeyPage : public CPropertyPage
class CJmcHotkeyPage : public CGroupedPage
//vls-end//
{
	DECLARE_DYNCREATE(CJmcHotkeyPage)

    void SetControls();
//vls-begin// grouped hotkeys
    void ResetList();
    CImageList m_ImageList;
//vls-end//
    int AddItem(void*);

    BOOL m_bNewItem;
    CAcceptKeyEdit m_wndHotCtrl;

// Construction
public:
	CJmcHotkeyPage();
	~CJmcHotkeyPage();
//vls-begin// grouped hotkeys
    virtual void GroupListChanged();
//vls-end//

// Dialog Data
	//{{AFX_DATA(CJmcHotkeyPage)
	enum { IDD = IDD_JMC_HOTKEY };
	CListCtrl	m_cHotkeysList;
	CString	m_strText;
	CString	m_strKey;
//vls-begin// grouped hotkeys
	CGroupCombo	m_cGroup;
	CGroupCombo	m_cGroupList;
    CString m_strGroup;
//vls-end//
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CJmcHotkeyPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CJmcHotkeyPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnAdd();
	afx_msg void OnItemchangedAlias(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRemove();
	afx_msg void OnChangeText();
	afx_msg void OnKillfocusKey();
	afx_msg void OnChangeKey();
	afx_msg void OnSelchangeGroups();
	afx_msg void OnSelchangeGrp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JMCHOTKEYPAGE_H__C886D24E_EC5C_11D4_92C5_0001027ED904__INCLUDED_)
