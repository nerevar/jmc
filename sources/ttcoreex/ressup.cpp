#include "StdAfx.h"
#include "ressup.h"
#include "tintin.h"
// --CHANGED by Anton Likhtarov for resource support
HINSTANCE rs::hInst;
wchar_t DLLEXPORT langfile[BUFFER_SIZE];
wchar_t DLLEXPORT langsect[BUFFER_SIZE];

wchar_t* rs::rs(int StrId) {
    static wchar_t buf[256];
//* en:loading interface from .ini
	wchar_t bf[32]; 
	wchar_t fn[MAX_PATH+2];
	swprintf(bf,L"str%d",StrId);
	if(langfile[1] != L':')
	{
	    MakeAbsolutePath(fn, langfile, szBASE_DIR);
      ::GetPrivateProfileString(langsect,bf,L"",buf, 256, fn);
	}else{
	  ::GetPrivateProfileString(langsect,bf,L"",buf, 256, langfile);
	}
	if(buf[0]) 
	{
		//now replacing all '\\n' with real CR
		wchar_t *ptr;
		for(ptr=buf;*ptr!=L'\0';ptr++)
			if(*ptr==L'\\'&&*(ptr+1)==L'n')
			{
				*ptr++=L' ';
				*ptr=L'\n';
			}
		return buf;
	}
//*/en
    if(hInst) return LoadString(hInst, StrId, buf, sizeof(buf)) ? buf : NULL;
    return NULL;
}
// --END
