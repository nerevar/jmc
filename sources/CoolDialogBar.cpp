// CoolDialogBar.cpp : implementation file
//

#include "stdafx.h"
#include "afxpriv.h"    // for CDockContext
#include "afxwin.h"    // for CDockContext
#include "CoolDialogBar.h"
#include "smc.h"
#include "resource.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CCoolDialogBar


CCoolDialogBar::CCoolDialogBar() : 
m_clrBtnHilight(::GetSysColor(COLOR_BTNHILIGHT)),
m_clrBtnShadow(::GetSysColor(COLOR_BTNSHADOW))
{
    
    m_sizeMin = CSize(32, 32);
	/*
    m_sizeHorz = CSize(200, 200);
    m_sizeVert = CSize(200, 200);
    m_sizeFloat = CSize(200, 200);
    */
    m_bTracking = FALSE;
    m_bInRecalcNC = FALSE;
    m_cxEdge = 6;
	m_cxBorder = 3;
	m_cxGripper = 20;
    m_brushBkgd.CreateSolidBrush(GetSysColor(COLOR_BTNFACE));

	// Rob Wolpov 10/15/98 Added support for diagonal resizing
	m_cyBorder	 = 3;		
	m_cCaptionSize = GetSystemMetrics(SM_CYSMCAPTION);
	m_cMinWidth	 = GetSystemMetrics(SM_CXMIN);
	m_cMinHeight	 = GetSystemMetrics(SM_CYMIN);
}

CCoolDialogBar::~CCoolDialogBar()
{
}

BEGIN_MESSAGE_MAP(CCoolDialogBar, CControlBar)
    //{{AFX_MSG_MAP(CCoolDialogBar)
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_SETCURSOR()
	ON_WM_WINDOWPOSCHANGING()
    ON_WM_WINDOWPOSCHANGED()
    ON_WM_NCPAINT()
    ON_WM_NCLBUTTONDOWN()
    ON_WM_NCHITTEST()
    ON_WM_NCCALCSIZE()
    ON_WM_LBUTTONDOWN()
    ON_WM_CAPTURECHANGED()
    ON_WM_LBUTTONDBLCLK()
	ON_WM_NCLBUTTONDBLCLK()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCoolDialogBar message handlers

void CCoolDialogBar::OnUpdateCmdUI(class CFrameWnd *pTarget, int bDisableIfNoHndler)
{
    UpdateDialogControls(pTarget, bDisableIfNoHndler);
}

BOOL CCoolDialogBar::Create(CWnd* pParentWnd, LPWSTR &pTitle, CSize& InitialSize, UINT nID, DWORD dwStyle )
{
    ASSERT_VALID(pParentWnd);   // must have a parent
    ASSERT (!((dwStyle & CBRS_SIZE_FIXED) && (dwStyle & CBRS_SIZE_DYNAMIC)));
	
    // save the style -- AMENDED by Holger Thiele - Thankyou
    m_dwStyle = dwStyle & CBRS_ALL;

	// create the base window
    CString wndclass = AfxRegisterWndClass(CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW),
        m_brushBkgd, 0);
    if (!CWnd::Create(wndclass, pTitle, dwStyle, CRect(0,0,0,0),
        pParentWnd, nID))
        return FALSE;

	// create the child dialog

	// use the dialog dimensions as default base dimensions
	CRect rc(0, 0 , 100, 200);
    m_sizeHorz = m_sizeVert = m_sizeFloat = InitialSize;
	m_sizeHorz.cy += m_cxEdge + m_cxBorder;
	m_sizeVert.cx += m_cxEdge + m_cxBorder;
    GetClientRect(&rc);
//vls-begin// multiple output
    if (m_wndCode != 0)
        ShowWindow(SW_HIDE);
//vls-end//

    return TRUE;
}

vector <CCoolDialogBar *> CCoolDialogBar::AllVisibleNeighbours(int *OwnIndex) 
{
	vector <CCoolDialogBar *> ret;
	
	CDockBar *pDockBar = (CDockBar*)GetParent();
	ASSERT_KINDOF(CDockBar , pDockBar);
	
	bool row = false;
	CPtrArray *bars = &pDockBar->m_arrBars;
	for (int i = 0; i < bars->GetSize(); i++) {
		//CControlBar *pBar = pDockBar->GetDockedControlBar(i);
		CControlBar* pBar = (CControlBar*)pDockBar->m_arrBars[i];
		if (HIWORD(pBar) == 0)
			pBar = NULL;
		if (!pBar) {
			if (row)
				break;
			ret.clear();
			continue;
		}
		if (!pBar->IsVisible())
			continue;
		if (this == pBar) {
			row = true;
			if (OwnIndex)
				*OwnIndex = ret.size();
		}
		ret.push_back((CCoolDialogBar*)pBar);
	}

	return ret;
}

