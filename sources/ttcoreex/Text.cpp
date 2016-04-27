#include "stdafx.h"
#include "tintin.h"

extern char *get_arg_in_braces();
/**********************************/
/* load a file for input to mud.  */
/**********************************/
void read_file(char *arg)
{
  FILE *myfile;
  char buffer[BUFFER_SIZE], *cptr;
  get_arg_in_braces(arg, arg, WITH_SPACES);

  if ( !arg || !*arg ) {
	  tintin_puts2(rs::rs(1173));
	  return;
  }


  if((myfile=fopen(arg, "r"))==NULL) {
    char buff[MAX_PATH];
    sprintf (buff, rs::rs(1174), arg);
    tintin_puts2(buff);
    return;
  }
  while(fgets(buffer, sizeof(buffer), myfile)) {
    for(cptr=buffer; *cptr && *cptr!='\n'; cptr++);
    *cptr='\0';
    write_line_mud(buffer);
  }
  fclose(myfile);
  tintin_puts2(rs::rs(1175));
}


//* en
void spit_command(char *arg)
{
  char 
      filename[BUFFER_SIZE],
      spformat[BUFFER_SIZE],
      soptions[BUFFER_SIZE],
        buffer[BUFFER_SIZE],
       *cptr1/*,*cptr2,*cptr3*/;
  FILE 
	  *myfile;
  BOOL
      bPassSpace = FALSE;  
  BOOL
	  bSilent    = FALSE;

  
  arg=get_arg_in_braces(arg, filename, STOP_SPACES);
  arg=get_arg_in_braces(arg, spformat, WITH_SPACES);
  arg=get_arg_in_braces(arg, soptions, STOP_SPACES);

  if ( !filename || !*filename ) {
	  tintin_puts2(rs::rs(1173));
	  return;
  }

  if ( !spformat || !*spformat ) {
	  strcpy(spformat,"%0");
  }
  BOOL hasD = FALSE;
  for(cptr1=spformat;*cptr1&&*cptr1!='\0';cptr1++)
    if(*cptr1=='%'&&*(cptr1+1)=='0')
	  hasD = TRUE;
  if(!hasD) 
	  strcpy(cptr1," %0\0");
  
  for(cptr1=soptions;*cptr1&&*cptr1!='\0';cptr1++)
  {
    if(*cptr1=='n')
	  bPassSpace = TRUE;
	if(*cptr1=='s')
		bSilent = TRUE;
  }	 

  if((myfile=fopen(filename, "r"))==NULL) {
    char buff[MAX_PATH];
    sprintf(buff, rs::rs(1174), filename);
    tintin_puts2(buff);
    return;
  }
  while(fgets(buffer, sizeof(buffer), myfile)) 
  {
	char cbuffer[BUFFER_SIZE];
    
    if((*buffer=='\n'||strlen(buffer)<2)&&bPassSpace)
	  continue;

/*
    for(cptr1=spformat,cptr3=cbuffer; *cptr1 && *cptr1!='\0'; cptr1++,cptr3++)
      if(*cptr1=='@')
      {
		 for(cptr2=buffer;*cptr2&&*cptr2!='\0'&&*cptr2!='\n';cptr2++,cptr3++)
			 *cptr3=*cptr2;
		 cptr3--;
	  }
	  else
        *cptr3=*cptr1;
	*cptr3='\0';
*/
		    for(cptr1=buffer;*cptr1&&*cptr1!='\n';cptr1++);
			*cptr1='\0';
            sprintf(vars[0], "%s", buffer);
            substitute_vars(spformat,cbuffer, sizeof(cbuffer));
    parse_input(cbuffer, TRUE);
  }
  fclose(myfile);
  if(!bSilent)
	  tintin_puts2(rs::rs(1175));
}

char lastGrab[BUFFER_SIZE] = "\0";
int  lastLine = 0;
int  maxLines = 0;

int count_lines(char *fn)
{
	FILE *myfile;
  if((myfile=fopen(fn, "r"))==NULL) {
    char buff[MAX_PATH];
    sprintf(buff, rs::rs(1174), fn);
    tintin_puts2(buff);
    return 0;
  }
  char buffer[BUFFER_SIZE];
  int ln = 0;
  while(fgets(buffer, sizeof(buffer), myfile)) 
    ln++;
  fclose(myfile);
  return ln;
}

void grab_command(char *arg)
{
  char 
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
  
  arg=get_arg_in_braces(arg, arg1, STOP_SPACES);
  arg=get_arg_in_braces(arg, arg2, WITH_SPACES);
  arg=get_arg_in_braces(arg, arg3, WITH_SPACES);

  if(*arg1 && isdigit(*arg1))
  {
	  wn = TRUE;
	  needLine = atoi(arg1);
  }
  else if(!_strcmpi(arg1,lastGrab))
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
	  strcpy(wn?arg3:arg2,"%0\0");
  }
  BOOL hasD = FALSE;
  cptr=wn?arg3:arg2;
  for(;*cptr&&*cptr!='\0';cptr++)
    if(*cptr=='%'&&*(cptr+1)=='0')
	  hasD = TRUE;
  if(!hasD) 
	  strcpy(cptr," %0\0");

  if((myfile=fopen(wn?arg2:arg1, "r"))==NULL) {
    char buff[MAX_PATH];
    sprintf(buff, rs::rs(1174), wn?arg2:arg1);
    tintin_puts2(buff);
    return;
  }
  int currLine = 0;
  while(fgets(buffer, sizeof(buffer), myfile)) 
  {
    currLine++;
	if(currLine == needLine)
	{
		    for(cptr=buffer;*cptr&&*cptr!='\n';cptr++);
			*cptr='\0';
            sprintf(vars[0], "%s", buffer);
            substitute_vars(wn?arg3:arg2,result, sizeof(result));
			parse_input(result, TRUE);
			break;
	}
  }
  lastLine = (needLine != maxLines)?needLine:0;
  strcpy(lastGrab,wn?arg2:arg1);
  fclose(myfile);
}
//*/en
