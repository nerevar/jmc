/* Autoconf patching by David Hedbor, neotron@lysator.liu.se */
/*********************************************************************/
/* file: parse.c - some utility-functions                            */
/*                             TINTIN III                            */
/*          (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t             */
/*                     coded by peter unold 1992                     */
/*********************************************************************/
#include "stdafx.h"
#include "tintin.h"
//#include "cmds.h"
extern jmc_cmdi jmc_cmds[JMC_CMDS_NUM];
bool parse_tintin_command(char *command, char *arg);
void do_speedwalk(char *cp) ;

void all_command();
void read_command();
void session_command();
void write_command();
void writesession_command();
void zap_command();
extern char *space_out();
void write_com_arg_mud(char* command, char* argument);
char *get_arg_all(char *s, char *arg);
char *get_arg_with_spaces(char * s, char* arg);
char *get_arg_stop_spaces(char *s, char *arg);
extern char *cryptkey;
extern char mQueue[BUFFER_SIZE];
extern char cCommentChar;
extern void tstphandler();
void prefix_command(char *arg);

char s_prefix[BUFFER_SIZE];


/**************************************************************************/
/* parse input, check for TINTIN commands and aliases and send to session */
/**************************************************************************/
void parse_input(char *input)
{
    char command[BUFFER_SIZE], arg[BUFFER_SIZE], result[BUFFER_SIZE];
    char *input2;

    bPasswordEcho = TRUE;

    if(*input=='\0') {
        write_line_mud("");
        return ; 
    }

    if ( verbatim  && *input == cCommandChar && *(input+1) == 'v' ) {// check verbatim command
        char command[BUFFER_SIZE];
        char* input2=get_arg_stop_spaces(input+1, command);
        if(command[0] == 'v' && is_abrev(command, "verbatim")){
            char arg[BUFFER_SIZE];
            get_arg_all(input2, arg);
            verbatim_command(arg);
            return;
        }
    }
    
    if (verbatim ) {
        write_line_mud(input);
        return ;
    }
  
    if (*input==verbatim_char) {
        input++;
        write_line_mud(input);
        return ;
    }
    substitute_myvars(input, result);
    input2=result;
    while(*input2) {
//* en:colon
        // if(*input2==';')
        if(*input2==cCommandDelimiter||(bColon && *input2==';')&&*(input2-1)!='\\')
            input2++;
//*/en
        input2=get_arg_stop_spaces(input2, command);
        input2=get_arg_all(input2, arg);

//* en:prefix
		if(s_prefix[0] && command[0] != cCommandChar)
		{
			char p_command[BUFFER_SIZE];
			strcpy(p_command,command);
			strcat(p_command," ");
			strcat(p_command,arg);
			strcpy(command,s_prefix);
			strcpy(arg,p_command);
		}
//*/en
        if(*command==cCommandChar) 
		{
			if (bDisplayCommands) {
				// output command in square brackets
				char strInputCommand[BUFFER_SIZE], strOutputBuffer[BUFFER_SIZE];
				strcpy(strInputCommand, "\r[");
				strcat(strInputCommand, command);
				if (*arg != '\0') {
					strcat(strInputCommand, " ");
					strcat(strInputCommand, arg);
				}
				strcat(strInputCommand, "]");

				add_codes(strInputCommand, strOutputBuffer, "brown", TRUE);

				tintin_puts2(strOutputBuffer);
			}

            parse_tintin_command(command+1, arg);
		}
//* en:comments
        else if(*command==cCommentChar);
//*/en
        else
		{
//* en:waits
          if(iWaitState>0)
		  {
		   char *ptr1,*ptr2;
     	   for(ptr1=mQueue;*ptr1&&*ptr1!='\0';ptr1++);
	       *ptr1++=cCommandDelimiter;
           for(ptr2=command;*ptr2&&*ptr2!='\0';ptr2++,ptr1++)
             *ptr1=*ptr2;
		   if(*arg)
			   *ptr1++=' ';
           for(ptr2=arg;*ptr2&&*ptr2!='\0';ptr2++,ptr1++)
             *ptr1=*ptr2;
		   *ptr1++='\0';
		   continue;
		  }
//* en

          ALIAS_INDEX ind;
			
		  if( (ind=AliasList.find(command)) != AliasList.end() && ind->second->m_pGroup->m_bEnabled ) 
		  {
            
            int i;
            char *cpsource, *cpsource2, newcommand[BUFFER_SIZE], end;

            strcpy(vars[0], arg);

            for(i=1, cpsource=arg; i<10; i++) 
			{
                /* Next lines CHANGED to allow argument grouping with aliases */
                while (*cpsource == ' ')
                    cpsource++;
                end = (*cpsource == '{') ? '}' : ' ';
                cpsource = (*cpsource == '{') ? cpsource+1 : cpsource;
                for(cpsource2=cpsource; *cpsource2 && *cpsource2!=end; cpsource2++);
                strncpy(vars[i], cpsource, cpsource2-cpsource);
                *(vars[i]+(cpsource2-cpsource))='\0';
                cpsource=(*cpsource2) ? cpsource2+1 : cpsource2;
            }
            ALIAS* pal = ind->second;
            prepare_actionalias((char*)pal->m_strRight.data(), newcommand); 
            if(!strcmp(pal->m_strRight.data(), newcommand) && *arg) 
			{
                strcat(newcommand, " "); 
                strcat(newcommand, arg);
            }

            parse_input(newcommand);
		  }
          else 
		   if(speedwalk && !*arg && is_speedwalk_dirs(command))
            do_speedwalk(command);
           else 
		   {
                get_arg_with_spaces(arg,arg);
                write_com_arg_mud(command, arg);
           }
		}
    }
    return;
}