CSize CCoolDialogBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	if(IsFloating())
		return m_sizeFloat;

	CDockBar *pDockBar = (CDockBar*)GetParent();
	ASSERT_KINDOF(CDockBar , pDockBar);

	CRect dockrc;
	pDockBar->GetWindowRect(dockrc);
	//m_pDockSite->GetControlBar(m_nDockBarID)->GetWindowRect(dockrc);

	CSize sz = (bHorz ? m_sizeHorz : m_sizeVert);

	int width = (bHorz ? sz.cx : sz.cy);
	int height = (bHorz ? sz.cy : sz.cx);
		
	if (bStretch) {
		width = 32767;
	} else {
		int own;
		vector <CCoolDialogBar *> row = AllVisibleNeighbours(&own);
		for (int i = 0; i < row.size(); i++) {
			CSize sz2 = (bHorz ? row[i]->m_sizeHorz : row[i]->m_sizeVert);
			height = max(height, (bHorz ? sz2.cy : sz2.cx));
		}
		if (row.size() == 1) {
			width = (bHorz ? dockrc.Width() : dockrc.Height());
		} else if (own + 1 < row.size()) {
			CRect myrc, rc;
			GetWindowRect(myrc);
			row[own + 1]->GetWindowRect(rc);
			if (bHorz)
				width = rc.left - myrc.left + 2;
			else
				width = rc.top - myrc.top + 1;
		} else {
			CRect myrc;
			GetWindowRect(myrc);
			if (bHorz)
				width = dockrc.right - myrc.left - 1;
			else
				width = dockrc.bottom - myrc.top + 1;
		}
	}
		
	if (bHorz)
		sz = CSize(width, height);
	else
		sz = CSize(height, width);

	sz.cx = max(sz.cx, m_sizeMin.cx);
	sz.cy = max(sz.cy, m_sizeMin.cy);

	if (bHorz)
		m_sizeHorz = sz;
	else
		m_sizeVert = sz;

	return sz;
}

CSize CCoolDialogBar::CalcDynamicLayout(int nLength, DWORD dwMode)
{
	// Rob Wolpov 10/15/98 Added support for diagonal sizing
	if (IsFloating())
	{
		// Enable diagonal arrow cursor for resizing
		GetParent()->GetParent()->ModifyStyle(MFS_4THICKFRAME,0);
	}    
	if (dwMode & (LM_HORZDOCK | LM_VERTDOCK))
	{
		SetWindowPos(NULL, 0, 0, 0, 0,
			SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER |
			SWP_NOACTIVATE | SWP_FRAMECHANGED);
	 	m_pDockSite->RecalcLayout();
	    return CControlBar::CalcDynamicLayout(nLength,dwMode);
	}

    if (dwMode & LM_MRUWIDTH)
        return m_sizeFloat;

    if (dwMode & LM_COMMIT)
    {
        m_sizeFloat.cx = nLength;
        return m_sizeFloat;
    }

	// Rob Wolpov 10/15/98 Added support for diagonal sizing
	if (IsFloating())
	{
		RECT	window_rect;
		POINT	cursor_pt;
		
		GetCursorPos(&cursor_pt);
		GetParent()->GetParent()->GetWindowRect(&window_rect);
		
		switch (m_pDockContext->m_nHitTest)
		{
		case HTTOPLEFT:
			m_sizeFloat.cx = max(window_rect.right - cursor_pt.x,
				m_cMinWidth) - m_cxBorder;
			m_sizeFloat.cy = max(window_rect.bottom - m_cCaptionSize - 
				cursor_pt.y,m_cMinHeight) - 1;
			m_pDockContext->m_rectFrameDragHorz.top = min(cursor_pt.y,
				window_rect.bottom - m_cCaptionSize - m_cMinHeight) - 
				m_cyBorder;
			m_pDockContext->m_rectFrameDragHorz.left = min(cursor_pt.x,
				window_rect.right - m_cMinWidth) - 1;
			return m_sizeFloat;
			
		case HTTOPRIGHT:
			m_sizeFloat.cx = max(cursor_pt.x - window_rect.left,
				m_cMinWidth);
			m_sizeFloat.cy = max(window_rect.bottom - m_cCaptionSize - 
				cursor_pt.y,m_cMinHeight) - 1;
			m_pDockContext->m_rectFrameDragHorz.top = min(cursor_pt.y,
				window_rect.bottom - m_cCaptionSize - m_cMinHeight) - 
				m_cyBorder;
			return m_sizeFloat;
			
		case HTBOTTOMLEFT:
			m_sizeFloat.cx = max(window_rect.right - cursor_pt.x,
				m_cMinWidth) - m_cxBorder;
			m_sizeFloat.cy = max(cursor_pt.y - window_rect.top - 
				m_cCaptionSize,m_cMinHeight);
			m_pDockContext->m_rectFrameDragHorz.left = min(cursor_pt.x,
				window_rect.right - m_cMinWidth) - 1;
			return m_sizeFloat;
			
		case HTBOTTOMRIGHT:
			m_sizeFloat.cx = max(cursor_pt.x - window_rect.left,
				m_cMinWidth);
			m_sizeFloat.cy = max(cursor_pt.y - window_rect.top - 
				m_cCaptionSize,m_cMinHeight);
			return m_sizeFloat;
		}
	}
	
	if (dwMode & LM_LENGTHY)
        return CSize(m_sizeFloat.cx,
            m_sizeFloat.cy = max(m_sizeMin.cy, nLength));
    else
        return CSize(max(m_sizeMin.cx, nLength), m_sizeFloat.cy);
}

