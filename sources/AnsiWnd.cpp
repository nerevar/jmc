// AnsiWnd.cpp : implementation file
//

#include "stdafx.h"
#include "smc.h"
#include "MAinFrm.h"
#include "smcdoc.h"
#include "AnsiWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAnsiWnd

CAnsiWnd::CAnsiWnd()
{
    m_bAnsiBold = FALSE;
    m_nCurrentBg = 0;
    m_nCurrentFg = 7;

    m_bSelected = FALSE;

}

CAnsiWnd::~CAnsiWnd()
{
}


BEGIN_MESSAGE_MAP(CAnsiWnd, CWnd)
	//{{AFX_MSG_MAP(CAnsiWnd)
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CAPTURECHANGED()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CAnsiWnd message handlers

BOOL CAnsiWnd::PreCreateWindow(CREATESTRUCT& cs)
{
    cs.style += WS_VSCROLL;
	return CWnd::PreCreateWindow(cs);
}

void CAnsiWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

    CRect rect;
    GetClientRect(&rect);

    CRgn rgn;
    rgn.CreateRectRgn(rect.left, rect.top, rect.right, rect.bottom);
    dc.SelectClipRgn(&rgn);

    int ScrollIndex = GetScrollPos(SB_VERT);

    POSITION pos = m_strList.FindIndex(ScrollIndex+m_nPageSize);
    ASSERT(pos);
    rect.top = rect.bottom-pDoc->m_nYsize;
    int i = 0;

    dc.SetBkMode(OPAQUE);
    CFont* pOldFont = dc.SelectObject(&pDoc->m_fntText);

    while ( pos && i++ <= m_nPageSize) {
        CString str = m_strList.GetPrev(pos);

        if ( dc.RectVisible(&rect) )
            DrawWithANSI(&dc, rect, &str, m_nPageSize - i);

        rect.top -= pDoc->m_nYsize;
        rect.bottom -= pDoc->m_nYsize;
    }
    dc.SelectObject(pOldFont);
}


void CAnsiWnd::OnSetFocus(CWnd* pOldWnd) 
{
	CWnd::OnSetFocus(pOldWnd);
	
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
    pFrame->m_editBar.SetFocus();
	
}


int CAnsiWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
    SetClassLong(GetSafeHwnd(), GCL_STYLE, GetClassLong(GetSafeHwnd(), GCL_STYLE)-CS_VREDRAW);
    while ( m_strList.GetCount () < pDoc->m_nScrollSize ) 
        m_strList.AddTail("");

    // Init colors 
    m_nCurrentBg = 0;
    m_nCurrentFg =7;
    

//===================================================================================================================

    ((CMainFrame*)AfxGetMainWnd())->m_editBar.GetDlgItem(IDC_EDIT)->SetFont(&pDoc->m_fntText);

    CRect rect;
    GetClientRect(&rect);

    // To init screen dimentions in characters !!!
    
    SetScrollSettings();
    return 0;
}

void CAnsiWnd::OnSize(UINT nType, int cx, int cy) 
{
    CWnd::OnSize(nType, cx, cy);
    m_nLastPageSize = m_nPageSize;
    m_nPageSize = cy/pDoc->m_nYsize;
    m_nYDiff = cy - m_nPageSize*pDoc->m_nYsize;
    SetScrollSettings(FALSE);
//vls-begin// multiple output
//    pDoc->m_nOutWindowCharsSize = max (cx/pDoc->m_nCharX, 1);
    pDoc->m_nOutputWindowCharsSize[m_wndCode] = max (cx/pDoc->m_nCharX, 1);
//vls-end//
}