/**********************************************************************/
/* return TRUE if commands only consists of capital letters N,S,E ... */
/**********************************************************************/
int is_speedwalk_dirs(char *cp)
{
  char command[2];
  int flag;
  flag=FALSE;

  while(*cp) {
      command[0] = *cp;
      command[1] = 0;
    if(*cp!='n' && *cp!='e' && *cp!='s' && *cp!='w' && *cp!='u' && *cp!='d' &&
    !isdigit(*cp) && !searchnode_list(common_pathdirs, cp) )
      return FALSE;
    if(!isdigit(*cp))
      flag=TRUE;
    cp++;
  }
  return flag;
}

/**************************/
/* do the speedwalk thing */
/**************************/
void do_speedwalk(char *cp)
{
  char sc[2];
  char *loc; 
  int multflag,loopcnt,i;
  strcpy(sc, "x");
  while(*cp) {
    loc=cp;
    multflag=FALSE;
    while(isdigit(*cp)) {
      cp++;
      multflag=TRUE;
    }
    if(multflag && *cp) {
      sscanf(loc,"%d%c",&loopcnt,sc);
      i=0;
      while(i++<loopcnt)
    	write_com_arg_mud(sc, "");
    }
    else if (*cp) {
      sc[0]=*cp;
      write_com_arg_mud(sc, "");
    }
    /* Added the if to make sure we didn't move the pointer outside the 
       bounds of the origional pointer.  Corrects the bug with speedwalking
       where if you typed "u7" tintin would go apeshit. (JE)
    */
    if (*cp)
       cp++;
  }
}

/*************************************/
/* parse most of the tintin-commands */
/*************************************/
bool parse_tintin_command(char *command, char *arg)
{
  /*
   this lines almost totally rewrote to put all command functions
   into a struct array. Size of array defined in tintin.h;
   Array itself defined in cmds.h.
   Struct:
    char alias    - with it you can easily add aliases with same function name
	void function_name - main stuff
	char hlpfile  - this field is for help_command. do not forget to link 
	                aliases to one help file.
	would rely it would not crash... En.
  */
  CharLower(command);

  if(isdigit(*command)) {
	  int a1=0;
	  int a2=0;
  	  sscanf(command,"%d:%d",&a1,&a2);
	  get_arg_in_braces(arg, arg, WITH_SPACES);

    do_cycle(      1,     a1,    1,         a2,  arg    );
 /* do_cycle( bound1, bound2, step,      delay,  command); */
    return false;
  }

  if(*command == cCommandChar) return false;

  for(int i=0;i<JMC_CMDS_NUM;i++)
	  if((command[0]==jmc_cmds[i].alias[0])
		  &&(is_abrev(command,jmc_cmds[i].alias)))
	  {
		  (*(jmc_cmds[i].jmcfn))(arg);
		  return true;
	  }
  
  tintin_puts2(rs::rs(1145));
  return false;
}


