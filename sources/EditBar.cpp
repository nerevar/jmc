// EditBar.cpp : implementation file
//

#include "stdafx.h"
#include "smc.h"
#include "EditBar.h"
#include "MainFrm.h"
#include "resource.h"
#include "smcdoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CString m_editStr;
/////////////////////////////////////////////////////////////////////////////
// CEditBar

CEditBar::CEditBar()
{
    m_nCurrItem = 0;
    m_nCurSelStart = m_nCurSelEnd = 0;
    m_nHistorySize = ::GetPrivateProfileInt(L"Main", L"History" , 20, szGLOBAL_PROFILE);
    m_bClearInput = ::GetPrivateProfileInt(L"Main", L"ClearInput" , 1, szGLOBAL_PROFILE);
    m_bTokenInput = ::GetPrivateProfileInt(L"Main", L"TokenInput" , 0, szGLOBAL_PROFILE);
    m_bKillOneToken = ::GetPrivateProfileInt(L"Main", L"KillOneToken" , 0, szGLOBAL_PROFILE);
    m_bScrollEnd = ::GetPrivateProfileInt(L"Main", L"ScrollEnd" , 1, szGLOBAL_PROFILE);
    m_bScrollMode = FALSE;

    m_posCurPos = NULL;
    m_bExtending = FALSE;
    m_bExtendingChange = FALSE;
}

CEditBar::~CEditBar()
{
    WritePrivateProfileInt(L"Main", L"History" , m_nHistorySize, szGLOBAL_PROFILE);
    WritePrivateProfileInt(L"Main", L"ClearInput" , m_bClearInput, szGLOBAL_PROFILE);
	WritePrivateProfileInt(L"Main", L"TokenInput" , m_bTokenInput, szGLOBAL_PROFILE);
	WritePrivateProfileInt(L"Main", L"ScrollEnd" , m_bScrollEnd, szGLOBAL_PROFILE);
	WritePrivateProfileInt(L"Main", L"KillOneToken" , m_bKillOneToken, szGLOBAL_PROFILE);
}


BEGIN_MESSAGE_MAP(CEditBar, CDialogBar)
	//{{AFX_MSG_MAP(CEditBar)
	ON_WM_SIZE()
    ON_EN_SETFOCUS(IDC_EDIT, OnSetFocus)
    ON_EN_KILLFOCUS(IDC_EDIT, OnKillFocus)
    ON_EN_CHANGE(IDC_EDIT, OnTextChanged)
	ON_WM_HELPINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CEditBar message handlers
BOOL CEditBar::Create(CWnd* pParentWnd, UINT nStyle, UINT nID )
{
    return  CDialogBar::Create(pParentWnd, IDD_EDIT_BAR , nStyle , nID );
}


void CEditBar::OnSize(UINT nType, int cx, int cy) 
{
	CDialogBar::OnSize(nType, cx, cy);
    CRect CliRect;
    GetClientRect(&CliRect);
    CWnd* pWnd = GetDlgItem(IDC_EDIT);
    if ( pWnd )
        pWnd->MoveWindow(0 , 0 , CliRect.Width() , CliRect.Height() );
}

