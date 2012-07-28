#if !defined(AFX_COLORSETTINGSDLG_H__6CA8B7A4_9623_11D1_8592_0060977E8CAC__INCLUDED_)
#define AFX_COLORSETTINGSDLG_H__6CA8B7A4_9623_11D1_8592_0060977E8CAC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ColorSettingsDlg.h : header file
//

#include "ColorButton.h"
/////////////////////////////////////////////////////////////////////////////
// CColorSettingsDlg dialog

class CColorSettingsDlg : public CDialog
{
// Construction
public:
	CColorSettingsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CColorSettingsDlg)
	enum { IDD = IDD_COLORS_DLG };
	CColourSetButton m_ForeColor;
	CColourSetButton m_BackColor;
	CColourSetButton m_Color7;
	CColourSetButton m_Color6;
	CColourSetButton m_Color5;
	CColourSetButton m_Color4;
	CColourSetButton m_Color3;
	CColourSetButton m_Color2;
	CColourSetButton m_Color1;
	CColourSetButton m_Color0;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColorSettingsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CColorSettingsDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORSETTINGSDLG_H__6CA8B7A4_9623_11D1_8592_0060977E8CAC__INCLUDED_)
