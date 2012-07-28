// OneChar.cpp : implementation file
//

#include "stdafx.h"
#include "smc.h"
#include "OneChar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COneChar

COneChar::COneChar()
{
}

COneChar::~COneChar()
{
}


BEGIN_MESSAGE_MAP(COneChar, CEdit)
	//{{AFX_MSG_MAP(COneChar)
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COneChar message handlers

void COneChar::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	//CEdit::OnChar(nChar, nRepCnt, nFlags);
	// CString str = nChar;
	// SetWindowText(str);

    SetWindowText("");
    CEdit::OnChar(nChar, nRepCnt, nFlags);
}
