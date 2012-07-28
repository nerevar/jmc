#ifndef _EDIT_BAR_H_
#define _EDIT_BAR_H_
// EditBar.h : header file
//



/////////////////////////////////////////////////////////////////////////////
// CEditBar window

class CEditBar : public CDialogBar
{
// Construction
public:
	CEditBar();
    BOOL Create(CWnd* pParentWnd, UINT nStyle, UINT nID );

// Attributes
public:
    CString& GetLine();
    CString& CleanLine();
	CString m_str;
	void SetHistorySize(UINT size) ;
	inline UINT GetHistorySize() {return m_nHistorySize;};
	inline CStringList& GetHistory() {return m_History;};
    int m_nCurrItem;
	int tokenSetup;

protected:
    void DoPaste();
    CStringList m_History;
    void PrevLine();
    void NextLine();
    int m_nCurSelStart , m_nCurSelEnd;
	UINT m_nHistorySize;
    BOOL m_bScrollMode;
    CString m_strSrollMask;

    CStringList m_lstTabWords;
    POSITION m_posCurPos;
    CString m_strStartLine, m_strEndLine;
    BOOL m_bExtending, m_bExtendingChange;


// Operations
public:
    int m_nCursorPosWhileListing;
    int m_nMinStrLen;
    BOOL m_bClearInput;
    BOOL m_bTokenInput;
    BOOL m_bKillOneToken;
    BOOL m_bScrollEnd;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditBar)
	public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CEditBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CEditBar)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	//}}AFX_MSG
    afx_msg void OnSetFocus();
    afx_msg void OnKillFocus();
    afx_msg void OnTextChanged();
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
#endif //_EDIT_BAR_H_
