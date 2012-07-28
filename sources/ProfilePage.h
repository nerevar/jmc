#if !defined(AFX_PROFILEPAGE_H__D40E42E2_094A_11D2_8A74_444553540000__INCLUDED_)
#define AFX_PROFILEPAGE_H__D40E42E2_094A_11D2_8A74_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ProfilePage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProfilePage dialog

class CProfilePage : public CPropertyPage
{
	DECLARE_DYNCREATE(CProfilePage)

// Construction
public:
	CProfilePage();
	~CProfilePage();

// Dialog Data
	//{{AFX_DATA(CProfilePage)
	enum { IDD = IDD_PROFILE_PAGE };
	CString	m_strCommand;
	CString	m_strSaveName;
	CString	m_strStartFileName;
    CString m_strLangSect;
    CString m_strLangFile;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CProfilePage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CProfilePage)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROFILEPAGE_H__D40E42E2_094A_11D2_8A74_444553540000__INCLUDED_)
