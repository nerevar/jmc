#include "stdafx.h"
#include "Tray.h"

CTray::CTray()
{
	// do nothing
}


CTray::CTray(int nIconID, LPCTSTR szTip)
{
	memset (&iconData, 0, sizeof(iconData));
	
	iconData.cbSize = sizeof(iconData);
	iconData.hWnd = AfxGetMainWnd()->m_hWnd;
	iconData.uID = ID_TRAY;
	iconData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	iconData.uCallbackMessage = WM_USER + 701;
	iconData.hIcon = ::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(nIconID));
	iconData.hWnd = AfxGetMainWnd()->m_hWnd;
	strcpy(iconData.szTip, szTip);

	isInSysTray = FALSE;
}

BOOL CTray::add()
{
	isInSysTray = TRUE;

	return Shell_NotifyIcon(NIM_ADD, &iconData);
}

BOOL CTray::remove()
{
	isInSysTray = FALSE;

	return Shell_NotifyIcon(NIM_DELETE, &iconData);
}