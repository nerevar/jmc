// smcView.h : interface of the CSmcView class
//
/////////////////////////////////////////////////////////////////////////////


#include <vector>

extern CCriticalSection InputSection;
extern CString strInput;
extern HANDLE  hInputDoneEvent;

class CSmcView : public CView
{
protected: // create from serialization only
	CSmcView();
	DECLARE_DYNCREATE(CSmcView)


// Attributes
public:
	CSmcDoc* GetDocument();

    // ANSI support members
    int      m_nCurrentBg, m_nCurrentFg;
    BOOL     m_bAnsiBold; 
    CStringList m_strList;
	std::vector<int> m_LineCountsList;
	int      m_TotalLinesReceived;
	int m_nPageSize, m_nLastPageSize, m_nLineWidth;
    void DrawWithANSI(CDC* pDC, CRect& rect, CString* str, int StringPos = 0);


protected:

    int m_nStartSelectX, m_nStartSelectY, m_nEndSelectX, m_nEndSelectY; // Selection while grabbing screen
    int m_nStartTrackX, m_nStartTrackY, m_nEndTrackX, m_nEndTrackY; // Tracking positions of mouse
                                                                        // while grabbing screen
    int m_nYDiff; // Y difference between page size of window size

	BOOL m_bSelected;

	void ConvertCharPosition(int TextRow, int TextCol, int *LineNum, int *CharPos);

    void SetScrollSettings(BOOL bResetPosition = TRUE);
    void RedrawOneLine(CDC* pDC, int LineNum);



    void SetCurrentANSI(LPCSTR strcCode);



// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSmcView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSmcView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CSmcView)
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
    afx_msg LONG OnLineEntered( UINT wParam, LONG lParam);
    // afx_msg LONG OnAddedDrowLine( UINT wParam, LONG lParam);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in smcView.cpp
inline CSmcDoc* CSmcView::GetDocument()
   { return (CSmcDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////
