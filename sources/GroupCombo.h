#if !defined(AFX_GROUPCOMBO_H__09AF2BA1_EB46_11D4_87F4_99AE7647AF15__INCLUDED_)
#define AFX_GROUPCOMBO_H__09AF2BA1_EB46_11D4_87F4_99AE7647AF15__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GroupCombo.h : header file
//
#include <afxtempl.h>

/////////////////////////////////////////////////////////////////////////////
// CGroupCombo window

class CGroupCombo : public CComboBox
{
// Construction
public:
	CGroupCombo();

// Attributes
public:
    void Reinit(BOOL bAddAll = TRUE);
    void SelectGroup(void* pGrp);
    static CList <CGroupCombo* , CGroupCombo*> m_GroupComboList;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGroupCombo)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGroupCombo();

	// Generated message map functions
protected:
	//{{AFX_MSG(CGroupCombo)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GROUPCOMBO_H__09AF2BA1_EB46_11D4_87F4_99AE7647AF15__INCLUDED_)
