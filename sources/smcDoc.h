// smcDoc.h : interface of the CSmcDoc class
//
/////////////////////////////////////////////////////////////////////////////

enum UpdateReasons {
    TEXT_ARRIVED = 1,
    SCROLL_SIZE_CHANGED
};

extern unsigned long __stdcall ClientThread(void * pParam);
extern HANDLE  hInputDoneEvent ;
extern CCriticalSection InputSection;
extern CString strInput;
extern DWORD dwThreadID;

//vls-begin// script files
extern BOOL bScriptFileListChanged;
//vls-end//

//* en: colors=restore default
static COLORREF DefColors[16] = {
    RGB(0, 0, 0), 
    RGB(128, 0, 0), 
    RGB(0, 128, 0), 
    RGB(128, 128, 0), 
    RGB(0, 0, 128), 
    RGB(128, 0, 128),
    RGB(0, 128, 128), 
    RGB(192, 192, 192),

    RGB(128, 128, 128), 
    RGB(255, 0, 0), 
    RGB(0, 255, 0), 
    RGB(255, 255, 0), 
    RGB(0, 0, 255), 
    RGB(255, 0, 255), 
    RGB(0, 255, 255), 
    RGB(255, 255, 255)
};

//* en

#include "ScriptParseDlg.h"
#include "MudEmuDlg.h"

class CSmcDoc : public CDocument
{
protected: // create from serialization only
	CSmcDoc();
	DECLARE_DYNCREATE(CSmcDoc)



// Attributes
public:

	BOOL DoProfileSave();

    CScriptParseDlg m_ParseDlg;
    CMudEmuDlg      m_MudEmulator;

// Operations
public:
    CString m_strProfileName, m_strDefSetFile, 
		m_strDefSaveFile, m_strSaveCommand;

	LOGFONT m_lfText;
	CFont m_fntText;

    wchar_t m_cCommandChar; // command char. default is '#'

    BOOL m_bFrozen;

//vls-begin// multiple output
//    int m_nWindowCharsSize, m_nOutWindowCharsSize;
    int m_nWindowCharsSize;
    int m_nOutputWindowCharsSize[MAX_OUTPUT];
//vls-end//


    int m_nYsize, m_nCharX;
	BOOL m_bRectangleSelection;
	BOOL m_bRemoveESCSelection;
	BOOL m_bLineWrap;
	BOOL m_bShowTimestamps;
	BOOL m_bShowHiddenText;
    void RecalcCharSize();

    // COLOR SUPPORT
    BOOL     m_bDarkOnly;
    COLORREF m_ForeColors[16];
    COLORREF m_BackColors[16];

    CStringList m_lstTabWords;
    void FillTabWords(const wchar_t* strWords);


//vls-begin// multiple output
//    CStringList m_strTempList, m_strOutoputTempList; // list of lines to draw 
//    int m_nUpdateCount, m_nOutputUpdateCount; // count of lines arrives from the mud 
//    CCriticalSection m_UpdateSection, m_UpdateOutputSection; // crit section to prevent access to lines list
    CStringList m_strTempList;
    int m_nUpdateCount;
	BOOL m_bClearContents;
    CCriticalSection m_UpdateSection;

    CStringList m_strOutputTempList[MAX_OUTPUT];
    int m_nOutputUpdateCount[MAX_OUTPUT];
	BOOL m_bClearOutputContents[MAX_OUTPUT];
    CCriticalSection m_OutputUpdateSection[MAX_OUTPUT];
//vls-end//


    CCriticalSection m_KeyListSection;

    void DrawSome(const wchar_t* str);

    BOOL m_bSplitOnBackscroll;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSmcDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL CanCloseFrame(CFrameWnd* pFrame);
	virtual void DeleteContents();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSmcDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CSmcDoc)
    afx_msg void OnParseScript();
    afx_msg void OnBreakScript();
	afx_msg void OnUpdateParseScript(CCmdUI* pCmdUI);
	afx_msg void OnOptionsFont();
	afx_msg void OnScriptingReload();
	afx_msg void OnOptionsColors();
	afx_msg void OnDrawTextAdded();
	afx_msg void OnOutputTextAdded();
	afx_msg void OnPause();
	afx_msg void OnUpdatePause(CCmdUI* pCmdUI);
	afx_msg void OnOptionsScrollbuffer();
	afx_msg void OnOptionsKeywords();
	afx_msg void OnScriptingLunchdebuger();
	afx_msg void OnUpdateScriptingLunchdebuger(CCmdUI* pCmdUI);
	afx_msg void OnViewMudemulator();
	afx_msg void OnUpdateViewMudemulator(CCmdUI* pCmdUI);
	afx_msg void OnShowOutput();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
extern CSmcDoc* pDoc;

/////////////////////////////////////////////////////////////////////////////