BOOL CEditBar::PreTranslateMessage(MSG* pMsg) 
{
    CWnd* pWnd;
    CEdit* pEdit;
	if ( pMsg->message == WM_KEYDOWN ) {
        if ( ((CMainFrame*)AfxGetMainWnd())->GetActiveView()->PreTranslateMessage(pMsg) )
            return TRUE;
        switch ( pMsg->wParam ) {
			case VK_RETURN:
			{
				tokenSetup=0;
	 	        if(GetKeyState(VK_SHIFT)&0x1000)  {tokenSetup = 2;}
	 	        if(GetKeyState(VK_CONTROL)&0x1000){tokenSetup = 1;}
     
	            pWnd = ((CMainFrame*)AfxGetMainWnd())->GetActiveView();
		        if ( pWnd ) 
			        pWnd->PostMessage(WM_USER+100 , 0 , 0 );
	            return TRUE;
			}
        case VK_TAB:
            {
                // Check we are in extending mode 
                pEdit = (CEdit*)GetDlgItem(IDC_EDIT);
                if ( m_bExtending ) {
                    // substitute word by new value
                    if ( !m_posCurPos ) 
                        m_posCurPos= m_lstTabWords.GetHeadPosition ();

                    CString strWord = m_lstTabWords.GetNext(m_posCurPos);
                    CString str = m_strStartLine;
                    str += strWord;
                    str+= m_strEndLine;
                    m_bExtendingChange = TRUE;
                    pEdit->SetWindowText (str);
                    pEdit->SetSel (m_strStartLine.GetLength () + strWord.GetLength (), 
                            m_strStartLine.GetLength () + strWord.GetLength ());
                    m_bExtendingChange = FALSE;
                    return TRUE;
                }
                
                // ok, get current line, word etc 
                CString strText;
                pEdit->GetWindowText (strText);

                if ( !strText.GetLength () ) 
                    return TRUE;

                int start, end;
                pEdit->GetSel (start, end);

                if ( end <= 0  || strText[end-1] == ' ') 
                    return TRUE;

                m_strEndLine = strText.Right(strText.GetLength() - end);

                strText = strText.Left(end);

                int cpos = strText.ReverseFind(' ');

                CString strWord;
                if ( cpos < 0 ) {
                    strWord =  strText;
                    m_strStartLine.Empty ();
                } else {
                    strWord = strText.Right(strText.GetLength () - cpos-1);
                    m_strStartLine = strText.Left (cpos +1);
                }
                m_lstTabWords.RemoveAll ();
                // now find all words same as this one 
                CSmcDoc* pDoc = (CSmcDoc*)((CMainFrame*)AfxGetMainWnd())->GetActiveDocument ();
                if ( !pDoc ) 
                    return TRUE;

                m_lstTabWords.AddHead(strWord );
                POSITION pos = pDoc->m_lstTabWords.GetHeadPosition ();
                while ( pos ) {
                    CString str = pDoc->m_lstTabWords.GetNext(pos);
                    if ( !wcsnicmp(str, strWord, strWord.GetLength()) ) {
                        m_lstTabWords.AddTail (str);
                    }
                }

                if ( m_lstTabWords.GetCount () < 2 ) {
                    m_lstTabWords.RemoveAll ();
                    return TRUE;
                }
                m_bExtending = TRUE;
                m_posCurPos = m_lstTabWords.GetHeadPosition();
                m_lstTabWords.GetNext (m_posCurPos);
                strWord = m_lstTabWords.GetNext (m_posCurPos);
                strText = m_strStartLine + strWord + m_strEndLine;
                m_bExtendingChange = TRUE;
                pEdit ->SetWindowText (strText);
                int selpos = m_strStartLine.GetLength () + strWord.GetLength ();
                pEdit->SetSel (selpos, selpos);
                m_bExtendingChange = FALSE;
                return TRUE;

            }
            break;
        case VK_UP:
            if ( GetKeyState(VK_CONTROL) >= 0 ) {
                PrevLine();
                return TRUE;
            } else 
                return FALSE;
        case VK_DOWN:
            if ( GetKeyState(VK_CONTROL) >= 0 ) {
                NextLine();
                return TRUE;
            } else 
                return FALSE;
        case 'C':
            if ( GetKeyState(VK_CONTROL)&0x1000){
                pEdit = (CEdit*)GetDlgItem(IDC_EDIT);
                pEdit->Copy();
                return TRUE;
            }
            break;
		case VK_DELETE:
            if ( GetKeyState(VK_SHIFT)&0x1000){
                pEdit = (CEdit*)GetDlgItem(IDC_EDIT);
                pEdit->Cut();
                return TRUE;
            }
            break;
        case 'X':
            if ( GetKeyState(VK_CONTROL)&0x1000){
                pEdit = (CEdit*)GetDlgItem(IDC_EDIT);
                pEdit->Cut();
                return TRUE;
            }
            break;
		case VK_INSERT:
            if ( GetKeyState(VK_CONTROL)&0x1000){
                pEdit = (CEdit*)GetDlgItem(IDC_EDIT);
                pEdit->Copy();
                return TRUE;
            }
            if ( GetKeyState(VK_SHIFT)&0x1000){
                /*pEdit = (CEdit*)GetDlgItem(IDC_EDIT);
                pEdit->Paste();*/
                DoPaste();
                return TRUE;
            }
            break;
        case 'V':
            if ( GetKeyState(VK_CONTROL)&0x1000){
                /*pEdit = (CEdit*)GetDlgItem(IDC_EDIT);
                pEdit->Paste();*/
                DoPaste();
                return TRUE;
            }
            break;
        default:
            break;
        };
    }
	if ( pMsg->message == WM_SYSKEYDOWN ) {
        if ( ((CMainFrame*)AfxGetMainWnd())->GetActiveView()->PreTranslateMessage(pMsg) )
            return TRUE;

    }
    
	return CDialogBar::PreTranslateMessage(pMsg);
}

