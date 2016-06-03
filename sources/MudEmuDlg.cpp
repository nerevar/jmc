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
	m_strLogPath = _T("");
	m_bHandleRMA = FALSE;
	//}}AFX_DATA_INIT
    m_pBuff = NULL;
    m_nBufSize= m_nOffset = 0;
	m_hLogFile = INVALID_HANDLE_VALUE;
	m_dwTickStarted = m_dwTickPlayed = 0;
}


void CMudEmuDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMudEmuDlg)
	DDX_Control(pDX, IDC_BTN_STARTSTOP_EMUL_LOG, m_StartStopButton);
	DDX_Control(pDX, IDC_SEND, m_SendButton);
	DDX_Control(pDX, IDC_SEND_LINE, m_SendLineButton);
	DDX_Control(pDX, IDC_CHK_EMULATE_RMA, m_HandleRMAButton);
	DDX_Text(pDX, IDC_TEXT, m_strText);
	DDX_Text(pDX, IDC_EDIT_EMUL_LOG_PATH, m_strLogPath);
	DDX_Check(pDX, IDC_CHK_EMULATE_RMA, m_bHandleRMA);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMudEmuDlg, CDialog)
	//{{AFX_MSG_MAP(CMudEmuDlg)
	ON_BN_CLICKED(IDC_BTN_STARTSTOP_EMUL_LOG, OnStartStopLog)
	ON_BN_CLICKED(IDC_SEND, OnSend)
	ON_BN_CLICKED(IDC_SEND_LINE, OnSendLine)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMudEmuDlg message handlers

void CMudEmuDlg::OnStartStopLog() 
{
	UpdateData();

	if (m_pBuff != NULL) {
		KillTimer(1);
		ResetEvent(eventMudEmuTextArrives);
		delete[] m_pBuff;
		m_pBuff = NULL;
		m_StartStopButton.SetWindowText(L"Start");
		m_SendButton.EnableWindow(TRUE);
		m_SendLineButton.EnableWindow(TRUE);
		m_HandleRMAButton.EnableWindow(TRUE);
		return;
	}

    m_hLogFile  = CreateFile(m_strLogPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, NULL, NULL );
    if ( m_hLogFile != INVALID_HANDLE_VALUE ) { 
		m_StartStopButton.SetWindowText(L"Stop");
		m_SendButton.EnableWindow(FALSE);
		m_SendLineButton.EnableWindow(FALSE);
		m_HandleRMAButton.EnableWindow(FALSE);

		DWORD dwSize = GetFileSize(m_hLogFile, NULL);
        m_pBuff = new char[dwSize+2];
		ReadFile(m_hLogFile, m_pBuff, dwSize, &dwSize, NULL);
		m_pBuff[dwSize] = 0;
		CloseHandle(m_hLogFile);

		m_nOffset = 0;
		m_nBufSize = dwSize;

		m_dwTickStarted = GetTickCount();
		m_dwTickPlayed = 0;
		
		SendData();
	}
}

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
    m_pBuff = new char[(m_nBufSize+2)*sizeof(wchar_t)];
    wcscpy((wchar_t*)m_pBuff, m_strText);
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
    m_pBuff = new char[(m_nBufSize+2)*sizeof(wchar_t)];
    wcscpy((wchar_t*)m_pBuff, line);
    
    SendData();
}

void CMudEmuDlg::OnCancel() 
{
	ShowWindow(SW_HIDE);
	// CDialog::OnCancel();
}

void CMudEmuDlg::SendData()
{
    //EnableWindow(FALSE);
    SetTimer(1, 20, NULL);
}

void CMudEmuDlg::OnTimer(UINT nIDEvent) 
{
	ASSERT(nIDEvent == 1);

    if ( IsConnected() ) {
        ResetEvent(eventMudEmuTextArrives);
        delete[] m_pBuff;
        m_pBuff = NULL;
        KillTimer(1);
		m_StartStopButton.SetWindowText(L"Start");
		m_SendButton.EnableWindow(TRUE);
		m_SendLineButton.EnableWindow(TRUE);
		m_HandleRMAButton.EnableWindow(TRUE);
        EnableWindow(TRUE);
    }
    

    if ( WaitForSingleObject (eventMudEmuTextArrives, 0 ) == WAIT_TIMEOUT ) {
        // now fill buffer
		int max_length = sizeof(strMudEmuText) - 1 - nMudEmuTextSize;

		if ( m_bHandleRMA ) {
			int max_time = (GetTickCount() - m_dwTickStarted) - m_dwTickPlayed;
			int to_play = max_length;
			char *to_check = m_pBuff + m_nOffset;
			for (;;) {
				char *rma_cmd = strstr(to_check, "\x1bp:");
				if ( rma_cmd == NULL )
					break;
				to_play = rma_cmd - (m_pBuff + m_nOffset);
				if ( to_play > max_length )
					break;
				int time_ms = atoi(rma_cmd + 3);
				if ( time_ms > max_time )
					break;
				max_time -= time_ms;
				m_dwTickPlayed += time_ms;
				to_check = rma_cmd + 3;
				
			}
			if ( to_play < max_length )
				max_length = to_play;
		}

		if ( max_length > 0 ) {
			if ( m_nBufSize - m_nOffset <= max_length ) {
				memcpy(&strMudEmuText[nMudEmuTextSize], m_pBuff+m_nOffset, m_nBufSize - m_nOffset);
				nMudEmuTextSize += m_nBufSize - m_nOffset;
				m_nOffset = m_nBufSize;
			} else {
				memcpy(&strMudEmuText[nMudEmuTextSize], m_pBuff+m_nOffset, max_length);
				nMudEmuTextSize += max_length;
				m_nOffset += max_length;
			}
			SetEvent(eventMudEmuTextArrives);
		}

        if ( m_nOffset >= m_nBufSize ) {
            KillTimer(1);
            delete[] m_pBuff;
            m_pBuff= NULL;
			m_StartStopButton.SetWindowText(L"Start");
			m_SendButton.EnableWindow(TRUE);
			m_SendLineButton.EnableWindow(TRUE);
			m_HandleRMAButton.EnableWindow(TRUE);
            EnableWindow(TRUE);
        }
    }

	CDialog::OnTimer(nIDEvent);
}
