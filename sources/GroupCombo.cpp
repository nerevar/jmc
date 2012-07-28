// GroupCombo.cpp : implementation file
//

#include "stdafx.h"
#include "smc.h"
#include "GroupCombo.h"
#include "ttcoreex/tintinx.h"
#include "ttcoreex/ttobjects.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGroupCombo

CGroupCombo::CGroupCombo()
{
}

CGroupCombo::~CGroupCombo()
{
}

CList<CGroupCombo* , CGroupCombo*> CGroupCombo::m_GroupComboList;


BEGIN_MESSAGE_MAP(CGroupCombo, CComboBox)
	//{{AFX_MSG_MAP(CGroupCombo)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGroupCombo message handlers

int CGroupCombo::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CComboBox::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_GroupComboList.AddTail (this);
	
	return 0;
}

void CGroupCombo::Reinit(BOOL bAddAll )
{
    int OldSel = GetCurSel();
    if ( OldSel < 0 ) 
        OldSel = 0;
    ResetContent();
    
    int count;
    PPCGROUP ppGrp = GetGroupsList (&count);

    int ind;

    if ( bAddAll ) {
        CString t;
        t.LoadString(IDS_ALL_GROUPS);
        ind = AddString(t);
        SetItemData (ind, NULL);
    }

    for ( int i = 0 ; i < count ; i++ ) {
        ind = AddString(ppGrp[i]->m_strName.data() );
        SetItemData (ind, (DWORD)ppGrp[i]);
    }

    SetCurSel(min(OldSel, count-1));
}


void CGroupCombo::SelectGroup(void* p)
{
    PCGROUP pGrp = (PCGROUP)p;
    int Count = GetCount();
    for ( int i = 0 ; i < Count ; i++ ) {
        PCGROUP pg = (PCGROUP)GetItemData(i);
        if ( pg == pGrp ) {
            SetCurSel(i);
            return;
        }
    }
    ASSERT(0);
}

