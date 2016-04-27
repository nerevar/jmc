// smcView.cpp : implementation of the CSmcView class
//

#include "stdafx.h"
#include "smc.h"

#include "mainfrm.h"
#include "smcDoc.h"
#include "smcView.h"



//  checked in/out

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CSmcView

IMPLEMENT_DYNCREATE(CSmcView, CView)

BEGIN_MESSAGE_MAP(CSmcView, CView)
	//{{AFX_MSG_MAP(CSmcView)
	ON_WM_SETFOCUS()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CAPTURECHANGED()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
    ON_MESSAGE(WM_USER+100, OnLineEntered)
//    ON_MESSAGE(WM_USER+101, OnAddedDrowLine)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSmcView construction/destruction


CSmcView::CSmcView()
{
	// Set up global preferences

    m_bAnsiBold = FALSE;
    m_nCurrentBg = 0;
    m_nCurrentFg = 7;

    m_bSelected = FALSE;


    // Create event for macro thread

}

CSmcView::~CSmcView()
{
    CSmcDoc* pDoc = (CSmcDoc*)GetDocument();

    // Save colors 

}



BOOL CSmcView::PreCreateWindow(CREATESTRUCT& cs)
{
    cs.style += WS_VSCROLL;
	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CSmcView drawing

static int LengthWithoutANSI(const char* str) 
{
	int ret = 0;
	for(; *str; str++) {
		if(*str == 0x1B) {
			for(; *str && *str != 'm'; str++);
		} else {
			ret++;
		}
	}
	return ret;
}
static int NumOfLines(int StrLength, int LineWidth) 
{
	if (LineWidth <= 0)
		return 0;
	int ret = StrLength / LineWidth;
	if ((StrLength == 0) || (StrLength % LineWidth))
		ret++;
	return ret;
}
void CSmcView::OnDraw(CDC* pDC)
{
    CSmcDoc* pDoc = GetDocument();
    CRect rect;
    GetClientRect(&rect);

    CRgn rgn;
    rgn.CreateRectRgn(rect.left, rect.top, rect.right, rect.bottom);
    pDC->SelectClipRgn(&rgn);

    int ScrollIndex = GetScrollPos(SB_VERT);
	int last_line = min(ScrollIndex + m_nPageSize, pDoc->m_nScrollSize - 1);

    POSITION pos = m_strList.FindIndex(ScrollIndex+m_nPageSize);
    ASSERT(pos);

    pDC->SetBkMode(OPAQUE);
    CFont* pOldFont = pDC->SelectObject(&pDoc->m_fntText);

	int top = rect.top;

	m_LineCountsList.clear();
	for(int n_line = 0, total_lines = 0; pos && total_lines <= m_nPageSize; n_line++) {
        CString str = m_strList.GetPrev(pos);

		int length = LengthWithoutANSI((const char*)str);
		int lines = pDoc->m_bLineWrap ? NumOfLines(length, m_nLineWidth) : 1;

		m_LineCountsList.push_back(lines);
		total_lines += lines;

		rect.top = rect.bottom - pDoc->m_nYsize * lines;
		
		if ( pDC->RectVisible(&rect) )
			DrawWithANSI(pDC, rect, &str, m_nPageSize - n_line - 1);

		rect.bottom = rect.top;

		if (rect.bottom <= top)
			break;
    }
    pDC->SelectObject(pOldFont);
}

void CSmcView::ConvertCharPosition(int TextRow, int TextCol, int *LineNum, int *CharPos)
{
	int row = m_nPageSize;
	*LineNum = TextRow;
	*CharPos = TextCol;
	for (int i = 0; i < m_LineCountsList.size(); i++) {
		row -= m_LineCountsList[i];
		if (row <= TextRow) {
			*LineNum = m_nPageSize - i - 1;
			*CharPos = TextCol + (m_nLineWidth - 1) * (TextRow - row);
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSmcView diagnostics

#ifdef _DEBUG
void CSmcView::AssertValid() const
{
	CView::AssertValid();
}

void CSmcView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CSmcDoc* CSmcView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSmcDoc)));
	return (CSmcDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSmcView message handlers

void CSmcView::OnSetFocus(CWnd* pOldWnd) 
{
	CView::OnSetFocus(pOldWnd);
	
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
    pFrame->m_editBar.SetFocus();
	
}

LONG CSmcView::OnLineEntered( UINT wParam, LONG lParam)
{
    CSmcDoc* pDoc = (CSmcDoc*)GetDocument();
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
    InputSection.Lock();
    CString strCommand = (LPCSTR)pFrame->m_editBar.GetLine();
    if ( strInput.GetLength() ) {
        strInput += cCommandDelimiter;
        strInput += strCommand;
    } else {
        strInput = strCommand;
    }
/*    CString OutText;
    if ( pFrame->GetInputDisplay() && bPasswordEcho ) 
        OutText = (LPCSTR)(strCommand + "\n");
*/    InputSection.Unlock();
    SetEvent(hInputDoneEvent);
/*    if ( pFrame->GetInputDisplay() && bPasswordEcho ) 
        pDoc->DrawSome ((LPSTR)(LPCSTR)OutText);
*/    return 0;
}


void CSmcView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();

    CSmcDoc* pDoc = (CSmcDoc*)GetDocument();


    while ( m_strList.GetCount () < pDoc->m_nScrollSize ) 
        m_strList.AddTail("");

    // Init colors 
    m_nCurrentBg = 0;
    m_nCurrentFg =7;
    

//===================================================================================================================

	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();
    ASSERT_KINDOF(CMainFrame , pFrm);

    pFrm->m_editBar.m_nCursorPosWhileListing = AfxGetApp()->GetProfileInt("Main" , "CursorWileList" , 1);
    pFrm->m_editBar.m_nMinStrLen = AfxGetApp()->GetProfileInt("Main", "MinStrLen" , 2);
	pFrm->m_editBar.GetDlgItem(IDC_EDIT)->SetFont(&pDoc->m_fntText);

    CRect rect;
    GetClientRect(&rect);

    // To init screen dimentions in characters !!!
/*    OnSize(0 , rect.Width(), rect.Height());
*/
    
    SetScrollSettings();

    if ( dwThreadID == 0 ) 
	    CreateThread(NULL, 0, &ClientThread, NULL, 0, &dwThreadID);

}

void CSmcView::OnDestroy() 
{

	CView::OnDestroy();
}



void CSmcView::OnSize(UINT nType, int cx, int cy) 
{
	CSmcDoc* pDoc = GetDocument();
    CView::OnSize(nType, cx, cy);
    
	m_nLastPageSize = m_nPageSize;
    m_nPageSize = min(cy/pDoc->m_nYsize, pDoc->m_nScrollSize);
    
	m_nYDiff = cy - m_nPageSize*pDoc->m_nYsize;
	
	m_nLineWidth = cx/pDoc->m_nCharX;
	if(cx % pDoc->m_nCharX)
		m_nLineWidth++;

    SetScrollSettings(FALSE);
    pDoc->m_nWindowCharsSize = max (cx/pDoc->m_nCharX, 1);
    InvalidateRect(NULL, FALSE);
    UpdateWindow();
}

void CSmcView::SetScrollSettings(BOOL bResetPosition )
{
    CSmcDoc* pDoc = GetDocument();
    int TotalCount = max(pDoc->m_nScrollSize-m_nPageSize, 0 );
    int OldPos = GetScrollPos(SB_VERT);
    SetScrollRange(SB_VERT, 0, TotalCount -1, FALSE);
    if ( bResetPosition ) {
        SetScrollPos(SB_VERT, TotalCount -1 , TRUE);
    } else {
        int LastLine = OldPos + m_nLastPageSize;
        SetScrollPos(SB_VERT, LastLine - m_nPageSize, TRUE);
    }
}


void CSmcView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
    CSmcDoc* pDoc = GetDocument();
    int Pos = GetScrollPos(SB_VERT);
    CRect rect;
    CFont* pOldFont;
    switch ( nSBCode ) {
    case SB_LINEUP:
        if ( Pos ) {
            Pos --;
            SetScrollPos(SB_VERT, Pos, TRUE);
			InvalidateRect(NULL, FALSE );
			UpdateWindow();
			/*
            CDC* pDC = GetDC();
            GetClientRect(&rect);
            pDC->ScrollDC(0 ,pDoc->m_nYsize, rect , NULL , NULL , NULL );

            pDC->SetBkMode(OPAQUE);
            pOldFont = pDC->SelectObject(&pDoc->m_fntText);

            pDC->SelectObject(&pDoc->m_fntText);
            RedrawOneLine(pDC , Pos);
            RedrawOneLine(pDC , Pos+1);
            pDC->SelectObject(pOldFont);
            ReleaseDC(pDC);
			*/
        }
        break;
    case SB_LINEDOWN:
        if ( Pos < pDoc->m_nScrollSize -1-m_nPageSize ) {
            Pos ++;
            SetScrollPos(SB_VERT, Pos, TRUE);
			InvalidateRect(NULL, FALSE );
			UpdateWindow();
			/*
            CDC* pDC = GetDC();
            GetClientRect(&rect);
            pDC->ScrollDC(0 ,-pDoc->m_nYsize, rect , NULL , NULL , NULL );

            pDC->SetBkMode(OPAQUE);
            pOldFont = pDC->SelectObject(&pDoc->m_fntText);

            pDC->SelectObject(&pDoc->m_fntText);

            RedrawOneLine(pDC , Pos+m_nPageSize);
            pDC->SelectObject(pOldFont);
            ReleaseDC(pDC);
			*/
        }
        break;
    case SB_PAGEDOWN:
        if ( Pos < pDoc->m_nScrollSize -1-m_nPageSize ) {
            Pos += m_nPageSize;
            if ( Pos > pDoc->m_nScrollSize -1-m_nPageSize ) 
                Pos = pDoc->m_nScrollSize -1-m_nPageSize;
            SetScrollPos(SB_VERT, Pos, TRUE);
            InvalidateRect(NULL, FALSE );
            UpdateWindow();
        }
        break;
    case SB_PAGEUP:
        if ( Pos ) {
            Pos -= m_nPageSize;
            if ( Pos < 0 ) 
                Pos = 0;
            SetScrollPos(SB_VERT, Pos, TRUE);
            InvalidateRect(NULL, FALSE );
            UpdateWindow();
        }
        break;
    case SB_THUMBPOSITION:
        SetScrollPos(SB_VERT, nPos, TRUE);
        InvalidateRect(NULL, FALSE );
        UpdateWindow();
        break;
    case SB_THUMBTRACK:
        SetScrollPos(SB_VERT, nPos, TRUE);
        InvalidateRect(NULL, FALSE );
        UpdateWindow();
        break;
    default :
        break;
    };
}

