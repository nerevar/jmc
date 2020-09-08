// AcceptKeyEdit.cpp : implementation file 
//

#include "stdafx.h"
#include "AcceptKeyEdit.h"
#include "ttcoreex/tintinx.h"
#include "ttcoreex/ttobjects.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef _MT
//#error qqq
#endif

#ifndef _DLL
//#error qqq
#endif


/////////////////////////////////////////////////////////////////////////////
// CAcceptKeyEdit

CAcceptKeyEdit::CAcceptKeyEdit()
{
	m_fKeyPressed = FALSE;
}

CAcceptKeyEdit::~CAcceptKeyEdit()
{
}


BEGIN_MESSAGE_MAP(CAcceptKeyEdit, CEdit)
	//{{AFX_MSG_MAP(CAcceptKeyEdit)
	ON_WM_KEYUP()
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAcceptKeyEdit message handlers



BOOL CAcceptKeyEdit::FormatKeyMapString(KEYMAP *key, CString &str)
{
	// Formats a string representing a given key.
	// Returns FALSE if key was not mappable. E.g it was Tab
	// and should be passed to superclass window function.

	str = _T("");

	if(key->uFlags & HOTKEYF_CONTROL)
		str += _T("Ctrl+");

	if(key->uFlags & HOTKEYF_ALT)
		str += _T("Alt+");

	if(key->uFlags & HOTKEYF_SHIFT)
		str += _T("Shift+");


    int ext;
    if ( key->uFlags&HOTKEYF_EXT ) {
        ext = 1;
    }
    else 
        ext = 0;

    // first check for present in the Names table
    for ( int i = 0 ; i < HotKeyNamesSize ; i++ ) {
        if ( HotKeyNames[i].m_scancode  == key->scan && HotKeyNames[i].m_Ext  == ext ) {
            str += HotKeyNames[i].m_strName ;
            return TRUE;
        }
    }
    if ( key->uFlags&HOTKEYF_EXT ) 
        return FALSE;

	if(key->uKey >= '0' && key->uKey <= '9' ||
			key->uKey >= 'A' && key->uKey <= 'Z')
		str += (char)key->uKey;
	else if(key->uKey >= VK_F1 && key->uKey <= VK_F24) {
		str += _T("F");
		CString strNum;
		strNum.Format(L"%d", key->uKey - VK_F1 + 1);
		str += strNum;
        
	}else 
        return FALSE;
    return TRUE;
}

void CAcceptKeyEdit::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// Either shows the key pressed in a text form or 
	// passes it up for processing
	CString str;
	KEYMAP key;
    str.Format(L"VK = %d Scancode = %d Extended = %d\n", nChar , LOBYTE(nFlags), HIBYTE(nFlags) & 0x1 );
    TRACE (str);
    

	key.uKey = nChar;
    key.scan = LOBYTE(nFlags);
	key.uFlags = 0;
	if(GetKeyState(VK_SHIFT) < 0)
		key.uFlags |= HOTKEYF_SHIFT;
	if(GetKeyState(VK_CONTROL) < 0)
		key.uFlags |= HOTKEYF_CONTROL;
	if(GetKeyState(VK_MENU) < 0)
		key.uFlags |= HOTKEYF_ALT;
    if ( HIBYTE(nFlags) & 0x1 ) 
        key.uFlags |= HOTKEYF_EXT;

	if(FormatKeyMapString(&key, str)) {
		m_key = key;
		SetWindowText(str);
		m_fKeyPressed = TRUE;
	}
	else
		CEdit::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CAcceptKeyEdit::OnChar(UINT /*nChar*/, UINT /*nRepCnt*/, UINT /*nFlags*/) 
{
	// Don't call it.
	// Or editor will insert another one char in it, which is unacceptable.
	// CEdit::OnChar(nChar, nRepCnt, nFlags);
}

BOOL CAcceptKeyEdit::PreTranslateMessage(MSG* pMsg) 
{
//    TRACE("CAcceptKeyEdit - PreTranslate message #%d\n", pMsg->message);

	if (pMsg->message == WM_KEYDOWN || pMsg->message == WM_CHAR || pMsg->message == WM_SYSKEYDOWN) 
		return TRUE;
	
	if (pMsg->message == WM_KEYUP || pMsg->message == WM_SYSKEYUP)
	{
		OnKeyUp(pMsg->wParam, LOWORD(pMsg->lParam), HIWORD(pMsg->lParam));
		return TRUE;
	}
	
	return CEdit::PreTranslateMessage(pMsg);
}