void CAnsiWnd::SetScrollSettings(BOOL bResetPosition )
{
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

void CAnsiWnd::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
    int Pos = GetScrollPos(SB_VERT);
    CRect rect;
    CFont* pOldFont;
    switch ( nSBCode ) {
    case SB_LINEUP:
        if ( Pos ) {
            Pos --;
            SetScrollPos(SB_VERT, Pos, TRUE);
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
        }
        break;
    case SB_LINEDOWN:
        if ( Pos < pDoc->m_nScrollSize -1-m_nPageSize ) {
            Pos ++;
            SetScrollPos(SB_VERT, Pos, TRUE);
            CDC* pDC = GetDC();
            GetClientRect(&rect);
            pDC->ScrollDC(0 ,-pDoc->m_nYsize, rect , NULL , NULL , NULL );

            pDC->SetBkMode(OPAQUE);
            pOldFont = pDC->SelectObject(&pDoc->m_fntText);

            pDC->SelectObject(&pDoc->m_fntText);

            RedrawOneLine(pDC , Pos+m_nPageSize);
            pDC->SelectObject(pOldFont);
            ReleaseDC(pDC);
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

void CAnsiWnd::RedrawOneLine(CDC* pDC, int LineNum) // Absolute number of line
{
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

BOOL CAnsiWnd::OnEraseBkgnd(CDC* pDC) 
{
    return TRUE;
	// return CWnd::OnEraseBkgnd(pDC);
}

void CAnsiWnd::SetCurrentANSI(LPCSTR strCode)
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

void CAnsiWnd::DrawWithANSI(CDC* pDC, CRect& rect, CString* str, int nStrPos)
{
    // Set def colors
    m_nCurrentBg = 0;
    m_nCurrentFg = 7;
    m_bAnsiBold = FALSE;
    CRect OutRect;
    int indexF, indexB;

    char* src = (LPSTR)(LPCSTR)*str;

    int LeftSide =0;
    // Lets do different drawing code for selected/unselected mode. Doing to to
    // keep high speed of drawing while unselected mode


    if ( m_bSelected && nStrPos <= m_nEndSelectY && nStrPos >= m_nStartSelectY) {
        BOOL  bOldInvert = nStrPos > m_nStartSelectY;
        BOOL bNewInvert = bOldInvert;
        int CharCount = 0;
        do  {
            // Get text to draw
            char Text[BUFFER_SIZE];
            char* dest = Text;
            int TextLen = 0;
            while (*src && *src != 0x1B ) {
                // check for current bold
                if ( nStrPos == m_nStartSelectY && CharCount == m_nStartSelectX) {
                    bNewInvert = TRUE;
                }
                if ( nStrPos == m_nEndSelectY && CharCount == m_nEndSelectX) {
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
            while ( TextLen && (Text[TextLen-1] == '\n' /*|| Text[TextLen-1] == '\r' */) )
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
            OutRect = rect;
            OutRect.left += LeftSide;
            
            // if ( *src ) 
            //    OutRect.right = OutRect.left + XShift;
            OutRect.right = OutRect.left + XShift;


            LeftSide += XShift;
            // if ( XShift || !*src ) 
            if ( XShift ) 
                pDC->ExtTextOut(OutRect.left, OutRect.top, ETO_OPAQUE, &OutRect, Text, TextLen, NULL);

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
        OutRect.left += LeftSide;
        indexF = m_nCurrentFg + (m_bAnsiBold && !pDoc->m_bDarkOnly ? 8 : 0 );
        indexB = m_nCurrentBg; //+ (m_bAnsiBold ? 8 : 0 );
        if ( bOldInvert ) {
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
            while ( TextLen && (Text[TextLen-1] == '\n' /*|| Text[TextLen-1] == '\r' */) )
                TextLen--;

            indexF = m_nCurrentFg + (m_bAnsiBold && !pDoc->m_bDarkOnly ? 8 : 0 );
            indexB = m_nCurrentBg; //+ (m_bAnsiBold ? 8 : 0 );
        
            pDC->SetTextColor(pDoc->m_ForeColors[indexF]);
            pDC->SetBkColor(pDoc->m_BackColors[indexB]);

            CRect myRect(0,0,0,0);
            int XShift;
            if ( TextLen) {
                pDC->DrawText(Text, TextLen, &myRect, DT_LEFT | DT_SINGLELINE | DT_NOCLIP | DT_CALCRECT | DT_NOPREFIX);
                XShift = myRect.Width();

            } else {
                XShift = 0;
            }
            OutRect = rect;
            OutRect.left += LeftSide;

            OutRect.right = OutRect.left + XShift;

            LeftSide += XShift;
            if ( XShift ) 
                pDC->ExtTextOut(OutRect.left, OutRect.top, ETO_OPAQUE, &OutRect, Text, TextLen, NULL);

/*            pDC->ExtTextOut(myRect.left, myRect.top, ETO_OPAQUE, &myRect, Text, TextLen, NULL);
        
            if ( TextLen) {
                pDC->DrawText(Text, TextLen, &myRect, DT_LEFT | DT_SINGLELINE | DT_NOCLIP | DT_CALCRECT);
                LeftSide += myRect.Width();
            }
*/
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
        indexF = m_nCurrentFg + (m_bAnsiBold && !pDoc->m_bDarkOnly ? 8 : 0 );
        indexB = m_nCurrentBg; //+ (m_bAnsiBold ? 8 : 0 );
        pDC->SetTextColor(pDoc->m_ForeColors[indexF]);
        pDC->SetBkColor(pDoc->m_BackColors[indexB]);
        pDC->ExtTextOut(OutRect.left, OutRect.top, ETO_OPAQUE, &OutRect, "", 0, NULL);
    }
}



void CAnsiWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CWnd::OnLButtonDown(nFlags, point);
    SetCapture();
    pDoc->m_bFrozen = TRUE;
    m_bSelected = TRUE;
    m_nStartTrackY = m_nEndTrackY = m_nEndSelectY = m_nStartSelectY = (point.y-m_nYDiff)/pDoc->m_nYsize;
    m_nStartTrackX = m_nEndTrackX = m_nStartSelectX = m_nEndSelectX = point.x/pDoc->m_nCharX;
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

void CAnsiWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
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
            if (i == m_nStartSelectY && i == m_nEndSelectY) {
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
                if ( *ptr == '\n' /*|| *ptr == '\r'*/ ) {
                    ptr++;
                    continue;
                }
                if ( *ptr == 0x1B ) {
                    ptr = SkipAnsi(ptr);
                    continue;
                } //* en: do not even try
                if ( /*i == m_nEndSelectY &&*/ count >= m_nEndSelectX ) 
                    break;
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
	CWnd::OnLButtonUp(nFlags, point);
}

void CAnsiWnd::OnMouseMove(UINT nFlags, CPoint point) 
{
    if ( m_bSelected && (m_nEndSelectY != (point.y-m_nYDiff)/pDoc->m_nYsize || m_nEndSelectX != point.x/pDoc->m_nCharX) ) {
        int OldEndTrackX = m_nEndTrackX, OldEndTrackY = m_nEndTrackY;
        m_nEndTrackY = (point.y-m_nYDiff)/pDoc->m_nYsize;
        m_nEndTrackX = point.x/pDoc->m_nCharX;

        // int StartX = m_nStartSelectX, StartY= m_nStartSelectY, EndX = m_nEndSelectX, EndY = m_nEndSelectY;

        // Now calculate SELECT positions !!!!
        if ( m_nStartTrackY <= m_nEndTrackY ){
            m_nStartSelectY = m_nStartTrackY;
            m_nEndSelectY = min(m_nEndTrackY, m_nPageSize);

            if ( m_nStartSelectY == m_nEndSelectY ) {
                m_nStartSelectX = min (m_nStartTrackX, m_nEndTrackX );
                m_nStartSelectX = max (0, m_nStartSelectX);
                m_nEndSelectX = max(m_nStartTrackX, m_nEndTrackX );
            }
            else {
                m_nStartSelectX = m_nStartTrackX;
                m_nEndSelectX = max(m_nEndTrackX, 0);
            }
        }
        else {
            m_nStartSelectY = max(m_nEndTrackY,0);
            m_nEndSelectY = m_nStartTrackY;

            m_nStartSelectX = max(m_nEndTrackX, 0);
            m_nEndSelectX = m_nStartTrackX;
        }


        CRect CliRect;
        GetClientRect(&CliRect);
        CliRect.top = min(OldEndTrackY, m_nEndTrackY)*pDoc->m_nYsize+m_nYDiff;
        CliRect.bottom = max(OldEndTrackY, m_nEndTrackY)*pDoc->m_nYsize + pDoc->m_nYsize +m_nYDiff;

        InvalidateRect(&CliRect, FALSE);
        UpdateWindow();
    }
	CWnd::OnMouseMove(nFlags, point);
}

void CAnsiWnd::OnCaptureChanged(CWnd *pWnd) 
{
	OnLButtonUp(0, CPoint(0,0));
	CWnd::OnCaptureChanged(pWnd);
}

void CAnsiWnd::OnUpdate(LPARAM lHint) 
{
    switch ( lHint ) {
    case TEXT_ARRIVED:
        // redraw etc 
        {
            CRect rect, rectSmall;

	        GetClientRect(&rect);

//vls-begin// multiple output
//	        ASSERT(pDoc->m_nOutputUpdateCount == pDoc->m_strOutoputTempList.GetCount()-1 );
//
//	        m_strList.SetAt(m_strList.GetTailPosition(), pDoc->m_strOutoputTempList.GetHead());
//	        // pDoc->m_strOutoputTempList.RemoveHead();
//
//            POSITION pos = pDoc->m_strOutoputTempList.GetHeadPosition();
//            pDoc->m_strOutoputTempList.GetNext(pos);
//
//	        while(pos) {
//		        CString str = pDoc->m_strOutoputTempList.GetNext(pos);
//		        m_strList.AddTail(str);
//		        m_strList.RemoveHead();
//	        }
//            rectSmall.left = 0;
//	        rectSmall.right = rect.right;
//	        rectSmall.bottom = rect.bottom; 
//	        rectSmall.top = rect.bottom -pDoc->m_nYsize*(pDoc->m_nOutputUpdateCount+1);
//            if ( pDoc->m_nOutputUpdateCount ) 
//	            ScrollWindowEx(0, -pDoc->m_nYsize*pDoc->m_nOutputUpdateCount, NULL, &rect, NULL, /*&rectSmall*/ NULL , SW_INVALIDATE | SW_ERASE);
            if (pDoc->m_strOutputTempList[m_wndCode].GetCount() > 0 && pDoc->m_nOutputUpdateCount[m_wndCode] == pDoc->m_strOutputTempList[m_wndCode].GetCount()-1) {
                m_strList.SetAt(m_strList.GetTailPosition(), pDoc->m_strOutputTempList[m_wndCode].GetHead());
                // pDoc->m_strOutoputTempList.RemoveHead();
                
                POSITION pos = pDoc->m_strOutputTempList[m_wndCode].GetHeadPosition();
                pDoc->m_strOutputTempList[m_wndCode].GetNext(pos);
                
                while(pos) {
                    CString str = pDoc->m_strOutputTempList[m_wndCode].GetNext(pos);
                    m_strList.AddTail(str);
                    m_strList.RemoveHead();
                }
                rectSmall.left = 0;
                rectSmall.right = rect.right;
                rectSmall.bottom = rect.bottom; 
                rectSmall.top = rect.bottom -pDoc->m_nYsize*(pDoc->m_nOutputUpdateCount[m_wndCode]+1);
                if ( pDoc->m_nOutputUpdateCount[m_wndCode] ) 
                    ScrollWindowEx(0, -pDoc->m_nYsize*pDoc->m_nOutputUpdateCount[m_wndCode], NULL, &rect, NULL, /*&rectSmall*/ NULL , SW_INVALIDATE | SW_ERASE);
            }
//vls-end//
            /*else */
            InvalidateRect(&rectSmall, FALSE);
	        UpdateWindow();
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