CString& CEditBar::GetLine()
{
    static CString str;
    static CString stx;
    m_bScrollMode = FALSE;

    CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT);
    pEdit->GetWindowText(str);
    if ( str.GetLength() >= m_nMinStrLen ) {
        POSITION pos = m_History.Find(str);
        if ( pos ) {
            m_History.RemoveAt(pos);
        }
        m_History.AddTail(str);
        if ( m_History.GetCount() > (int)m_nHistorySize ) {
            m_History.RemoveHead();
        }
        m_nCurrItem = m_History.GetCount();
    }
//*en 1Token, [0Token], Scroll to end
	if(!tokenSetup)
	{
		if ( m_bClearInput) {
		    pEdit->SetWindowText(L"");
	        pEdit->SetSel(0, str.GetLength());
		}
		else
		if(m_bTokenInput)
			{
			int pos = str.Find(L' ')+1;
			if(pos == 0 && !m_bKillOneToken)
				 pos = str.GetLength();
			stx = str.Left(pos);
			if(stx[stx.GetLength()-1]!=L' ' && !m_bKillOneToken)
				    stx = stx + ' ';
			   pEdit->SetWindowText(stx);
		       pEdit->SetSel(m_bScrollEnd?stx.GetLength():0, stx.GetLength());
			}
		else
		    pEdit->SetSel(0, str.GetLength());
	}else if(tokenSetup&2)
			pEdit->SetSel(0, str.GetLength());
		  else
		  {
		    pEdit->SetWindowText(L"");
	        pEdit->SetSel(0, str.GetLength());
		  }

    return str;
//*/en
}

//* en
CString& CEditBar::CleanLine()
{
    static CString str;
    CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT);
    pEdit->GetWindowText(str);
    pEdit->SetWindowText(L"");
    pEdit->SetSel(0, 0);
    return str;
//*/en
}

void CEditBar::NextLine()
{
    // Start scroll mode
    if ( !m_bScrollMode ) {
        m_bScrollMode = TRUE;
        GetDlgItem(IDC_EDIT)->GetWindowText(m_strSrollMask);
    }

    if ( m_nCurrItem >= m_History.GetCount()-1 ) {
        GetDlgItem(IDC_EDIT)->SetWindowText(m_strSrollMask);
        m_nCurrItem = m_History.GetCount();
        return;
    }
    if ( m_bScrollMode && m_strSrollMask.GetLength() ) { // masked scroll
        m_nCurrItem ++;
        POSITION pos;
        while ( m_nCurrItem < m_History.GetCount() && (pos = m_History.FindIndex(m_nCurrItem)) ) {
            CString str = m_History.GetAt(pos);
            if ( !wcscmp(m_strSrollMask, str.Left(m_strSrollMask.GetLength()) ) ) { // matched
                CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT);
                int Curr = m_nCurrItem;
                pEdit->SetWindowText(str);
                if ( m_nCursorPosWhileListing ) 
                    pEdit->SetSel(str.GetLength(), str.GetLength(), TRUE);
                else 
                    pEdit->SetSel(0, 0, TRUE);
                m_bScrollMode = TRUE;
                m_nCurrItem = Curr;
                break;
            }
            m_nCurrItem++;
        }
    } else {
        m_nCurrItem++;
        POSITION pos = m_History.FindIndex(m_nCurrItem);
        CString str = m_History.GetAt(pos);
        CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT);
        int Curr = m_nCurrItem;
        pEdit->SetWindowText(str);
        if ( m_nCursorPosWhileListing ) 
            pEdit->SetSel(str.GetLength(), str.GetLength(), TRUE);
        else 
            pEdit->SetSel(0, 0, TRUE);
        m_bScrollMode = TRUE;
        m_nCurrItem = Curr;
    }

}

