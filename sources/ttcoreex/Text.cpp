#include "stdafx.h"
#include "tintin.h"

extern wchar_t *get_arg_in_braces();
/**********************************/
/* load a file for input to mud.  */
/**********************************/
void read_file(wchar_t *arg)
{
  get_arg_in_braces(arg,arg,WITH_SPACES,sizeof(arg)/sizeof(wchar_t)-1);

  if ( !arg || !*arg ) {
	  tintin_puts2(rs::rs(1173));
	  return;
  }

  int size = read_file_contents(arg, NULL, 0);

  if (size <= 0) {
	wchar_t buff[MAX_PATH];
    swprintf (buff, rs::rs(1174), arg);
    tintin_puts2(buff);
    return;
  }

  wchar_t* buffer = new wchar_t[size];
  read_file_contents(arg, buffer, size);

  wchar_t* cptr = buffer;
  while(size > 0) {
    wchar_t *start = cptr;
    for(; *cptr && *cptr!=L'\n' && size > 0; cptr++, size--);
      *cptr=L'\0';
      if ( *start  ) 
		write_line_mud(start);
	  cptr++;
	  size--;
  }
  delete[] buffer;

  tintin_puts2(rs::rs(1175));
}


//* en
void spit_command(wchar_t *arg)
{
  wchar_t 
      filename[BUFFER_SIZE],
      spformat[BUFFER_SIZE],
      soptions[BUFFER_SIZE],
       *cptr1/*,*cptr2,*cptr3*/;
  BOOL
      bPassSpace = FALSE;  
  BOOL
	  bSilent    = FALSE;

  
  arg=get_arg_in_braces(arg,filename,STOP_SPACES,sizeof(filename)/sizeof(wchar_t)-1);
  arg=get_arg_in_braces(arg,spformat,WITH_SPACES,sizeof(spformat)/sizeof(wchar_t)-1);
  arg=get_arg_in_braces(arg,soptions,STOP_SPACES,sizeof(soptions)/sizeof(wchar_t)-1);

  if ( !filename || !*filename ) {
	  tintin_puts2(rs::rs(1173));
	  return;
  }

  if ( !spformat || !*spformat ) {
	  wcscpy(spformat,L"%0");
  }
  BOOL hasD = FALSE;
  for(cptr1=spformat;*cptr1&&*cptr1!=L'\0';cptr1++)
    if(*cptr1==L'%'&&*(cptr1+1)==L'0')
	  hasD = TRUE;
  if(!hasD) 
	  wcscpy(cptr1,L" %0\0");
  
  for(cptr1=soptions;*cptr1&&*cptr1!=L'\0';cptr1++)
  {
    if(*cptr1==L'n')
	  bPassSpace = TRUE;
	if(*cptr1==L's')
		bSilent = TRUE;
  }	 

  int size = read_file_contents(filename, NULL, 0);
  if(size <= 0) {
    wchar_t buff[MAX_PATH];
    swprintf(buff, rs::rs(1174), filename);
    tintin_puts2(buff);
    return;
  }
  
  wchar_t *buffer = new wchar_t[size];
  read_file_contents(filename, buffer, size);

  wchar_t* cptr = buffer;
  while(size > 0) {
	wchar_t cbuffer[BUFFER_SIZE];
    wchar_t *start = cptr;
    for(; *cptr && *cptr!=L'\n' && size > 0; cptr++, size--);
      *cptr=L'\0';
      if ( *start ) { 
		if ((*start == L'\n' || wcslen(start)<2)&&bPassSpace)
			continue;
		swprintf(vars[0], L"%ls", start);
        substitute_vars(spformat,cbuffer, sizeof(cbuffer)/sizeof(wchar_t));
		parse_input(cbuffer, TRUE);
	  }
	  cptr++;
	  size--;
  }

  delete[] buffer;
  
  if(!bSilent)
	  tintin_puts2(rs::rs(1175));
}

wchar_t lastGrab[BUFFER_SIZE] = L"\0";
int  lastLine = 0;
int  maxLines = 0;

int count_lines(wchar_t *fn)
{
	FILE *myfile;
  if((myfile=_wfopen(fn, L"r"))==NULL) {
    wchar_t buff[MAX_PATH];
    swprintf(buff, rs::rs(1174), fn);
    tintin_puts2(buff);
    return 0;
  }
  wchar_t buffer[BUFFER_SIZE];
  int ln = 0;
  while(fgetws(buffer, sizeof(buffer)/sizeof(wchar_t), myfile)) 
    ln++;
  fclose(myfile);
  return ln;
}

void grab_command(wchar_t *arg)
{
  wchar_t 
      arg1[BUFFER_SIZE],
      arg2[BUFFER_SIZE],
      arg3[BUFFER_SIZE],
    buffer[BUFFER_SIZE],
    result[BUFFER_SIZE],
   *cptr;
  FILE 
	  *myfile;
  int needLine = 0;

  BOOL wn = FALSE;
  
  arg=get_arg_in_braces(arg,arg1,STOP_SPACES,sizeof(arg1)/sizeof(wchar_t)-1);
  arg=get_arg_in_braces(arg,arg2,WITH_SPACES,sizeof(arg2)/sizeof(wchar_t)-1);
  arg=get_arg_in_braces(arg,arg3,WITH_SPACES,sizeof(arg3)/sizeof(wchar_t)-1);

  if(*arg1 && iswdigit(*arg1))
  {
	  wn = TRUE;
	  needLine = _wtoi(arg1);
  }
  else if(!wcsicmp(arg1,lastGrab))
	  needLine = lastLine+1;
  else
  {
	  needLine = 1;
      maxLines = count_lines(arg1);
  }

  if (((!wn)&&(!arg1||!*arg1))||((wn)&&(!arg2||!*arg2))) 
  {
	  tintin_puts2(rs::rs(1173));
	  return;
  }

  if (((!wn)&&(!arg2||!*arg2))||((wn)&&(!arg3||!*arg3))) 
  {
	  wcscpy(wn?arg3:arg2,L"%0\0");
  }
  BOOL hasD = FALSE;
  cptr=wn?arg3:arg2;
  for(;*cptr&&*cptr!=L'\0';cptr++)
    if(*cptr==L'%'&&*(cptr+1)==L'0')
	  hasD = TRUE;
  if(!hasD) 
	  wcscpy(cptr,L" %0\0");

  if((myfile=_wfopen(wn?arg2:arg1, L"r"))==NULL) {
    wchar_t buff[MAX_PATH];
    swprintf(buff, rs::rs(1174), wn?arg2:arg1);
    tintin_puts2(buff);
    return;
  }
  int currLine = 0;
  while(fgetws(buffer, sizeof(buffer)/sizeof(wchar_t), myfile)) 
  {
    currLine++;
	if(currLine == needLine)
	{
		    for(cptr=buffer;*cptr&&*cptr!=L'\n';cptr++);
			*cptr=L'\0';
            swprintf(vars[0], L"%ls", buffer);
            substitute_vars(wn?arg3:arg2,result, sizeof(result)/sizeof(wchar_t));
			parse_input(result, TRUE);
			break;
	}
  }
  lastLine = (needLine != maxLines)?needLine:0;
  wcscpy(lastGrab,wn?arg2:arg1);
  fclose(myfile);
}
//*/en
