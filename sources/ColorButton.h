#if !defined(AFX_COMMONDOCUMENTPARAMSDLG_H__9B3807C7_CC41_11D0_B333_0020AFCD0366__INCLUDED_)
#define AFX_COMMONDOCUMENTPARAMSDLG_H__9B3807C7_CC41_11D0_B333_0020AFCD0366__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// CommonDocumentParamsDlg.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// CColourSetButton window

class CColourSetButton : public CButton
{
// Construction
public:
	CColourSetButton();

// Attributes
public:
    COLORREF m_cCurrColour;
    inline void SetColor(COLORREF cr ) { m_cCurrColour = cr; };
    inline COLORREF GetColor() const { return m_cCurrColour; };

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColourSetButton)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CColourSetButton();

	// Generated message map functions
protected:
	//{{AFX_MSG(CColourSetButton)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMMONDOCUMENTPARAMSDLG_H__9B3807C7_CC41_11D0_B333_0020AFCD0366__INCLUDED_)