void CSmcView::RedrawOneLine(CDC* pDC, int LineNum) // Absolute number of line
{
    CSmcDoc* pDoc = GetDocument();
    int Pos = GetScrollPos(SB_VERT);
    if ( LineNum > Pos+m_nPageSize ) 
        return;

    CRect rect;
    GetClientRect(&rect);
    int Y = rect.bottom - (Pos+m_nPageSize-LineNum+1)*pDoc->m_nYsize;
    POSITION pos = m_strList.FindIndex(LineNum);
    ASSERT(pos);
    CString str = m_strList.GetAt(pos);
    rect.top = Y;
    rect.bottom = rect.top+pDoc->m_nYsize;
    DrawWithANSI(pDC, rect, &str);
}

BOOL CSmcView::OnEraseBkgnd(CDC* pDC) 
{
    return TRUE;
	// return CView::OnEraseBkgnd(pDC);
}


BOOL CSmcView::PreTranslateMessage(MSG* pMsg) 
{
    CSmcDoc* pDoc = GetDocument();

    if ( pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN) {
        WORD AltState = 0;
        if ( GetKeyState(VK_MENU)&0x1000 )
            AltState += HOTKEYF_ALT;
        if ( GetKeyState(VK_CONTROL)&0x1000)
            AltState += HOTKEYF_CONTROL;
        if ( GetKeyState(VK_SHIFT)&0x1000)
            AltState += HOTKEYF_SHIFT;

        BYTE bt = HIBYTE(HIWORD(pMsg->lParam)) & 0x1;
        if ( bt )
            AltState += HOTKEYF_EXT;
    
        LPCSTR action = GetHotkeyValue(MAKEWORD(LOBYTE(HIWORD(pMsg->lParam)), AltState));

        if ( action ) {
            InputSection.Lock();
            if ( strInput.GetLength() ) {
                strInput += cCommandDelimiter;
                strInput += action;
            } else {
                strInput = action;
            }
            InputSection.Unlock();
            strcat((char*)action, "\n");
            SetEvent(hInputDoneEvent);
            pDoc->m_KeyListSection.Unlock();
/*            if ( ((CMainFrame*)AfxGetMainWnd())->GetInputDisplay() && bPasswordEcho ) 
                pDoc->DrawSome((char*)action);
*/            return TRUE;
        }

    }
	return FALSE;
}

