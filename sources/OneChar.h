#if !defined(AFX_ONECHAR_H__21606003_0955_11D2_8A74_444553540000__INCLUDED_)
#define AFX_ONECHAR_H__21606003_0955_11D2_8A74_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// OneChar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COneChar window

class COneChar : public CEdit
{
// Construction
public:
	COneChar();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COneChar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~COneChar();

	// Generated message map functions
protected:
	//{{AFX_MSG(COneChar)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ONECHAR_H__21606003_0955_11D2_8A74_444553540000__INCLUDED_)