void CCoolDialogBar::OnWindowPosChanging(WINDOWPOS FAR* lpwndpos) 
{
    CControlBar::OnWindowPosChanging(lpwndpos);

	if(!::IsWindow(m_hWnd) )
		return;

	if (IsFloating())
		return;
    
}

void CCoolDialogBar::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
//    MessageBox("winposchanged", "JMC", MB_OK | MB_ICONSTOP);
    CControlBar::OnWindowPosChanged(lpwndpos);

	if(!::IsWindow(m_hWnd) )
		return;
    if (m_bInRecalcNC) 
	{
		CRect rc;
		GetClientRect(rc);
		return;
	}

    // Find on which side are we docked
    UINT nDockBarID = GetParent()->GetDlgCtrlID();

    // Return if dropped at same location
    if (nDockBarID == m_nDockBarID // no docking side change
        && (lpwndpos->flags & SWP_NOSIZE) // no size change
        && ((m_dwStyle & CBRS_BORDER_ANY) != CBRS_BORDER_ANY))
        return;

	m_nDockBarID = nDockBarID;

    // Force recalc the non-client area
	m_bInRecalcNC = TRUE;
	SetWindowPos(NULL, 0,0,0,0,
		SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
	m_bInRecalcNC = FALSE;
}

BOOL CCoolDialogBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
    if ((nHitTest != HTRIGHT && nHitTest != HTBOTTOM) || m_bTracking)
        return CControlBar::OnSetCursor(pWnd, nHitTest, message);

	if (nHitTest == HTBOTTOM)
        SetCursor(LoadCursor(NULL, IDC_SIZENS));
    else
        SetCursor(LoadCursor(NULL, IDC_SIZEWE));
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////
// Mouse Handling
//
void CCoolDialogBar::OnLButtonUp(UINT nFlags, CPoint point) 
{
    //MessageBox("lup", "JMC", MB_OK | MB_ICONSTOP);
    if (!m_bTracking /*&& !false_move*/)
        CControlBar::OnLButtonUp(nFlags, point);
    else
    {
        ClientToWnd(point);
        StopTracking(TRUE);
    }
}

void CCoolDialogBar::OnMouseMove(UINT nFlags, CPoint point) 
{
    //MessageBox("mmove", "JMC", MB_OK | MB_ICONSTOP);
    //if(!false_move)
	if (IsFloating() || !m_bTracking)
    {
        CControlBar::OnMouseMove(nFlags, point);
        return;
    }

    CPoint cpt = m_rectTracker.CenterPoint();

    ClientToWnd(point);

	if (m_bTrackHorz)
    {
        if (cpt.y != point.y)
        {
            OnInvertTracker(m_rectTracker);
            m_rectTracker.OffsetRect(0, point.y - cpt.y);
            OnInvertTracker(m_rectTracker);
        }
    }
    else 
    {
        if (cpt.x != point.x)
        {
            OnInvertTracker(m_rectTracker);
            m_rectTracker.OffsetRect(point.x - cpt.x, 0);
            OnInvertTracker(m_rectTracker);
        }
    }
}

