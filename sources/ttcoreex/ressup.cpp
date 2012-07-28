#include "StdAfx.h"
#include "ressup.h"
#include "tintin.h"
// --CHANGED by Anton Likhtarov for resource support
HINSTANCE rs::hInst;
char DLLEXPORT langfile[BUFFER_SIZE];
char DLLEXPORT langsect[BUFFER_SIZE];

char* rs::rs(int StrId) {
    static char buf[256];
//* en:loading interface from .ini
	char bf[32]; 
	char fn[MAX_PATH+2];
	sprintf(bf,"str%d",StrId);
	if(langfile[1] != ':')
	{
	    MakeAbsolutePath(fn, langfile, szBASE_DIR);
      ::GetPrivateProfileString(langsect,bf,"",buf, 256, fn);
	}else{
	  ::GetPrivateProfileString(langsect,bf,"",buf, 256, langfile);
	}
	if(buf[0]) 
	{
		//now replacing all '\\n' with real CR
		char *ptr;
		for(ptr=buf;*ptr!='\0';ptr++)
			if(*ptr=='\\'&&*(ptr+1)=='n')
			{
				*ptr++=' ';
				*ptr='\n';
			}
		return buf;
	}
//*/en
    if(hInst) return LoadString(hInst, StrId, buf, sizeof(buf)) ? buf : NULL;
    return NULL;
}
// --END
