#if !defined(AFX_CHARSUBSTPAGE_H__33B73995_EEEA_11D1_85AA_0060977E8CAC__INCLUDED_)
#define AFX_CHARSUBSTPAGE_H__33B73995_EEEA_11D1_85AA_0060977E8CAC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// CharSubstPage.h : header file
//


#include "ttcoreex\\tintinx.h"
#include "onechar.h"
/////////////////////////////////////////////////////////////////////////////
// CCharSubstPage dialog

class CCharSubstPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CCharSubstPage)
// Construction
public:
	CCharSubstPage();   
	~CCharSubstPage();   

    char m_charsSubst[SUBST_ARRAY_SIZE];

// Dialog Data
	//{{AFX_DATA(CCharSubstPage)
	enum { IDD = IDD_SUBST_PAGE };
	COneChar	m_cOldChar;
	COneChar	m_cNewChar;
	CListBox	m_cSubstList;
	BOOL	m_bAllowSubst;
	CString	m_strNewChar;
	CString	m_strOldChar;
	int		m_nReciveSingle;
	int		m_nSendSingle;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCharSubstPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
    void InitControls();
    void EnableControls();

	// Generated message map functions
	//{{AFX_MSG(CCharSubstPage)
	afx_msg void OnSelchangeSubstList();
	afx_msg void OnAllowSubst();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnRemove();
	afx_msg void OnAdd();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHARSUBSTPAGE_H__33B73995_EEEA_11D1_85AA_0060977E8CAC__INCLUDED_)