void CCoolDialogBar::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
    // Compute the rectangle of the mobile edge
	CRect tmpRect;
    GetWindowRect(tmpRect);
    m_rectBorderHorz = m_rectBorderVert = CRect(0, 0, tmpRect.Width(), tmpRect.Height());
    
    DWORD dwBorderStyle = m_dwStyle | CBRS_BORDER_ANY;

    switch(m_nDockBarID)
    {
		
    case AFX_IDW_DOCKBAR_TOP:
        dwBorderStyle &= ~CBRS_BORDER_BOTTOM;
		dwBorderStyle &= ~CBRS_BORDER_RIGHT;
        lpncsp->rgrc[0].left += m_cxGripper;
        lpncsp->rgrc[0].bottom += -m_cxEdge;
        lpncsp->rgrc[0].top += m_cxBorder;
        lpncsp->rgrc[0].right += -m_cxBorder;
	    m_rectBorderHorz.top = m_rectBorderHorz.bottom - m_cxEdge;
		m_rectBorderVert.left = m_rectBorderVert.right - m_cxEdge;
        break;
    case AFX_IDW_DOCKBAR_BOTTOM:
        dwBorderStyle &= ~CBRS_BORDER_TOP;
		dwBorderStyle &= ~CBRS_BORDER_RIGHT;
        lpncsp->rgrc[0].left += m_cxGripper;
        lpncsp->rgrc[0].top += m_cxEdge;
        lpncsp->rgrc[0].bottom += -m_cxBorder;
        lpncsp->rgrc[0].right += -m_cxBorder;
        m_rectBorderHorz.bottom = m_rectBorderHorz.top + m_cxEdge;
		m_rectBorderVert.left = m_rectBorderVert.right - m_cxEdge;
        break;
    case AFX_IDW_DOCKBAR_LEFT:
        dwBorderStyle &= ~CBRS_BORDER_RIGHT;
		dwBorderStyle &= ~CBRS_BORDER_BOTTOM;
        lpncsp->rgrc[0].right += -m_cxEdge;
        lpncsp->rgrc[0].left += m_cxBorder;
        lpncsp->rgrc[0].bottom += -m_cxBorder;
        lpncsp->rgrc[0].top += m_cxGripper;
		m_rectBorderHorz.top = m_rectBorderHorz.bottom - m_cxEdge;
        m_rectBorderVert.left = m_rectBorderVert.right - m_cxEdge;
        break;
    case AFX_IDW_DOCKBAR_RIGHT:
        dwBorderStyle &= ~CBRS_BORDER_LEFT;
		dwBorderStyle &= ~CBRS_BORDER_BOTTOM;
        lpncsp->rgrc[0].left += m_cxEdge;
        lpncsp->rgrc[0].right += -m_cxBorder;
        lpncsp->rgrc[0].bottom += -m_cxBorder;
        lpncsp->rgrc[0].top += m_cxGripper;
		m_rectBorderHorz.top = m_rectBorderHorz.bottom - m_cxEdge;
        m_rectBorderVert.right = m_rectBorderVert.left + m_cxEdge;
        break;
		
    default:
        m_rectBorderHorz.SetRectEmpty();
		m_rectBorderVert.SetRectEmpty();
        break;
    }

    SetBarStyle(dwBorderStyle);
}

void CCoolDialogBar::OnNcPaint() 
{
    EraseNonClient();

	CWindowDC dc(this);
    dc.Draw3dRect(m_rectBorderVert, GetSysColor(COLOR_BTNHIGHLIGHT),
                    GetSysColor(COLOR_BTNSHADOW));
	dc.Draw3dRect(m_rectBorderHorz, GetSysColor(COLOR_BTNHIGHLIGHT),
                    GetSysColor(COLOR_BTNSHADOW));

	DrawGripper(dc);
	
	CRect pRect;
	GetClientRect( &pRect );
	InvalidateRect( &pRect, TRUE );
}

void CCoolDialogBar::OnNcLButtonDown(UINT nHitTest, CPoint point) 
{
    //MessageBox("ncldown", "JMC", MB_OK | MB_ICONSTOP);
    if (m_bTracking) return;

	if((nHitTest == HTSYSMENU) && !IsFloating())
        GetDockingFrame()->ShowControlBar(this, FALSE, FALSE);
    else if ((nHitTest == HTMINBUTTON) && !IsFloating())
        m_pDockContext->ToggleDocking();
	else if ((nHitTest == HTCAPTION) && !IsFloating() && (m_pDockBar != NULL))
    {
        // start the drag
        ASSERT(m_pDockContext != NULL);
        m_pDockContext->StartDrag(point);
    }
    else if ((nHitTest == HTRIGHT || nHitTest == HTBOTTOM) && !IsFloating())
		StartTracking(nHitTest == HTBOTTOM);
	else    
        CControlBar::OnNcLButtonDown(nHitTest, point);
}