void CSmcView::SetCurrentANSI(LPCSTR strCode)
{
    ASSERT(strCode);
    if ( strCode[0] == 0 ) 
        return;

    int value = atoi(strCode);
    if ( !value ) {
        m_nCurrentBg = 0;
        m_nCurrentFg = 7;
        m_bAnsiBold = FALSE;
        return;
    }

    if ( value == 1 ) {
        m_bAnsiBold = TRUE;
    }

    if ( value <= 37 && value >= 30) {
        m_nCurrentFg = value-30;
        return;
    }
    if ( value <= 47 && value >= 40) {
        m_nCurrentBg = value-40;
        return;
    }
}

void CSmcView::DrawWithANSI(CDC* pDC, CRect& rect, CString* str, int nStrPos)
{
    CSmcDoc* pDoc = (CSmcDoc*)GetDocument();
    // Set def colors
    m_nCurrentBg = 0;
    m_nCurrentFg = 7;
    m_bAnsiBold = FALSE;
    CRect OutRect;
    int indexF, indexB;

    char* src = (LPSTR)(LPCSTR)*str;

    int LeftSide =0, TopSide = 0;
    // Lets do different drawing code for selected/unselected mode. Doing to to
    // keep high speed of drawing while unselected mode


    if ( m_bSelected && nStrPos <= m_nEndSelectY && nStrPos >= m_nStartSelectY) {
        BOOL  bOldInvert = !pDoc->m_bRectangleSelection && (nStrPos > m_nStartSelectY);
        BOOL bNewInvert = bOldInvert;
        int CharCount = 0;
        do  {
            // Get text to draw
            char Text[BUFFER_SIZE];
            char* dest = Text;
            int TextLen = 0;
            while (*src && *src != 0x1B ) {
                // check for current bold
                if ( (pDoc->m_bRectangleSelection || nStrPos == m_nStartSelectY) && CharCount == m_nStartSelectX) {
                    bNewInvert = TRUE;
                }
                if ( (pDoc->m_bRectangleSelection || nStrPos == m_nEndSelectY) && CharCount == m_nEndSelectX + 1) {
                    bNewInvert = FALSE;
                }
                 if ( bNewInvert != bOldInvert) 
                     break;

                *dest++ = *src++;
                TextLen++;
                CharCount ++;
            }
            *dest = 0;

            // Draw text

            // Skip \n from the end
            while ( TextLen && (Text[TextLen-1] == '\n' ) )
                TextLen--;

            indexF = m_nCurrentFg + (m_bAnsiBold && !pDoc->m_bDarkOnly ? 8 : 0 );
            indexB = m_nCurrentBg; //+ (m_bAnsiBold ? 8 : 0 );

			if ( bOldInvert ) {
				pDC->SetTextColor(0xFFFFFF-pDoc->m_ForeColors[indexF]);
				pDC->SetBkColor(0xFFFFFF-pDoc->m_BackColors[indexB]);
			} else {
				pDC->SetTextColor(pDoc->m_ForeColors[indexF]);
				pDC->SetBkColor(pDoc->m_BackColors[indexB]);
			}

            CRect myRect(0,0,0,0) ;
            int XShift;
            if ( TextLen) {
                pDC->DrawText(Text, TextLen, &myRect, DT_LEFT | DT_SINGLELINE | DT_NOCLIP | DT_CALCRECT | DT_NOPREFIX );
                XShift = myRect.Width();

            } else {
                XShift = 0;
            }

			if ( XShift ) {
				//LeftSide += XShift;
                //pDC->ExtTextOut(OutRect.left, OutRect.top, ETO_OPAQUE, &OutRect, Text, TextLen, NULL);
				int index = 0;
				while ( pDoc->m_bLineWrap && LeftSide + XShift > rect.Width() ) {
					int len = (rect.Width() - LeftSide) / pDoc->m_nCharX;

					OutRect = rect;
					OutRect.left += LeftSide;
					OutRect.top += TopSide;
					OutRect.right = rect.right;

					pDC->ExtTextOut(OutRect.left, OutRect.top, ETO_OPAQUE, &OutRect, &(Text[index]), len, NULL);
					index += len;
					TextLen -= len;

					LeftSide = 0;
					TopSide += myRect.Height();
					XShift -=  len * pDoc->m_nCharX;
				}
				OutRect = rect;
				OutRect.left += LeftSide;
				OutRect.top += TopSide;
				OutRect.right = OutRect.left + XShift;

				LeftSide += XShift;
                pDC->ExtTextOut(OutRect.left, OutRect.top, ETO_OPAQUE, &OutRect, &(Text[index]), TextLen, NULL);
			}
			
            // !!!! Look for it ! Every time you draw to the end of string !!!! May be change rectangle ???

            if ( bOldInvert != bNewInvert ) {
                bOldInvert = bNewInvert;
                continue;
            }

            // Now check for ANSI colors
            if ( !*src++ ) // if end of string - get out
                break;

            // check for [ command and digit after it. IF not - skip to end of ESC command
            if ( *src++ != '[' /*|| !isdigit(*src)*/ ) {
                while ( *src && *src != 'm' ) src++;
                if ( *src == 'm' )
                    src++;
                continue;
            }
            // now Get colors command and use it
            do {        
                // may be need skip to ; . But .... Speed
                Text[0] = 0;
                dest = Text;
                while ( isdigit(*src) ) 
                    *dest++ = *src++;
                *dest = 0;
                if ( Text[0] ) 
                    SetCurrentANSI(Text);
            } while ( *src && *src++ != 'm' );
        }while ( *src );
        // draw to end of the window
        OutRect = rect;
		OutRect.top += TopSide;
        OutRect.left += LeftSide;
        indexF = m_nCurrentFg + (m_bAnsiBold && !pDoc->m_bDarkOnly ? 8 : 0 );
        indexB = m_nCurrentBg; //+ (m_bAnsiBold ? 8 : 0 );
        if (!pDoc->m_bRectangleSelection && bOldInvert ) {
            pDC->SetTextColor(0xFFFFFF-pDoc->m_ForeColors[indexF]);
			pDC->SetBkColor(0xFFFFFF-pDoc->m_BackColors[indexB]);
        } else {
            pDC->SetTextColor(pDoc->m_ForeColors[indexF]);
			pDC->SetBkColor(pDoc->m_BackColors[indexB]);
        }
        pDC->ExtTextOut(OutRect.left, OutRect.top, ETO_OPAQUE, &OutRect, "", 0, NULL);
    } else {
        do  {
            // Get text to draw
            char Text[BUFFER_SIZE];
            char* dest = Text;
            int TextLen = 0;
            while (*src && *src != 0x1B ) {
                *dest++ = *src++;
                TextLen++;
            }
            *dest = 0;
            // Draw text

            // Skip \n  from the end
            while ( TextLen && (Text[TextLen-1] == '\n' ) )
                TextLen--;

            indexF = m_nCurrentFg + (m_bAnsiBold && !pDoc->m_bDarkOnly ? 8 : 0 );
            indexB = m_nCurrentBg; //+ (m_bAnsiBold ? 8 : 0 );
        
			if (pDoc->m_bShowHiddenText && indexB == indexF)
				pDC->SetTextColor(0xFFFFFF-pDoc->m_ForeColors[indexF]);
			else
				pDC->SetTextColor(pDoc->m_ForeColors[indexF]);
			//pDC->SetTextColor(pDoc->m_ForeColors[indexF]);
			pDC->SetBkColor(pDoc->m_BackColors[indexB]);

            CRect myRect(0,0,0,0);
            int XShift;
            if ( TextLen) {
                pDC->DrawText(Text, TextLen, &myRect, DT_LEFT | DT_SINGLELINE | DT_NOCLIP | DT_CALCRECT | DT_NOPREFIX);
                XShift = myRect.Width();
            } else {
                XShift = 0;
            }

            if ( XShift ) {
				int index = 0;
				while ( pDoc->m_bLineWrap && LeftSide + XShift > rect.Width() ) {
					int len = (rect.Width() - LeftSide) / pDoc->m_nCharX;

					OutRect = rect;
					OutRect.left += LeftSide;
					OutRect.top += TopSide;
					OutRect.right = rect.right;

					pDC->ExtTextOut(OutRect.left, OutRect.top, ETO_OPAQUE, &OutRect, &(Text[index]), len, NULL);
					index += len;
					TextLen -= len;

					LeftSide = 0;
					TopSide += myRect.Height();
					XShift -=  len * pDoc->m_nCharX;
				}
				OutRect = rect;
				OutRect.left += LeftSide;
				OutRect.top += TopSide;
				OutRect.right = OutRect.left + XShift;

				LeftSide += XShift;
                pDC->ExtTextOut(OutRect.left, OutRect.top, ETO_OPAQUE, &OutRect, &(Text[index]), TextLen, NULL);
			}

            // Now check for ANSI colors
            if ( !*src++ ) // if end of string - get out
                break;

            // check for [ command and digit after it. IF not - skip to end of ESC command
            if ( *src++ != '[' /*|| !isdigit(*src)*/ ) {
                while ( *src && *src != 'm' ) src++;
                if ( *src == 'm' )
                    src++;
                continue;
            }
            // now Get colors command and use it
            do {        
                // may be need skip to ; . But .... Speed
                Text[0] = 0;
                dest = Text;
                while ( isdigit(*src) ) 
                    *dest++ = *src++;
                *dest = 0;
                if ( Text[0] ) 
                    SetCurrentANSI(Text);
            } while ( *src && *src++ != 'm' );
        }while ( *src );
        OutRect = rect;
        OutRect.left += LeftSide;
		OutRect.top += TopSide;
        indexF = m_nCurrentFg + (m_bAnsiBold && !pDoc->m_bDarkOnly ? 8 : 0 );
        indexB = m_nCurrentBg; //+ (m_bAnsiBold ? 8 : 0 );


		if (pDoc->m_bShowHiddenText && indexB == indexF)
			pDC->SetTextColor(0xFFFFFF-pDoc->m_ForeColors[indexF]);
		else
			pDC->SetTextColor(pDoc->m_ForeColors[indexF]);
		//pDC->SetTextColor(pDoc->m_ForeColors[indexF]);
		pDC->SetBkColor(pDoc->m_BackColors[indexB]);

        pDC->ExtTextOut(OutRect.left, OutRect.top, ETO_OPAQUE, &OutRect, "", 0, NULL);
    }
}



