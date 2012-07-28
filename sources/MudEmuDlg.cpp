// MudEmuDlg.cpp : implementation file
//

#include "stdafx.h"
#include "smc.h"
#include "MudEmuDlg.h"

#include "ttcoreex\\ttcoreex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMudEmuDlg dialog


CMudEmuDlg::CMudEmuDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMudEmuDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMudEmuDlg)
	m_strText = _T("");
	//}}AFX_DATA_INIT
    m_pBuff = NULL;
    m_nBufSize= m_nOffset = 0;

}


void CMudEmuDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMudEmuDlg)
	DDX_Text(pDX, IDC_TEXT, m_strText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMudEmuDlg, CDialog)
	//{{AFX_MSG_MAP(CMudEmuDlg)
	ON_BN_CLICKED(IDC_SEND, OnSend)
	ON_BN_CLICKED(IDC_SEND_LINE, OnSendLine)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMudEmuDlg message handlers

void CMudEmuDlg::OnSend() 
{
    if ( IsConnected() ) {
        CString t;
        t.LoadString(IDS_ERR_DISCONNECT);
        MessageBox (t, ::AfxGetAppName(), MB_OK | MB_ICONSTOP);
        return;
    }

    UpdateData();

    if ( m_strText.GetLength () <= 0 ) {
        MessageBeep(MB_OK);
        return;
    }

    m_nBufSize = m_strText.GetLength ();
    m_nOffset = 0;
    m_pBuff = new char[m_nBufSize+2];
    strcpy(m_pBuff, (LPCSTR)m_strText);
    m_strText.Empty ();
    UpdateData(FALSE);
    
    SendData();
}

void CMudEmuDlg::OnSendLine() 
{
    if ( IsConnected() ) {
        CString t;
        t.LoadString(IDS_ERR_DISCONNECT);
        MessageBox (t, ::AfxGetAppName(), MB_OK | MB_ICONSTOP);
        return;
    }

    UpdateData();

    if ( m_strText.GetLength () <= 0 ) {
        MessageBeep(MB_OK);
        return;
    }

    int pos = m_strText.Find ('\n');
    CString line;
    if ( pos < 0 ) {
        line = m_strText;
        m_strText.Empty ();
    } else {
        line = m_strText.Left (pos+1);
        m_strText = m_strText.Right (m_strText.GetLength () - pos -1);
    }

    UpdateData(FALSE);

    m_nBufSize = line.GetLength ();
    m_nOffset = 0;
    m_pBuff = new char[m_nBufSize+2];
    strcpy(m_pBuff, (LPCSTR)line);
    
    SendData();
}

void CMudEmuDlg::OnCancel() 
{
	ShowWindow(SW_HIDE);
	// CDialog::OnCancel();
}

void CMudEmuDlg::SendData()
{
    EnableWindow(FALSE);
    SetTimer(1, 200, NULL);
}


void CMudEmuDlg::OnTimer(UINT nIDEvent) 
{
	ASSERT(nIDEvent == 1);

    if ( IsConnected() ) {
        ResetEvent(eventMudEmuTextArrives);
        delete m_pBuff;
        m_pBuff = NULL;
        KillTimer(1);
        EnableWindow(TRUE);
    }
    

    if ( WaitForSingleObject (eventMudEmuTextArrives, 0 ) == WAIT_TIMEOUT ) {
        // now fill buffer
        if ( m_nBufSize - m_nOffset <=512 ) {
            memcpy(strMudEmuText, m_pBuff+m_nOffset, m_nBufSize - m_nOffset);
            nMudEmuTextSize = m_nBufSize - m_nOffset;
            m_nOffset = m_nBufSize;
        } else {
            memcpy(strMudEmuText, m_pBuff+m_nOffset, 512);
            nMudEmuTextSize = 512;
            m_nOffset += 512;
        }
        SetEvent(eventMudEmuTextArrives);
        if ( m_nOffset >= m_nBufSize ) {
            KillTimer(1);
            delete m_pBuff;
            m_pBuff= NULL;
            EnableWindow(TRUE);
        }
    }

	CDialog::OnTimer(nIDEvent);
}