LRESULT CCoolDialogBar::OnNcHitTest(CPoint point) 
{
    if (IsFloating())
        return CControlBar::OnNcHitTest(point);

    CRect rc;
    GetWindowRect(rc);
    point.Offset(-rc.left, -rc.top);
	if(m_rectClose.PtInRect(point))
		return HTSYSMENU;
	else if (m_rectUndock.PtInRect(point))
		return HTMINBUTTON;
	else if (m_rectGripper.PtInRect(point))
		return HTCAPTION;
    else if (m_rectBorderVert.PtInRect(point))
		return HTRIGHT;
	else if (m_rectBorderHorz.PtInRect(point))
        return HTBOTTOM;
    else
        return CControlBar::OnNcHitTest(point);
}

void CCoolDialogBar::OnLButtonDown(UINT nFlags, CPoint point) 
{
    if (m_pDockBar != NULL/* || false_move*/)
    {
        // start the drag
        //ASSERT(m_pDockContext != NULL);
        ClientToScreen(&point);
        m_pDockContext->StartDrag(point);

    }
    else
    {
        CWnd::OnLButtonDown(nFlags, point);
    }
}

void CCoolDialogBar::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
    //MessageBox("ldbl", "JMC", MB_OK | MB_ICONSTOP);
    // only toggle docking if clicked in "void" space
    if (m_pDockBar != NULL)
    {
        // toggle docking
        ASSERT(m_pDockContext != NULL);
        m_pDockContext->ToggleDocking();
    }
    else
    {
        CWnd::OnLButtonDblClk(nFlags, point);
    }
}

void CCoolDialogBar::StartTracking(BOOL bHorz)
{
    SetCapture();

    // make sure no updates are pending
    RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_UPDATENOW);
    m_pDockSite->LockWindowUpdate();

	m_rectTracker = (bHorz ? m_rectBorderHorz : m_rectBorderVert);
    m_ptOld = m_rectTracker.CenterPoint();
    m_bTracking = TRUE;
	m_bTrackHorz = bHorz;

    OnInvertTracker(m_rectTracker);
}

void CCoolDialogBar::OnCaptureChanged(CWnd *pWnd) 
{
    if (m_bTracking && pWnd != this)
        StopTracking(FALSE); // cancel tracking

    CControlBar::OnCaptureChanged(pWnd);
}

void CCoolDialogBar::StopTracking(BOOL bAccept)
{
    OnInvertTracker(m_rectTracker);
    m_pDockSite->UnlockWindowUpdate();
    m_bTracking = FALSE;
    ReleaseCapture();
    
    if (!bAccept) return;

    int maxsize, minsize, newsize;
    CRect rcc;
	GetWindowRect(rcc);
    m_pDockSite->GetWindowRect(rcc);

	CSize sz = IsHorz() ? m_sizeHorz : m_sizeVert;
	newsize = m_bTrackHorz ? sz.cy : sz.cx;
    maxsize = newsize + (m_bTrackHorz ? rcc.Height() : rcc.Width());
    minsize = m_bTrackHorz ? m_sizeMin.cy : m_sizeMin.cx;

    CPoint point = m_rectTracker.CenterPoint();
	int dsize = 0;
    switch (m_nDockBarID)
    {
    case AFX_IDW_DOCKBAR_TOP:
		if (m_bTrackHorz)
			dsize = point.y - m_ptOld.y; 
		else
			dsize = point.x - m_ptOld.x; 
		break;
    case AFX_IDW_DOCKBAR_BOTTOM:
		if (m_bTrackHorz)
			dsize = -point.y + m_ptOld.y; 
		else
			dsize = point.x - m_ptOld.x; 
		break;
    case AFX_IDW_DOCKBAR_LEFT:
		if (!m_bTrackHorz)
			dsize = point.x - m_ptOld.x; 
		else 
			dsize = point.y - m_ptOld.y;  
		break;
    case AFX_IDW_DOCKBAR_RIGHT:
		if (!m_bTrackHorz)
			dsize = -point.x + m_ptOld.x; 
		else
			dsize = point.y - m_ptOld.y;  
		break;
    }

	newsize += dsize;
    newsize = max(minsize, min(maxsize, newsize));

	if (m_bTrackHorz) {
		dsize = newsize - sz.cy;
		sz.cy = newsize;
	} else {
		dsize = newsize - sz.cx;
		sz.cx = newsize;
	}

	sz.cx = max(sz.cx, m_sizeMin.cx);
	sz.cy = max(sz.cy, m_sizeMin.cy);

    if (IsHorz())
        m_sizeHorz = sz;
    else
        m_sizeVert = sz;

	int own;
	vector <CCoolDialogBar *> row = AllVisibleNeighbours(&own);
	if (IsHorz() == m_bTrackHorz) { //just copy new size
		for (int i = 0; i < row.size(); i++)
			if (i != own) {
				if (IsHorz())
					row[i]->m_sizeHorz.cy = m_sizeHorz.cy;
				else
					row[i]->m_sizeVert.cx = m_sizeVert.cx;
			}
	} else { // resize neightbour
		int other = own + 1;
		if (other >= row.size())
			other -= 2;
		if (other >= 0) {
			if (IsHorz()) {
				CRect rc;
				row[other]->GetWindowRect(rc);
				row[other]->m_sizeHorz.cx = max(row[other]->m_sizeHorz.cx - dsize, m_sizeMin.cx);
				rc.left += dsize;
				GetParent()->ScreenToClient(rc);
				row[other]->MoveWindow(rc);
			} else {
				CRect rc;
				row[other]->GetWindowRect(rc);
				row[other]->m_sizeVert.cy = max(row[other]->m_sizeVert.cy - dsize, m_sizeMin.cy);
				rc.top += dsize;
				GetParent()->ScreenToClient(rc);
				row[other]->MoveWindow(rc);
			}
		}
	}

    m_pDockSite->RecalcLayout();
}