void CSmcView::OnLButtonDown(UINT nFlags, CPoint point) 
{
    CSmcDoc* pDoc = GetDocument();
	CView::OnLButtonDown(nFlags, point);
    SetCapture();
    pDoc->m_bFrozen = TRUE;
    m_bSelected = TRUE;
    
	int col = point.x/pDoc->m_nCharX;
	int row = (point.y-m_nYDiff)/pDoc->m_nYsize;
	int x, y;
	ConvertCharPosition(row, col, &y, &x);

    m_nStartTrackY = m_nEndTrackY = m_nEndSelectY = m_nStartSelectY = y;
    m_nStartTrackX = m_nEndTrackX = m_nStartSelectX = m_nEndSelectX = x;
}

static char* SkipAnsi(char* ptr)
{

    for ( ; *ptr ; ptr++ ) {
        if ( *ptr == 'm' ){
            ptr++;
            break;
        }
    }

    return ptr;
}

void CSmcView::OnLButtonUp(UINT nFlags, CPoint point) 
{
    CSmcDoc* pDoc = GetDocument();
    if ( m_bSelected ) {
        ReleaseCapture();
        m_bSelected = FALSE;
        InvalidateRect(NULL, FALSE);
        UpdateWindow();
        pDoc->m_bFrozen = FALSE;

        // Well, start forming text for Clipboard
        CString ResultStr;

        // Good, getting reall numbers of strings
        int ScrollIndex = GetScrollPos(SB_VERT)+1;
        ASSERT(m_nStartSelectY>=0);
        POSITION pos = m_strList.FindIndex(ScrollIndex+m_nStartSelectY);
        ASSERT(pos);
        int i = m_nStartSelectY;
        do { 
            CString tmpStr = m_strList.GetAt(pos);
            char* ptr = (LPSTR)(LPCSTR)tmpStr;
            int count = 0;
            if (pDoc->m_bRectangleSelection || i == m_nStartSelectY) {
                // Skip to StartX character
                while ( count < m_nStartSelectX && *ptr){
                    if ( *ptr == 0x1B ){
                        ptr = SkipAnsi(ptr);
                    }
                    else {
                        count++;
                        ptr++;
                    }
                } 
                
            }
            // characters skipped now copy nessesary info to string
            do {
				if ( !(*ptr))
					break;
                if ( *ptr == '\n' ) {
                    ptr++;
                    continue;
                }
				if ( count > m_nEndSelectX && (pDoc->m_bRectangleSelection || i == m_nEndSelectY)) 
                    break;
                if ( *ptr == 0x1B ) {
                    char *endansi = SkipAnsi(ptr);
					if (!pDoc->m_bRemoveESCSelection)
						for (char *p = ptr; p < endansi; p++)
							ResultStr += *p;
					ptr = endansi;
                    continue;
                }
                ResultStr+= *ptr++;
                count++;
            } while ( *ptr );
            if ( i != m_nEndSelectY ) 
                ResultStr +="\r\n";
            i++;
            pos = m_strList.FindIndex(ScrollIndex+i);
        } while ( i<=m_nEndSelectY && pos );
        // Put to clipboard
		if (strlen(ResultStr) != 0)
		{
			VERIFY(OpenClipboard());

			VERIFY(EmptyClipboard());

			LCID lc = GetUserDefaultLCID();
			HANDLE hData = GlobalAlloc(GMEM_ZEROINIT, sizeof(lc) );
			LCID* pLc = (LCID*)GlobalLock(hData);
			*pLc = lc;
			GlobalUnlock(hData);
			SetClipboardData(CF_LOCALE, hData);
			hData = GlobalAlloc(GMEM_ZEROINIT, ResultStr.GetLength()+1 );
			char* buff = (char*)GlobalLock(hData);
			strcpy (buff, (LPSTR)(LPCSTR)ResultStr);
			GlobalUnlock(hData);
			SetClipboardData(CF_TEXT, hData);
			CloseClipboard();
		}
    }
	CView::OnLButtonUp(nFlags, point);
}

