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

extern BOOL bReadingConfig;

static char *pstack = NULL;
static int stack_ptr = 0, stack_capacity = 0, stack_length = 0;
static BOOL parsing_stack = FALSE;

void free_parse_stack() {
	if (pstack)
		free(pstack);
	pstack = NULL;
	stack_ptr = stack_capacity = stack_length = 0;
}

static BOOL stack_maintain(int need_capacity) {
	if( pstack && need_capacity <= stack_capacity ) {
		return TRUE;
	}
	int to_allocate = need_capacity + BUFFER_SIZE*2;
	if( !pstack ) {
		pstack = (char*)malloc(to_allocate);
		
		stack_length = 0;
		if( pstack ) {
			stack_ptr = BUFFER_SIZE;
			pstack[stack_ptr] = '\0';
		}
	} else {
		char *new_ptr = (char*)realloc(pstack, to_allocate);
		if( !new_ptr )
			free(pstack);
		pstack = new_ptr;
	}
	if( pstack ) {
		stack_capacity = to_allocate;
		return TRUE;
	} else {
		stack_capacity = stack_length = stack_ptr = 0;
		tintin_puts("#Error: out of memory");
		return FALSE;
	}
}
static char* stack_top() {
	return pstack + stack_ptr;
}
static BOOL stack_push_front(char *arg) {
	int len = strlen(arg);
	
	if( stack_length > 0 )
		len++; //for command delimiter
	else //stack_length == 0, ptr can has any value; choose middle
		stack_ptr = stack_capacity / 2;

	if( !stack_maintain(stack_length + len + 1) )
		return FALSE;
	
	if( stack_ptr >= len ) { //new data could be simply copied
		stack_ptr -= len;
	} else {
		int rest_free = stack_capacity - stack_length - len;
		int desired_ptr = rest_free / 2 + len;
		int offset = desired_ptr - stack_ptr; //must be greater than zero
		memmove(pstack + stack_ptr + offset, pstack + stack_ptr, stack_length + 1);
		stack_ptr = desired_ptr - len;
	}
	strcpy(pstack + stack_ptr, arg);

	if( stack_length > 0 )
		pstack[stack_ptr+len-1] = cCommandDelimiter;
	else
		pstack[stack_ptr+len] = '\0';

	stack_length += len;

	return TRUE;
}
static BOOL stack_push_back(char *arg) {
	int len = strlen(arg);
	
	if( stack_length > 0 )
		len++; //for command delimiter
	else //stack_length == 0, ptr can has any value; choose middle
		stack_ptr = stack_capacity / 2;

	if( !stack_maintain(stack_length + len + 1) )
		return FALSE;
	
	if( stack_ptr + stack_length + len < stack_capacity ) { //new data could be simply copied
		
	} else {
		int rest_free = stack_capacity - stack_length - len;
		int desired_ptr = stack_capacity - len - stack_length - rest_free / 2 - 1;
		int offset = stack_ptr - desired_ptr; //must be greater than zero
		memmove(pstack + stack_ptr - offset, pstack + stack_ptr, stack_length + 1);
		stack_ptr = desired_ptr;
	}
	if( stack_length > 0 ) {
		pstack[stack_ptr + stack_length] = cCommandDelimiter;
		stack_length++;
		len--;
	}
	strcpy(pstack + stack_ptr + stack_length, arg);
	stack_length += len;

	return TRUE;
}
static void stack_pop_front(char *upto) {
	int len = upto - stack_top();
	if(*upto)
		len++; //command delimiter
	if( len >= stack_length ) {
		stack_length = 0;
		stack_ptr = stack_capacity / 2;
		pstack[stack_ptr] = '\0';
	} else {
		stack_ptr += len;
		stack_length -= len;
	}
}
/**************************************************************************/
/* parse input, check for TINTIN commands and aliases and send to session */
/**************************************************************************/
void parse_input(char *input, BOOL bExecuteNow)
{
    char command[BUFFER_SIZE], arg[BUFFER_SIZE];
	char result[BUFFER_SIZE];
    char *input2;

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
    
	int stack_length_initial = stack_length;

	if( !stack_push_front(input) )
		return;
 	if( !bExecuteNow && parsing_stack )
		return;
	parsing_stack = TRUE;
	
	//substitute_myvars(input, result);
    //input2=result;
	//while(*input2) {
	while( stack_length > stack_length_initial ) {
		input2 = stack_top();
//* en:colon
        // if(*input2==';')
        if( ((!bColon && *input2==cCommandDelimiter)||
			 ( bColon && *input2==';'              ))
			 && 
			 (input2 == result || *(input2-1)!='\\') )
            input2++;
//*/en
		
		char *current_token_start = input2;
        input2 = get_arg_stop_spaces(input2, command);
        input2 = get_arg_all(input2, arg);
		char *current_token_end = input2;

		substitute_myvars(command, result, sizeof(result));
		strcpy(command, result);
		substitute_myvars(arg, result, sizeof(result));
		strcpy(arg, result);

		stack_pop_front(input2);

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
				strcpy(strInputCommand, "\n[");
				strcat(strInputCommand, command);
				if (*arg != '\0') {
					strcat(strInputCommand, " ");
					strcat(strInputCommand, arg);
				}
				strcat(strInputCommand, "]");

				add_codes(strInputCommand, strOutputBuffer, "brown", TRUE);

				tintin_puts2(strOutputBuffer);

				if (bLogAsUserSeen) {
					if (hLogFile.is_open()) {
						log(strInputCommand);
						log("\n");
					}
					add_line_to_scrollbuffer(strInputCommand);
				}
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

		  ind = AliasList.begin();
		  char *test_str = current_token_start;
		  int test_len = current_token_end - current_token_start;
		  int offsets[33];
		  while (ind != AliasList.end() ) {
			  ALIAS *pal = ind->second;
			  int captured;
			  if (pal->m_pGroup->m_bEnabled && pal->m_PCRE.m_pPcre &&
				  (captured = pcre_exec(pal->m_PCRE.m_pPcre, pal->m_PCRE.m_pExtra, test_str, test_len, 0, 0, offsets, 33)) > 0) {
				  int i;
				  for ( i = 0 ; i < 10 ; i++ ) 
					  vars[i][0] = 0;
				  for ( i = 1 ; i < captured ; i++ ) {
					  if ( offsets[i*2] >= 0 ) {
						  int size = offsets[i*2+1]-offsets[i*2];
						  strncpy(vars[i-1], test_str+offsets[i*2], size);
						  *(vars[i-1]+size)='\0'; 
					  }
				  }
				  char newcommand[BUFFER_SIZE];
				  prepare_actionalias((char*)pal->m_strRight.data(), newcommand, sizeof(newcommand));
				  parse_input(newcommand);
				  break;
			  }
			  ind++;
		  }
		  if (ind != AliasList.end())
			  continue;
		
		  if( (ind=AliasList.find(command)) != AliasList.end() && ind->second->m_pGroup->m_bEnabled ) 
		  {
            
            int i;
            char *cpsource, *cpsource2, end;
			char newcommand[BUFFER_SIZE];

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
            prepare_actionalias((char*)pal->m_strRight.data(), newcommand, sizeof(newcommand)); 
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
	if( stack_length == 0 )
		parsing_stack = FALSE;
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
	else if(*s == 0x1B && *(s+1) == '[') { //don't even look into escape sequence!!!
		*arg++ = *s++;
		*arg++ = *s++;
		do {
			*arg++ = *s++;
		} while(*s && (isdigit(*s) || *s == ';'));
	}
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
	else if(*s == 0x1B) { //don't even look into escape sequence!!!
		*arg++ = *s++;
		if(*arg++ = *s++)
			do {
				*arg++ = *s++;
			} while(*s && (isdigit(*s) || *s == ';'));
	}
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
       } else if(*s==DEFAULT_CLOSE) {
         nest--;
       }
       *arg++ = *s++;
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
	else if(*s == 0x1B) { //don't even look into escape sequence!!!
		*arg++ = *s++;
		if(*arg++ = *s++)
			do {
				*arg++ = *s++;
			} while(*s && (isdigit(*s) || *s == ';'));
	}
	else if( ((!bColon && *s==cCommandDelimiter)||
		      ( bColon && *s==';'              )) 
			  &&
			  (*(s-1) != '\\')) {
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
  while(*s && isspace(*s))
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