void CCoolDialogBar::OnInvertTracker(const CRect& rect)
{
    ASSERT_VALID(this);
    ASSERT(!rect.IsRectEmpty());
    ASSERT(m_bTracking);

    CRect rct = rect, rcc, rcf;
    GetWindowRect(rcc);
    m_pDockSite->GetWindowRect(rcf);

    rct.OffsetRect(rcc.left - rcf.left, rcc.top - rcf.top);
    rct.DeflateRect(1, 1);

    CDC *pDC = m_pDockSite->GetDCEx(NULL,
        DCX_WINDOW|DCX_CACHE|DCX_LOCKWINDOWUPDATE);

    CBrush* pBrush = CDC::GetHalftoneBrush();
    HBRUSH hOldBrush = NULL;
    if (pBrush != NULL)
        hOldBrush = (HBRUSH)SelectObject(pDC->m_hDC, pBrush->m_hObject);

    pDC->PatBlt(rct.left, rct.top, rct.Width(), rct.Height(), PATINVERT);

    if (hOldBrush != NULL)
        SelectObject(pDC->m_hDC, hOldBrush);

    m_pDockSite->ReleaseDC(pDC);
}

BOOL CCoolDialogBar::IsHorz() const
{
    return (m_nDockBarID == AFX_IDW_DOCKBAR_TOP ||
        m_nDockBarID == AFX_IDW_DOCKBAR_BOTTOM);
}

CPoint& CCoolDialogBar::ClientToWnd(CPoint& point)
{
	switch (m_nDockBarID) {
	case AFX_IDW_DOCKBAR_LEFT:
		point.x += m_cxEdge;
		point.y += m_cxGripper;
		break;
	case AFX_IDW_DOCKBAR_TOP:
		point.y += m_cxEdge;
		point.x += m_cxGripper;
		break;
	case AFX_IDW_DOCKBAR_RIGHT:
		point.y += m_cxGripper;
		point.x += m_cxEdge;
		break;
	case AFX_IDW_DOCKBAR_BOTTOM:
		point.y += m_cxEdge;
		point.x += m_cxGripper;
		break;
	}    

    return point;
}

