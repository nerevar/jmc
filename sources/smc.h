// smc.h : main header file for the SMC application
//

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "ttcoreex\\tintinx.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CSmcApp:
// See smc.cpp for the implementation of this class
//

extern char szGLOBAL_PROFILE[MAX_PATH];

BOOL WritePrivateProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry,
	LPBYTE pData, UINT nBytes, LPCTSTR lpszFile);
BOOL GetPrivateProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry,
	BYTE** ppData, UINT* pBytes, LPCTSTR lpszFile);
BOOL WritePrivateProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue,LPCTSTR lpszFile);

extern CString m_editStr;


class CSmcApp : public CWinApp
{
public:
	CSmcApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSmcApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

    CString m_strCurrentProfile;
    GUID m_guidScriptLang;

	//{{AFX_MSG(CSmcApp)
	afx_msg void OnAppAbout();
	afx_msg void OnFileNewProfile();
	afx_msg void OnFileSaveprofile();
	afx_msg void OnFileLoadprofile();
	afx_msg void OnHelpContents();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CSmcApp theApp;

/////////////////////////////////////////////////////////////////////////////
