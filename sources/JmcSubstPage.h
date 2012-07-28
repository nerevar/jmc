//vls-begin// subst page
#if !defined(AFX_JMCSUBSTPAGE_H__F5652134_8F2E_411B_A357_F77DF4BA42E5__INCLUDED_)
#define AFX_JMCSUBSTPAGE_H__F5652134_8F2E_411B_A357_F77DF4BA42E5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// JmcSubstPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CJmcSubstPage dialog

class CJmcSubstPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CJmcSubstPage)

    void SetControls();
    int AddItem(void*);

    BOOL m_bNewItem;

// Construction
public:
	CJmcSubstPage();
	~CJmcSubstPage();

// Dialog Data
	//{{AFX_DATA(CJmcSubstPage)
	enum { IDD = IDD_JMC_SUBST };
	CListCtrl	m_cSubstList;
	CString	m_strName;
	CString	m_strText;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CJmcSubstPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CJmcSubstPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnAdd();
	afx_msg void OnRemove();
	afx_msg void OnChangeText();
	afx_msg void OnKillfocusName();
	afx_msg void OnItemchangedAlias(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeName();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JMCSUBSTPAGE_H__F5652134_8F2E_411B_A357_F77DF4BA42E5__INCLUDED_)
//vls-end//