void CCoolDialogBar::DrawGripper(CDC & dc)
{
    // no gripper if floating
	if( m_dwStyle & CBRS_FLOATING )
		return;

	// -==HACK==-
	// in order to calculate the client area properly after docking,
	// the client area must be recalculated twice (I have no idea why)
	m_pDockSite->RecalcLayout();
	// -==END HACK==-

	CRect gripper;
	GetWindowRect( gripper );
	ScreenToClient( gripper );
	gripper.OffsetRect( -gripper.left, -gripper.top );
	
	if( m_dwStyle & CBRS_ORIENT_HORZ ) {
		
		// gripper at left
		m_rectGripper.top		= gripper.top + 40;
		m_rectGripper.bottom	= gripper.bottom;
		m_rectGripper.left		= gripper.left;
		m_rectGripper.right	= gripper.left + 20;

		// draw close box
		m_rectClose.left = gripper.left + 7;
		m_rectClose.right = m_rectClose.left + 12;
		m_rectClose.top = gripper.top + 10;
		m_rectClose.bottom = m_rectClose.top + 12;
		dc.DrawFrameControl(m_rectClose, DFC_CAPTION, DFCS_CAPTIONCLOSE);

		// draw docking toggle box
		m_rectUndock = m_rectClose;
		m_rectUndock.OffsetRect(0,13);
		dc.DrawFrameControl(m_rectUndock, DFC_CAPTION, DFCS_CAPTIONMAX);

		gripper.top += 38;
		gripper.bottom -= 10;
		gripper.left += 10;
		gripper.right = gripper.left+3;
        dc.Draw3dRect( gripper, m_clrBtnHilight, m_clrBtnShadow );
		
		gripper.OffsetRect(4, 0);
        dc.Draw3dRect( gripper, m_clrBtnHilight, m_clrBtnShadow );
	}
	
	else {
		
		// gripper at top
		m_rectGripper.top		= gripper.top;
		m_rectGripper.bottom	= gripper.top + 20;
		m_rectGripper.left		= gripper.left;
		m_rectGripper.right		= gripper.right - 40;

		// draw close box
		m_rectClose.right = gripper.right - 10;
		m_rectClose.left = m_rectClose.right - 11;
		m_rectClose.top = gripper.top + 7;
		m_rectClose.bottom = m_rectClose.top + 11;
		dc.DrawFrameControl(m_rectClose, DFC_CAPTION, DFCS_CAPTIONCLOSE);


		// draw docking toggle box
		m_rectUndock = m_rectClose;
		m_rectUndock.OffsetRect(-13,0);
		dc.DrawFrameControl(m_rectUndock, DFC_CAPTION, DFCS_CAPTIONMAX);

		gripper.right -= 38;
		gripper.left += 10;
		gripper.top += 10;
		gripper.bottom = gripper.top+3;
		dc.Draw3dRect( gripper, m_clrBtnHilight, m_clrBtnShadow );
		
		gripper.OffsetRect(0, 4);
        dc.Draw3dRect( gripper, m_clrBtnHilight, m_clrBtnShadow );
	}

}

void CCoolDialogBar::OnNcLButtonDblClk(UINT nHitTest, CPoint point) 
{
    //MessageBox("ncldbl", "JMC", MB_OK | MB_ICONSTOP);
	// TODO: Add your message handler code here and/or call default
    if ((m_pDockBar != NULL) && (nHitTest == HTCAPTION))
    {
        // toggle docking
        ASSERT(m_pDockContext != NULL);
        m_pDockContext->ToggleDocking();
    }
    else
    {
        CWnd::OnNcLButtonDblClk(nHitTest, point);
    }
}



////////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////////
void CCoolDialogBar::Save()
{
	CString strSection("Docbar");
//vls-begin// multiple output
    if (m_wndCode > 0)
        strSection.Format(L"Docbar%d", m_wndCode+1);
//vls-end//
//	char bf[BUFFER_SIZE];
//	sprintf(bf,"at %d %d|%d %d|%d %d|%d",m_wndCode, m_sizeFloat.cx,m_sizeFloat.cy,m_sizeHorz.cx,m_sizeHorz.cy,m_sizeVert.cx,m_sizeVert.cy);
//	MessageBox(bf,"jmc",MB_OK|MB_ICONSTOP);

	///////////////////////////////////////////////////////////////////////////
    ::WritePrivateProfileInt(strSection, L"CXFloat", m_sizeFloat.cx, szGLOBAL_PROFILE);
	///////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
    ::WritePrivateProfileInt(strSection, L"CYFloat", m_sizeFloat.cy, szGLOBAL_PROFILE);

	///////////////////////////////////////////////////////////////////////////
    ::WritePrivateProfileInt(strSection, L"CXHorz", m_sizeHorz.cx, szGLOBAL_PROFILE);
	///////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
    ::WritePrivateProfileInt(strSection, L"CYHorz", m_sizeHorz.cy,szGLOBAL_PROFILE);

	///////////////////////////////////////////////////////////////////////////
    ::WritePrivateProfileInt(strSection, L"CXVert", m_sizeVert.cx,szGLOBAL_PROFILE);
	///////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
    ::WritePrivateProfileInt(strSection, L"CYVert", m_sizeVert.cy, szGLOBAL_PROFILE);

//vls-begin// multiple output
    ::WritePrivateProfileInt(strSection, L"Visible", m_bFlag, szGLOBAL_PROFILE);
    ::WritePrivateProfileInt(strSection, L"Docking", m_Dock, szGLOBAL_PROFILE);
    ::WritePrivateProfileInt(strSection, L"posX", m_mX, szGLOBAL_PROFILE);
    ::WritePrivateProfileInt(strSection, L"posY", m_mY, szGLOBAL_PROFILE);
    ::WritePrivateProfileString(strSection, L"Title", m_sTitle, szGLOBAL_PROFILE);
//vls-end//
}