/**********************************************/
/* get all arguments - don't remove "s and \s */
/**********************************************/
char *get_arg_all(char *s, char *arg)
{
  /* int inside=FALSE; */
  int nest=0;
  s=space_out(s);
  while(*s) {

    if(*s=='\\') {
      *arg++=*s++;
      if(*s)
	*arg++=*s++;
    }

    // else if(*s==';' && nest<1) {
	else if((*s==cCommandDelimiter||(bColon && *s==';'&&*(s-1)!='\\')) && nest<1) {
	break;
    }

    else if(*s==DEFAULT_OPEN) {
      nest++;
      *arg++=*s++;
    }

    else if(*s==DEFAULT_CLOSE) {
      nest--;
      *arg++=*s++;
    }

    else
      *arg++=*s++;
  }

  *arg='\0';
  return s;
}

/**************************************/
/* get all arguments - remove "s etc. */
/* Example:                           */
/* In: "this is it" way way hmmm;     */
/* Out: this is it way way hmmm       */ 
/**************************************/
char *get_arg_with_spaces(char * s, char* arg)
{
  int nest=0;
  /* int inside=FALSE; */

  s=space_out(s);
  while(*s) {

    if(*s=='\\') {
      if(*++s)
	*arg++=*s++;
    }

    // else if(*s==';' && nest==0) {
	else if((*s==cCommandDelimiter||(bColon && *s==';'&&*(s-1)!='\\')) && nest==0) {
	break;
    }
    else if(*s==DEFAULT_OPEN) {
      nest++;
      *arg++=*s++;
    }
    else if(*s==DEFAULT_CLOSE) {
      *arg++=*s++;
      nest--;
    }
    else
      *arg++=*s++;
  }
*arg='\0'; 
return s;
}
/********************/
/* my own routine   */
/********************/
char *get_arg_in_braces(char *s, char *arg, int flag)
{
   int nest=0;
   char *ptr;
   s=space_out(s);
   ptr=s;
   if (*s!=DEFAULT_OPEN) {
     if (flag==0) 
       s=get_arg_stop_spaces(ptr,arg);
     else
       s=get_arg_with_spaces(ptr,arg);
     return s;
   }
   s++;
     while(*s!='\0' && !(*s==DEFAULT_CLOSE && nest==0)) {
       if(*s==DEFAULT_OPEN) {
         nest++;
       }
     
       else if(*s==DEFAULT_CLOSE) {
         nest--;
       }                                    
       *arg++=*s++;
     }
   if (!*s)
     tintin_puts2(rs::rs(1146));
   else
     s++;
   *arg='\0';
   return s;
   
}
/**********************************************/
/* get one arg, stop at spaces                */
/* remove quotes                              */
/**********************************************/
char *get_arg_stop_spaces(char *s, char *arg)
{
  int inside=FALSE;
  s=space_out(s);
  
  while(*s) {
    if(*s=='\\') {
      if(*++s)
	*arg++=*s++;
    }
    else if(*s=='"') {
      s++;
      inside=!inside;
    }

    // else if(*s==';') {
	else if(*s==cCommandDelimiter||(bColon && *s==';' &&*(s-1)!='\\')) {
      if(inside)
	*arg++=*s++;
      else
	break;
    }

    else if(!inside && *s==' ')
      break;
    else
      *arg++=*s++;
  }

  *arg='\0';
  return s;
}

/*********************************************/
/* spaceout - advance ptr to next none-space */
/* return: ptr to the first none-space       */
/*********************************************/ 
char *space_out(char *s)
{
  while(isspace(*s))
    s++;
  return s;
}

/************************************/
/* send command+argument to the mud */
/************************************/
void write_com_arg_mud(char* command, char* argument)
{
    char outtext[BUFFER_SIZE];
    int i;

    check_insert_path(command);
    strncpy(outtext, command, BUFFER_SIZE);
    if(*argument) {
      strncat(outtext, " ", BUFFER_SIZE-strlen(command)-1); 
      strncat(outtext, argument, BUFFER_SIZE-strlen(command)-2);
    }
    write_line_mud(outtext);
    i=strlen(outtext);
    outtext[i++]='\r';
    outtext[i++]='\n';
}


//* en
void prefix_command(char *arg)
{
 space_out(arg);
 strcpy(s_prefix,arg);
 char sn[BUFFER_SIZE];
 sprintf(sn,rs::rs(1255),arg);
 tintin_puts2(sn);
}
//*/en