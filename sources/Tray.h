#define ID_TRAY 55777

class CTray  
{
public:
	BOOL isInTray()	{return (isInSysTray);}

	//void ToolTip (CString clToolTip);
	//void Icon (HICON hIcon = NULL);
	//void Init (CWnd *pclParent, CString clToolTip = "");
	//void ShowInTray (bool bShow = true, bool bUpdateWindow = false);

	CTray(int nIconID, LPCTSTR szTip);
	CTray();
	//virtual ~CTray();

	BOOL add();
	BOOL remove();

private :
	NOTIFYICONDATA iconData;
	BOOL isInSysTray;
};