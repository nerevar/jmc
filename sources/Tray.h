#define ID_TRAY 55777

class CTray  
{
public:
	BOOL isInTray()	{return (isInSysTray);}

	CTray(int nIconID, LPCTSTR szTip);
	CTray();

	BOOL add();
	BOOL remove();

private :
	NOTIFYICONDATA iconData;
	BOOL isInSysTray;
};