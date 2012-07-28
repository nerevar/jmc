//vls-begin// script files
#if !defined(AFX_JMCSCRIPTFILESPAGE_H__4DDE6FCC_1BF2_4046_8404_7E28A5EFF0A4__INCLUDED_)
#define AFX_JMCSCRIPTFILESPAGE_H__4DDE6FCC_1BF2_4046_8404_7E28A5EFF0A4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// JmcScriptFilesPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CJmcScriptFilesPage dialog

class CJmcScriptFilesPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CJmcScriptFilesPage)

    void SetControls();

// Construction
public:
	CJmcScriptFilesPage();
	~CJmcScriptFilesPage();

// Dialog Data
	//{{AFX_DATA(CJmcScriptFilesPage)
	enum { IDD = IDD_JMC_SCRIPTFILES };
	CListBox	m_cFilesList;
	CButton	m_cUp;
	CButton	m_cRemove;
	CButton	m_cDown;
	CButton	m_cAdd;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CJmcScriptFilesPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CJmcScriptFilesPage)
	afx_msg void OnAdd();
	afx_msg void OnDown();
	afx_msg void OnSelchangeList();
	afx_msg void OnRemove();
	afx_msg void OnUp();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JMCSCRIPTFILESPAGE_H__4DDE6FCC_1BF2_4046_8404_7E28A5EFF0A4__INCLUDED_)
//vls-end//

