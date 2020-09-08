#if !defined(AFX_COOLDIALOGBAR_H__6EB5FA61_FFAD_11D1_98D4_444553540000__INCLUDED_)
#define AFX_COOLDIALOGBAR_H__6EB5FA61_FFAD_11D1_98D4_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// CoolDialogBar.h : header file
//


#include <vector>
/////////////////////////////////////////////////////////////////////////////
// CCoolDialogBar window

class CCoolDialogBar : public CControlBar
{
	const COLORREF	m_clrBtnHilight;
	const COLORREF	m_clrBtnShadow;

	// Construction / destruction
public:
    CCoolDialogBar();

// Attributes
public:
    BOOL IsHorz() const;

//vls-begin// multiple output
//    void Load();
//    void Save();
    int m_wndCode;
    BOOL m_bFlag;
	DWORD m_Dock;
	BOOL false_move;
	CPoint false_dest;
    CString m_sTitle;
    void Load();
    void Save();
    void SetTitle(LPCTSTR sTitle);
	int  m_mX;
	int  m_mY;
//vls-end//

	void Resize(int Width, int Height);

// Operations
public:

// Overridables
    virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

// Overrides
public:
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CCoolDialogBar)
    public:
    virtual BOOL Create(CWnd* pParentWnd, LPWSTR &pTitle, CSize& InitialSize, UINT nID, DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_LEFT);
    virtual CSize CalcFixedLayout( BOOL bStretch, BOOL bHorz );
    virtual CSize CalcDynamicLayout( int nLength, DWORD dwMode );
    //}}AFX_VIRTUAL

// Implementation
public:
    virtual ~CCoolDialogBar();
    void StartTracking(BOOL bHorz);
    void StopTracking(BOOL bAccept);
    void OnInvertTracker(const CRect& rect);
    
    // implementation helpers
    CPoint& ClientToWnd(CPoint& point);

	CSize       m_sizeHorz;
    CSize       m_sizeVert;
    CSize       m_sizeFloat;
	UINT        m_nDockBarID;
	BOOL        m_bInRecalcNC;
	std::vector <CCoolDialogBar *> AllVisibleNeighbours (int *OwnIndex = NULL);

protected:
	void		DrawGripper(CDC &dc);

    CSize       m_sizeMin;
    CRect       m_rectBorderVert;
	CRect       m_rectBorderHorz;
    CRect       m_rectTracker;
    CPoint      m_ptOld;
    BOOL        m_bTracking;
	BOOL		m_bTrackHorz;
    int         m_cxEdge;
	CRect		m_rectUndock;
	CRect		m_rectClose;
	CRect		m_rectGripper;
	int			m_cxGripper;
	int			m_cxBorder;
    CBrush		m_brushBkgd;

	// Rob Wolpov 10/15/98 Added support for diagonal resizing
	int  m_cyBorder;
	int  m_cMinWidth;
	int  m_cMinHeight;
	int  m_cCaptionSize;



// Generated message map functions
//protected:
public:
   //{{AFX_MSG(CCoolDialogBar)
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnWindowPosChanging(WINDOWPOS FAR* lpwndpos);
    afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
    afx_msg void OnNcPaint();
    afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
    afx_msg LRESULT OnNcHitTest(CPoint point);
    afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnCaptureChanged(CWnd *pWnd);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
	//}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COOLDIALOGBAR_H__6EB5FA61_FFAD_11D1_98D4_444553540000__INCLUDED_)