void CEditBar::PrevLine()
{
    if ( !m_bScrollMode ) {
        m_bScrollMode = TRUE;
        GetDlgItem(IDC_EDIT)->GetWindowText(m_strSrollMask);
    }

    if ( m_nCurrItem <= 0 )
        return;
    if ( m_bScrollMode && m_strSrollMask.GetLength() ) { // masked scroll
        m_nCurrItem --;
        POSITION pos;
        while ( m_nCurrItem >= 0 && (pos = m_History.FindIndex(m_nCurrItem)) ) {
            CString str = m_History.GetAt(pos);
            if ( !wcscmp(m_strSrollMask, str.Left(m_strSrollMask.GetLength()) ) ) { // matched
                CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT);
                int Curr = m_nCurrItem;
                pEdit->SetWindowText(str);
                if ( m_nCursorPosWhileListing ) 
                    pEdit->SetSel(str.GetLength(), str.GetLength(), TRUE);
                else 
                    pEdit->SetSel(0, 0, TRUE);
                m_bScrollMode = TRUE;
                m_nCurrItem = Curr;
                break;
            }
            m_nCurrItem--;
        }
    } else {
        m_nCurrItem --;
        POSITION pos = m_History.FindIndex(m_nCurrItem);
        CString str = m_History.GetAt(pos);
        CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT);
        int Curr = m_nCurrItem;
        pEdit->SetWindowText(str);
        if ( m_nCursorPosWhileListing ) 
            pEdit->SetSel(str.GetLength(), str.GetLength(), TRUE);
        else 
            pEdit->SetSel(0, 0, TRUE);
        m_nCurrItem = Curr;
        m_bScrollMode = TRUE;
    }
}

void CEditBar::OnSetFocus()
{
    CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT);
    pEdit->SetSel(m_nCurSelStart, m_nCurSelEnd, TRUE);
}

void CEditBar::OnKillFocus()
{
    CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT);
    pEdit->GetSel(m_nCurSelStart , m_nCurSelEnd);
}


void CEditBar::SetHistorySize(UINT size) 
{
	ASSERT(size > 0 );
	m_nHistorySize = size;
    while ( m_History.GetCount() > (int)m_nHistorySize ) 
        m_History.RemoveHead();
	m_nCurrItem = m_History.GetCount();	
}

void CEditBar::DoPaste()
{
    CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT);
    ASSERT(pEdit);
    if ( !OpenClipboard() ) 
        return;

	UINT uFormat = EnumClipboardFormats(0);

    while (uFormat) {
		if(uFormat == 0 || uFormat == CF_TEXT || uFormat == CF_UNICODETEXT)
			break;

		uFormat = EnumClipboardFormats(uFormat);
	}

	if(!uFormat)
		return;

	HANDLE hData = GetClipboardData(uFormat);
	ASSERT(hData);
	if ( hData == NULL )
		return;

	LPWSTR str;

	if (uFormat == CF_TEXT) {

		LPSTR pwsz = (LPSTR)GlobalLock(hData);
		GlobalUnlock(hData);
		CloseClipboard();

		int nLen = MultiByteToWideChar(CP_ACP, 0, pwsz, -1, NULL, 0) + 1;
		str = new wchar_t[ nLen + 1 ];
		MultiByteToWideChar(CP_ACP, 0, pwsz, -1, str, nLen);
		str[nLen] = L'\0';
		
		GlobalUnlock(hData);
	} else if (uFormat == CF_UNICODETEXT) {

		LPWSTR pwsz = (LPWSTR)GlobalLock(hData);
		int nLen = wcslen( pwsz );

		str = new wchar_t[ nLen + 1 ];
		wcscpy(str, pwsz);

		GlobalUnlock(hData);
	}
	CloseClipboard();

    CString strIns, strAdd;
    pEdit->GetWindowText(strIns);
    wchar_t* src = str;
    int StartSel, EndSel;
    pEdit->GetSel(StartSel, EndSel);
    CWnd* pWnd = ((CMainFrame*)AfxGetMainWnd())->GetActiveView();

    do {
        switch (*src ) {
        case L'\n':
            strIns = L"";
            pEdit->SetWindowText(strAdd);
            if ( pWnd ) 
                pWnd->SendMessage(WM_USER+100 , 0 , 0 );
            strAdd = L"";
            break;
        case '\r':
            break;
        case 0:
            break;
        default:
            strAdd += *src;
            break;
        };

    } while (*src++);
	delete[] str;
    if ( strAdd.GetLength() ) {
        int size = strAdd.GetLength();
        strAdd = strIns.Left(StartSel) + strAdd + strIns.Right(strIns.GetLength() - EndSel);
        
        pEdit->SetWindowText(strAdd);
        pEdit->SetSel(StartSel+ size, StartSel+size, TRUE);
    }
}


BOOL CEditBar::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	// TODO: Add your message handler code here and/or call default
	
	// return CDialogBar::OnHelpInfo(pHelpInfo);
    return TRUE;
}


void CEditBar::OnTextChanged() 
{
    CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT);
	pEdit->GetWindowText(m_editStr);
	wcscpy(editStr, m_editStr);
    m_bScrollMode = FALSE;
    m_nCurrItem = m_History.GetCount();
    if ( !m_bExtendingChange ) {
        m_posCurPos = NULL;
        m_lstTabWords.RemoveAll();
        m_bExtending = FALSE;
    }
}

