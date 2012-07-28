// CommonDocumentParamsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "smc.h"
#include "ColorButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CColourSetButton

CColourSetButton::CColourSetButton()
{
    m_cCurrColour = ::GetSysColor(COLOR_WINDOW );
}

CColourSetButton::~CColourSetButton()
{
}


BEGIN_MESSAGE_MAP(CColourSetButton, CButton)
	//{{AFX_MSG_MAP(CColourSetButton)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColourSetButton message handlers

void CColourSetButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
    CRect rect;
    GetClientRect(&rect);
    
    CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
    CBrush br(m_cCurrColour);

    // pDC->FillRect(&rect , &br );
    COLORREF oldcol = pDC->SetBkColor(m_cCurrColour);
    COLORREF oldtextcol = pDC->SetTextColor(0xffffff - m_cCurrColour);

    CPen *oldPen , wPen(PS_SOLID , 1 , RGB(255 , 255 , 255)) , bPen(PS_SOLID , 1 , RGB(0,0,0));
    
    if ( !(lpDrawItemStruct->itemState & ODS_SELECTED) ) {
        oldPen = (CPen*)pDC->SelectObject(&wPen);
        pDC->MoveTo(0, 0 );
        pDC->LineTo(rect.right , 0 );
        pDC->MoveTo(0, 0 );
        pDC->LineTo(0 , rect.bottom );

        pDC->MoveTo(1, 1 );
        pDC->LineTo(rect.right-1 , 1 );
        pDC->MoveTo(1, 1 );
        pDC->LineTo(1 , rect.bottom-1 );
        
        pDC->SelectObject(&bPen);

        pDC->MoveTo(rect.right-1, rect.bottom -1);
        pDC->LineTo(0, rect.bottom -1);
        pDC->MoveTo(rect.right-1, rect.bottom -1);
        pDC->LineTo(rect.right-1,0);

        pDC->MoveTo(rect.right-2, rect.bottom-2 );
        pDC->LineTo(1, rect.bottom -2);
        pDC->MoveTo(rect.right-2 , rect.bottom -2);
        pDC->LineTo(rect.right-2,1);
        pDC->SelectObject(oldPen);
    }
    else {
        oldPen = (CPen*)pDC->SelectObject(&wPen);
        pDC->MoveTo(0, 0 );
        pDC->LineTo(rect.right , 0 );
        pDC->MoveTo(0, 0 );
        pDC->LineTo(0 , rect.bottom );

        pDC->SelectObject(&bPen);

        pDC->MoveTo(1, 1 );
        pDC->LineTo(rect.right-1 , 1 );
        pDC->MoveTo(1, 1 );
        pDC->LineTo(1 , rect.bottom-1 );
        
        pDC->SelectObject(&wPen);

        pDC->MoveTo(rect.right-1, rect.bottom -1);
        pDC->LineTo(0, rect.bottom-1 );
        pDC->MoveTo(rect.right-1, rect.bottom-1 );
        pDC->LineTo(rect.right-1,0);

        pDC->SelectObject(&bPen);

        pDC->MoveTo(rect.right-2, rect.bottom-2);
        pDC->LineTo(1, rect.bottom -2);
        pDC->MoveTo(rect.right-2, rect.bottom-2);
        pDC->LineTo(rect.right-2,1);
        pDC->SelectObject(oldPen);
    }

    if ( lpDrawItemStruct->itemState & ODS_FOCUS) {
        CRect rect1 = rect;
        rect1.InflateRect(4 , 4 );
        pDC->DrawFocusRect(&rect1);
    }


    CString str;
    GetWindowText(str);
    // pDC->DrawText(str , &rect , DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    CSize size = pDC->GetTextExtent(str);
    rect.InflateRect(-3,-3);
    pDC->ExtTextOut( (rect.Width()-size.cx)/2+3 , (rect.Height()-size.cy)/2+3, ETO_OPAQUE , &rect, str, NULL);
    pDC->SetBkColor(oldcol);
    pDC->SetTextColor(oldtextcol);

}

BOOL CColourSetButton::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult) 
{
    if ( message == WM_COMMAND ) {
        CColorDialog dlg (m_cCurrColour , 0 , this);
        if ( dlg.DoModal() == IDOK ) {
            m_cCurrColour = dlg.GetColor();
            InvalidateRect(NULL);
            UpdateWindow();
        }
        return true;
    }
	return CButton::OnChildNotify(message, wParam, lParam, pLResult);
}

