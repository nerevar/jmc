#if !defined _ACCEPTKEYEDIT_H_
#define _ACCEPTKEYEDIT_H_

// AcceptKeyEdit.h : header file
//

	/* Flags used to describe key modifiers */

	/* Key map structure - an array of those represents current mapping */
typedef struct tagKeyMap {
    UINT    uKey;
    int     scan;
	int     msg;				// Message id
	UINT	uFlags;				// combination of KM_ flags for keycode
} KEYMAP;

/////////////////////////////////////////////////////////////////////////////
// CAcceptKeyEdit window

class CAcceptKeyEdit : public CEdit
{
// Construction
public:
	BOOL m_fKeyPressed;			// Shows if last pressed key was mappable
	KEYMAP m_key;				// Represents that key pressed
	BOOL FormatKeyMapString(KEYMAP *key, CString &str);
	CAcceptKeyEdit();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAcceptKeyEdit)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CAcceptKeyEdit();

	// Generated message map functions
protected:
	//{{AFX_MSG(CAcceptKeyEdit)
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
};

/////////////////////////////////////////////////////////////////////////////
#endif // _ACCEPTKEYEDIT_H_