void CSmcView::OnMouseMove(UINT nFlags, CPoint point) 
{
    CSmcDoc* pDoc = GetDocument();
    if (m_bSelected) {
		int col = point.x/pDoc->m_nCharX;
		int row = (point.y-m_nYDiff)/pDoc->m_nYsize;
		int x, y;
		ConvertCharPosition(row, col, &y, &x);

		if ( m_nEndTrackY != y || m_nEndTrackX != x ) {
			int OldEndTrackX = m_nEndTrackX, OldEndTrackY = m_nEndTrackY;
			m_nEndTrackY = y;
			m_nEndTrackX = x;

			// Now calculate SELECT positions !!!!
			m_nStartSelectY = min(m_nStartTrackY, m_nEndTrackY);
			m_nEndSelectY = max(m_nStartTrackY, m_nEndTrackY);
			if (pDoc->m_bRectangleSelection) {					
				m_nStartSelectX = min(m_nStartTrackX, m_nEndTrackX);
				m_nEndSelectX = max(m_nStartTrackX, m_nEndTrackX);
			} else {
				if (m_nStartSelectY == m_nEndSelectY) {
					m_nStartSelectX = min(m_nStartTrackX, m_nEndTrackX);
					m_nEndSelectX = max(m_nStartTrackX, m_nEndTrackX);
				} else if (m_nStartSelectY == m_nStartTrackY) {
					m_nStartSelectX = m_nStartTrackX;
					m_nEndSelectX = m_nEndTrackX;
				} else {
					m_nStartSelectX = m_nEndTrackX;
					m_nEndSelectX = m_nStartTrackX;
				}
				
			}

			InvalidateRect(NULL, FALSE);
			UpdateWindow();
		}
	}
	CView::OnMouseMove(nFlags, point);
}

