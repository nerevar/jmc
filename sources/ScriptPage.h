#if !defined(AFX_SCRIPTPAGE_H__D7C9DB41_8F46_11D4_87F2_B838CBE36C2D__INCLUDED_)
#define AFX_SCRIPTPAGE_H__D7C9DB41_8F46_11D4_87F2_B838CBE36C2D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ScriptPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CScriptPage dialog

class CScriptPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CScriptPage)

// Construction
public:
	CScriptPage();
	~CScriptPage();

// Dialog Data
	//{{AFX_DATA(CScriptPage)
	enum { IDD = IDD_SCRIPT_PAGE };
	CComboBox	m_cLang;
	BOOL	m_bAllowDebug;
	int		m_nErrorOutput;
	//}}AFX_DATA

    GUID m_guidLang;
    CLSID m_clsIDs[10];

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CScriptPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CScriptPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeLang();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCRIPTPAGE_H__D7C9DB41_8F46_11D4_87F2_B838CBE36C2D__INCLUDED_)
