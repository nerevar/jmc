#if !defined(AFX_ANSIWND_H__AE1EFBE6_A4A2_11D2_8975_0060080BBFF8__INCLUDED_)
#define AFX_ANSIWND_H__AE1EFBE6_A4A2_11D2_8975_0060080BBFF8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AnsiWnd.h : header file
//

#include <vector>

/////////////////////////////////////////////////////////////////////////////
// CAnsiWnd window

class CAnsiWnd : public CWnd
{
// Construction
public:
	CAnsiWnd();

// Attributes
public:
    int      m_nCurrentBg, m_nCurrentFg;
    BOOL     m_bAnsiBold; 
    CStringList m_strList;
	std::vector<int> m_LineCountsList;
	int m_TotalLinesReceived;
	int m_nPageSize, m_nLastPageSize;
	int m_nLineWidth;

//vls-begin// multiple output
    int m_wndCode;
//vls-end//

protected:

    int m_nStartSelectX, m_nStartSelectY, m_nEndSelectX, m_nEndSelectY; // Selection while grabbing screen
    int m_nStartTrackX, m_nStartTrackY, m_nEndTrackX, m_nEndTrackY; // Tracking positions of mouse
                                                                        // while grabbing screen
    int m_nYDiff; // Y difference between page size of window size

	BOOL m_bSelected;

	void ConvertCharPosition(int TextRow, int TextCol, int *LineNum, int *CharPos);

    void SetScrollSettings(BOOL bResetPosition = TRUE);
    void RedrawOneLine(CDC* pDC, int LineNum);



    void DrawWithANSI(CDC* pDC, CRect& rect, CString* str, int StringPos = 0);
    void SetCurrentANSI(const wchar_t *strcCode);
    
// Operations
public:
    void OnUpdate(LPARAM lHint);


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAnsiWnd)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CAnsiWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CAnsiWnd)
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnCaptureChanged(CWnd *pWnd);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANSIWND_H__AE1EFBE6_A4A2_11D2_8975_0060080BBFF8__INCLUDED_)