void CSmcView::OnCaptureChanged(CWnd *pWnd) 
{
	OnLButtonUp(0, CPoint(0,0));
	CView::OnCaptureChanged(pWnd);
}

int CSmcView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
    SetClassLong(GetSafeHwnd(), GCL_STYLE, GetClassLong(GetSafeHwnd(), GCL_STYLE)-CS_VREDRAW);
	return 0;
}

void CSmcView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
    CSmcDoc* pDoc = GetDocument();
    switch ( lHint ) {
    case TEXT_ARRIVED:
        // redraw etc 
        {
            CSmcDoc* pDoc = GetDocument();
            CRect rect, rectSmall;

	        GetClientRect(&rect);

			if ( pDoc->m_bClearContents ) {
				for ( POSITION it = m_strList.GetHeadPosition(); it != NULL; m_strList.GetNext(it))
						m_strList.SetAt(it, "");
			}

			CString str = pDoc->m_strTempList.GetHead();
			str.Replace((char)0x1, (char)' ');
	        m_strList.SetAt(m_strList.GetTailPosition(), str);
	        // pDoc->m_strTempList.RemoveHead();

            POSITION pos = pDoc->m_strTempList.GetHeadPosition();
            CString last_line = pDoc->m_strTempList.GetNext(pos);

			int dcnt_last_line = 0;
			int cnt_last_line = 1;
			if (pDoc->m_bLineWrap && m_LineCountsList.size() > 0) {
				int old_len = m_LineCountsList[0];
				cnt_last_line = NumOfLines(LengthWithoutANSI((const char*)last_line), m_nLineWidth);
				dcnt_last_line = cnt_last_line - old_len;
			}

			int new_lines = 0;
	        while(pos) {
		        str = pDoc->m_strTempList.GetNext(pos);
				str.Replace((char)0x1, (char)' ');

		        m_strList.AddTail(str);
		        m_strList.RemoveHead();

				new_lines += pDoc->m_bLineWrap ? 
					NumOfLines(LengthWithoutANSI((const char*)str), m_nLineWidth) : 1;;
	        }
            // check for splitted and head view 
            if ( pMainWnd->m_wndSplitter.GetRowCount () > 1 && pMainWnd->m_wndSplitter.GetPane(0,0) == this ) {
                int OldPos = GetScrollPos(SB_VERT);
                SetScrollPos(SB_VERT, OldPos-new_lines, TRUE);
            } else {
                rectSmall.left = 0;
	            rectSmall.right = rect.right;
	            rectSmall.bottom = rect.bottom; 
	            rectSmall.top = rect.bottom -pDoc->m_nYsize*(new_lines + cnt_last_line);
                if ( pDoc->m_nUpdateCount ) 
	                ScrollWindowEx(0, -pDoc->m_nYsize*(new_lines + dcnt_last_line), NULL, &rect, NULL, /*&rectSmall*/ NULL , SW_INVALIDATE | SW_ERASE);
                /*else */
                if ( pDoc->m_bClearContents ) 
					InvalidateRect(NULL, FALSE);
				else
					InvalidateRect(&rectSmall, FALSE);
	            UpdateWindow();
            }
        }        

        break;
    case SCROLL_SIZE_CHANGED:
        if ( pDoc->m_nScrollSize < m_strList.GetCount() ) { // remove some string from head of list
            while ( pDoc->m_nScrollSize < m_strList.GetCount() ) 
                m_strList.RemoveHead();
        }
        if ( pDoc->m_nScrollSize > m_strList.GetCount() ) {
            while ( pDoc->m_nScrollSize > m_strList.GetCount() ) 
                m_strList.AddHead("");
        }
        SetScrollSettings();
        InvalidateRect(NULL, FALSE);
        UpdateWindow();
        break;
    default:
        break;
    }
	
}