////////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////////
void CCoolDialogBar::Load()
{
	CString strSection("Docbar");
//vls-begin// multiple output
    if (m_wndCode > 0)
        strSection.Format(L"Docbar%d", m_wndCode+1);
//vls-end//

	//////////////////////////////////////////////////////////////////////
    m_sizeFloat.cx = ::GetPrivateProfileInt(strSection, L"CXFloat", m_sizeFloat.cx, szGLOBAL_PROFILE);
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
    m_sizeFloat.cy = ::GetPrivateProfileInt(strSection, L"CYFloat", m_sizeFloat.cy,szGLOBAL_PROFILE);

	//////////////////////////////////////////////////////////////////////
    m_sizeHorz.cx = ::GetPrivateProfileInt(strSection, L"CXHorz", m_sizeHorz.cx,szGLOBAL_PROFILE);
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
    m_sizeHorz.cy = ::GetPrivateProfileInt(strSection, L"CYHorz", m_sizeHorz.cy, szGLOBAL_PROFILE);

	//////////////////////////////////////////////////////////////////////
    m_sizeVert.cx = ::GetPrivateProfileInt(strSection, L"CXVert", m_sizeVert.cx, szGLOBAL_PROFILE);
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
    m_sizeVert.cy = ::GetPrivateProfileInt(strSection, L"CYVert", m_sizeVert.cy, szGLOBAL_PROFILE);

//vls-begin// multiple output
    m_bFlag = ::GetPrivateProfileInt(strSection, L"Visible", FALSE, szGLOBAL_PROFILE);
    m_Dock = ::GetPrivateProfileInt(strSection, L"Docking", 0xF000L, szGLOBAL_PROFILE);
	EnableDocking(m_Dock);
    ShowWindow(m_bFlag ? SW_SHOW : SW_HIDE);
    // ShowWindow(m_bVisible ? SW_SHOW : SW_HIDE);

    CString t, sDefault;
    wchar_t sTitle[4096] = L"";
    t.LoadString(IDS_OUTPUT);
    sDefault.Format(t, m_wndCode);
    ::GetPrivateProfileString(strSection, L"Title", sDefault, sTitle, 4096, szGLOBAL_PROFILE);
    m_sTitle = sTitle;
    SetWindowText(sTitle);
//vls-end//
//* en
	m_mX = ::GetPrivateProfileInt(strSection, L"posX", 0, szGLOBAL_PROFILE);
	m_mY = ::GetPrivateProfileInt(strSection, L"posY", 0, szGLOBAL_PROFILE);
//*/en
}


//vls-begin// multiple output
void CCoolDialogBar::SetTitle(LPCTSTR sTitle)
{
    m_sTitle = sTitle;
    SetWindowText(sTitle);
	int dc = m_Dock;
    if (IsFloating()) {
		EnableDocking(CBRS_ALIGN_ANY);
        m_pDockContext->ToggleDocking();
        m_pDockContext->ToggleDocking();
		EnableDocking(dc);
    }
}
//vls-end//

void CCoolDialogBar::Resize(int Width, int Height)
{
	Width += m_cxBorder * 2 + m_cxEdge * 2;
	Height += m_cyBorder * 2 + m_cxEdge * 2;

	if (IsFloating()) {
		m_sizeFloat.cx = Width;
		m_sizeFloat.cy = Height;
	} else {
		int dsize;
		if (IsHorz()) {
			dsize = Width - m_sizeHorz.cx;
			m_sizeHorz.cx = Width;
			m_sizeHorz.cy = Height;
		} else {
			dsize = Height - m_sizeVert.cy;
			m_sizeVert.cx = Width;
			m_sizeVert.cy = Height;
		}

		int own;
		vector <CCoolDialogBar *> row = AllVisibleNeighbours(&own);
		for (int i = 0; i < row.size(); i++)
			if (i != own) {
				if (IsHorz())
					row[i]->m_sizeHorz.cy = m_sizeHorz.cy;
				else
					row[i]->m_sizeVert.cx = m_sizeVert.cx;
			}
		
		int other = own + 1;
		if (other >= row.size())
			other -= 2;
		if (other >= 0) {
			if (IsHorz()) {
				CRect rc;
				row[other]->GetWindowRect(rc);
				row[other]->m_sizeHorz.cx = max(row[other]->m_sizeHorz.cx - dsize, m_sizeMin.cx);
				rc.left += dsize;
				GetParent()->ScreenToClient(rc);
				row[other]->MoveWindow(rc);
			} else {
				CRect rc;
				row[other]->GetWindowRect(rc);
				row[other]->m_sizeVert.cy = max(row[other]->m_sizeVert.cy - dsize, m_sizeMin.cy);
				rc.top += dsize;
				GetParent()->ScreenToClient(rc);
				row[other]->MoveWindow(rc);
			}
		}

		m_pDockSite->RecalcLayout();
	}
}
