#if !defined(AFX_LOGPARAMSPAGE_H__DC6099C2_0E95_11D2_8A74_444553540000__INCLUDED_)
#define AFX_LOGPARAMSPAGE_H__DC6099C2_0E95_11D2_8A74_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// LogParamsPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLogParamsPage dialog

class CLogParamsPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CLogParamsPage)

// Construction
public:
	CLogParamsPage();
	~CLogParamsPage();

// Dialog Data
	//{{AFX_DATA(CLogParamsPage)
	enum { IDD = IDD_LOG_PARAMS_PAGE };
	BOOL	m_bLogANSI;
	BOOL	m_bRMASupport;
	int		m_nAppendMode;
	BOOL	m_bHTML;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CLogParamsPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CLogParamsPage)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOGPARAMSPAGE_H__DC6099C2_0E95_11D2_8A74_444553540000__INCLUDED_)
