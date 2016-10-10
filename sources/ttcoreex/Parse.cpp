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
bool parse_tintin_command(wchar_t *command, wchar_t *arg);
void do_speedwalk(wchar_t *cp) ;

void all_command();
void read_command();
void session_command();
void write_command();
void writesession_command();
void zap_command();
extern wchar_t *space_out();
void write_com_arg_mud(wchar_t* command, wchar_t* argument);
wchar_t *get_arg_all(wchar_t *s, wchar_t *arg, int maxlength);
wchar_t *get_arg_with_spaces(wchar_t * s, wchar_t* arg, int maxlength);
wchar_t *get_arg_stop_spaces(wchar_t *s, wchar_t *arg, int maxlength);
extern wchar_t mQueue[BUFFER_SIZE];
extern wchar_t cCommentChar;
extern void tstphandler();
void prefix_command(wchar_t *arg);

wchar_t s_prefix[BUFFER_SIZE];

extern BOOL bReadingConfig;

static wchar_t *pstack = NULL;
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
		pstack = (wchar_t*)malloc(to_allocate * sizeof(wchar_t));
		
		stack_length = 0;
		if( pstack ) {
			stack_ptr = BUFFER_SIZE;
			pstack[stack_ptr] = L'\0';
		}
	} else {
		wchar_t *new_ptr = (wchar_t*)realloc(pstack, to_allocate * sizeof(wchar_t));
		if( !new_ptr )
			free(pstack);
		pstack = new_ptr;
	}
	if( pstack ) {
		stack_capacity = to_allocate;
		return TRUE;
	} else {
		stack_capacity = stack_length = stack_ptr = 0;
		tintin_puts2(L"#Error: out of memory");
		return FALSE;
	}
}
static wchar_t* stack_top() {
	return pstack + stack_ptr;
}
static BOOL stack_push_front(wchar_t *arg) {
	int len = wcslen(arg);
	
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
		memmove(pstack + stack_ptr + offset, pstack + stack_ptr, (stack_length + 1)*sizeof(wchar_t));
		stack_ptr = desired_ptr - len;
	}
	wcscpy(pstack + stack_ptr, arg);

	if( stack_length > 0 )
		pstack[stack_ptr+len-1] = cCommandDelimiter;
	else
		pstack[stack_ptr+len] = L'\0';

	stack_length += len;

	return TRUE;
}
static BOOL stack_push_back(wchar_t *arg) {
	int len = wcslen(arg);
	
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
		memmove(pstack + stack_ptr - offset, pstack + stack_ptr, (stack_length + 1)*sizeof(wchar_t));
		stack_ptr = desired_ptr;
	}
	if( stack_length > 0 ) {
		pstack[stack_ptr + stack_length] = cCommandDelimiter;
		stack_length++;
		len--;
	}
	wcscpy(pstack + stack_ptr + stack_length, arg);
	stack_length += len;

	return TRUE;
}
static void stack_pop_front(wchar_t *upto) {
	int len = upto - stack_top();
	if(*upto)
		len++; //command delimiter
	if( len >= stack_length ) {
		stack_length = 0;
		stack_ptr = stack_capacity / 2;
		pstack[stack_ptr] = L'\0';
	} else {
		stack_ptr += len;
		stack_length -= len;
	}
}
/**************************************************************************/
/* parse input, check for TINTIN commands and aliases and send to session */
/**************************************************************************/
void parse_input(wchar_t *input, BOOL bExecuteNow)
{
    wchar_t command[BUFFER_SIZE], arg[BUFFER_SIZE];
	wchar_t result[BUFFER_SIZE];
    wchar_t *input2;

    if(*input==L'\0') {
        write_line_mud(L"");
        return ; 
    }

    if ( verbatim  && *input == cCommandChar && *(input+1) == L'v' ) {// check verbatim command
        wchar_t* input2=get_arg_stop_spaces(input+1, command, sizeof(command)/sizeof(wchar_t) - 1);
        if(command[0] == L'v' && is_abrev(command, L"verbatim")){
            get_arg_all(input2, arg, sizeof(arg)/sizeof(wchar_t) - 1);
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
        // if(*input2==L';')
        if( ((!bColon && *input2==cCommandDelimiter)||
			 ( bColon && *input2==L';'              ))
			 && 
			 (input2 == result || *(input2-1)!=L'\\') )
            input2++;
//*/en
		
		wchar_t *current_token_start = input2;
        input2 = get_arg_stop_spaces(input2, command, sizeof(command)/sizeof(wchar_t) - 1);
        input2 = get_arg_all(input2, arg, sizeof(arg)/sizeof(wchar_t) - 1);
		wchar_t *current_token_end = input2;

		substitute_myvars(command, result, sizeof(result)/sizeof(wchar_t));
		wcscpy(command, result);
		substitute_myvars(arg, result, sizeof(result)/sizeof(wchar_t));
		wcscpy(arg, result);

		stack_pop_front(input2);

//* en:prefix
		if(s_prefix[0] && command[0] != cCommandChar)
		{
			wchar_t p_command[BUFFER_SIZE];
			wcscpy(p_command,command);
			wcscat(p_command,L" ");
			wcscat(p_command,arg);
			wcscpy(command,s_prefix);
			wcscpy(arg,p_command);
		}
//*/en
        if(*command==cCommandChar) 
		{
			if (bDisplayCommands) {
				// output command in square brackets
				wchar_t strInputCommand[BUFFER_SIZE], strOutputBuffer[BUFFER_SIZE];
				wcscpy(strInputCommand, L"\n[");
				wcscat(strInputCommand, command);
				if (*arg != L'\0') {
					wcscat(strInputCommand, L" ");
					wcscat(strInputCommand, arg);
				}
				wcscat(strInputCommand, L"]");

				add_codes(strInputCommand, strOutputBuffer, L"brown", TRUE);

				tintin_puts2(strOutputBuffer);

				if (bLogAsUserSeen) {
					if (hLogFile.is_open()) {
						log(strInputCommand);
						log(L"\n");
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
		   wchar_t *ptr1,*ptr2;
     	   for(ptr1=mQueue;*ptr1&&*ptr1!=L'\0';ptr1++);
	       *ptr1++=cCommandDelimiter;
           for(ptr2=command;*ptr2&&*ptr2!=L'\0';ptr2++,ptr1++)
             *ptr1=*ptr2;
		   if(*arg)
			   *ptr1++=L' ';
           for(ptr2=arg;*ptr2&&*ptr2!=L'\0';ptr2++,ptr1++)
             *ptr1=*ptr2;
		   *ptr1++=L'\0';
		   continue;
		  }
//* en

          ALIAS_INDEX ind;
		  ind = AliasList.begin();
		  wchar_t *test_str = current_token_start;
		  int test_len = current_token_end - current_token_start;
		  int offsets[33];
		  {
			  while (ind != AliasList.end() ) {
				  ALIAS *pal = ind->second;
				  int captured;
				  if (pal->m_pGroup->m_bEnabled && pal->m_PCRE.m_pPcre &&
					  (captured = pcre16_exec(pal->m_PCRE.m_pPcre, pal->m_PCRE.m_pExtra, test_str, test_len, 0, 0, offsets, 33)) > 0) {
					  int i;
					  for ( i = 0 ; i < 10 ; i++ ) 
						  vars[i][0] = 0;
					  for ( i = 1 ; i < captured ; i++ ) {
						  if ( offsets[i*2] >= 0 ) {
							  int size = offsets[i*2+1]-offsets[i*2];
							  wcsncpy(vars[i-1], test_str+offsets[i*2], size);
							  *(vars[i-1]+size)=L'\0'; 
						  }
					  }
					  wchar_t newcommand[BUFFER_SIZE];
					  prepare_actionalias((wchar_t*)pal->m_strRight.data(), newcommand, sizeof(newcommand)/sizeof(wchar_t));
					  parse_input(newcommand);
					  break;
				  }
				  ind++;
			  }
		  }
		  if (ind != AliasList.end())
			  continue;

		  if( (ind=AliasList.find(command)) != AliasList.end() && ind->second->m_pGroup->m_bEnabled ) 
		  {
            
            int i;
            wchar_t *cpsource, *cpsource2, end;
			wchar_t newcommand[BUFFER_SIZE];

            wcscpy(vars[0], arg);

            for(i=1, cpsource=arg; i<10; i++) 
			{
                /* Next lines CHANGED to allow argument grouping with aliases */
                while (*cpsource == L' ')
                    cpsource++;
                end = (*cpsource == L'{') ? L'}' : L' ';
                cpsource = (*cpsource == L'{') ? cpsource+1 : cpsource;
                for(cpsource2=cpsource; *cpsource2 && *cpsource2!=end; cpsource2++);
                wcsncpy(vars[i], cpsource, cpsource2-cpsource);
                *(vars[i]+(cpsource2-cpsource))=L'\0';
                cpsource=(*cpsource2) ? cpsource2+1 : cpsource2;
            }
            ALIAS* pal = ind->second;
            prepare_actionalias((wchar_t*)pal->m_strRight.data(), newcommand, sizeof(newcommand)/sizeof(wchar_t)); 
            if(!wcscmp(pal->m_strRight.data(), newcommand) && *arg) 
			{
                wcscat(newcommand, L" "); 
                wcscat(newcommand, arg);
            }

            parse_input(newcommand);
		  }
          else 
		   if(speedwalk && !*arg && is_speedwalk_dirs(command))
            do_speedwalk(command);
           else 
		   {
                get_arg_with_spaces(arg, arg, sizeof(arg)/sizeof(wchar_t) - 1);
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
int is_speedwalk_dirs(wchar_t *cp)
{
  wchar_t command[2];
  int flag;
  flag=FALSE;

  while(*cp) {
      command[0] = *cp;
      command[1] = 0;
    if(*cp!=L'n' && *cp!=L'e' && *cp!=L's' && *cp!=L'w' && *cp!=L'u' && *cp!=L'd' &&
    !iswdigit(*cp) && !searchnode_list(common_pathdirs, cp) )
      return FALSE;
    if(!iswdigit(*cp))
      flag=TRUE;
    cp++;
  }
  return flag;
}

/**************************/
/* do the speedwalk thing */
/**************************/
void do_speedwalk(wchar_t *cp)
{
  wchar_t sc[2];
  wchar_t *loc; 
  int multflag,loopcnt,i;
  wcscpy(sc, L"x");
  while(*cp) {
    loc=cp;
    multflag=FALSE;
    while(iswdigit(*cp)) {
      cp++;
      multflag=TRUE;
    }
    if(multflag && *cp) {
      swscanf(loc,L"%d%lc",&loopcnt,sc);
      i=0;
      while(i++<loopcnt)
    	write_com_arg_mud(sc, L"");
    }
    else if (*cp) {
      sc[0]=*cp;
      write_com_arg_mud(sc, L"");
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
bool parse_tintin_command(wchar_t *command, wchar_t *arg)
{
  /*
   this lines almost totally rewrote to put all command functions
   into a struct array. Size of array defined in tintin.h;
   Array itself defined in cmds.h.
   Struct:
    wchar_t alias    - with it you can easily add aliases with same function name
	void function_name - main stuff
	wchar_t hlpfile  - this field is for help_command. do not forget to link 
	                aliases to one help file.
	would rely it would not crash... En.
  */
  CharLower(command);

  if(iswdigit(*command)) {
	  int a1=0;
	  int a2=0;
  	  swscanf(command,L"%d:%d",&a1,&a2);
	  get_arg_in_braces(arg,arg,WITH_SPACES,BUFFER_SIZE-1);

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
wchar_t *get_arg_all(wchar_t *s, wchar_t *arg, int maxlength)
{
  /* int inside=FALSE; */
  int nest=0;
  s=space_out(s);
  while(*s && maxlength > 0) {

    if(*s==L'\\' && maxlength > 1) {
      *arg++=*s++;
	  maxlength--;
      if(*s) {
		*arg++=*s++;
		maxlength--;
	  }
    }

    // else if(*s==L';' && nest<1) {
	else if(*s == ANSI_COMMAND_CHAR && *(s+1) == L'[' && maxlength > 2) { //don't even look into escape sequence!!!
		*arg++ = *s++;
		maxlength--;
		*arg++ = *s++;
		maxlength--;
		do {
			*arg++ = *s++;
			maxlength--;
		} while(*s && (iswdigit(*s) || *s == L';') && maxlength > 0);
	}
	else if((*s==cCommandDelimiter||(bColon && *s==L';'&&*(s-1)!=L'\\')) && nest<1) {
	break;
    }

    else if(*s==DEFAULT_OPEN) {
      nest++;
      *arg++=*s++;
	  maxlength--;
    }

    else if(*s==DEFAULT_CLOSE) {
      nest--;
      *arg++=*s++;
	  maxlength--;
    }

    else {
      *arg++=*s++;
	  maxlength--;
	}
  }

  *arg=L'\0';
  return s;
}

/**************************************/
/* get all arguments - remove "s etc. */
/* Example:                           */
/* In: "this is it" way way hmmm;     */
/* Out: this is it way way hmmm       */ 
/**************************************/
wchar_t *get_arg_with_spaces(wchar_t * s, wchar_t* arg, int maxlength)
{
  int nest=0;
  /* int inside=FALSE; */

  s=space_out(s);
  while(*s && maxlength > 0) {

    if(*s==L'\\') {
		if(*++s) {
			*arg++=*s++;
			maxlength--;
		}
    }

    // else if(*s==L';' && nest==0) {
	else if(*s == ANSI_COMMAND_CHAR && *(s+1) == L'[' && maxlength > 2 ) { //don't even look into escape sequence!!!
		*arg++ = *s++;
		maxlength--;
		*arg++ = *s++;
		maxlength--;
		do {
			*arg++ = *s++;
			maxlength--;
		} while(*s && (iswdigit(*s) || *s == L';') && maxlength > 0);
	}
	else if((*s==cCommandDelimiter||(bColon && *s==L';'&&*(s-1)!=L'\\')) && nest==0) {
	break;
    }
    else if(*s==DEFAULT_OPEN) {
      nest++;
      *arg++=*s++;
	  maxlength--;
    }
    else if(*s==DEFAULT_CLOSE) {
      *arg++=*s++;
	  maxlength--;
      nest--;
    }
    else {
      *arg++=*s++;
	  maxlength--;
	}
  }
*arg=L'\0'; 
return s;
}
/********************/
/* my own routine   */
/********************/
wchar_t *get_arg_in_braces(wchar_t *s, wchar_t *arg, int flag, int maxlength)
{
   int nest=0;
   wchar_t *ptr;
   s=space_out(s);
   ptr=s;
   if (*s!=DEFAULT_OPEN) {
     if (flag==0) 
       s=get_arg_stop_spaces(ptr,arg,maxlength);
     else
       s=get_arg_with_spaces(ptr,arg,maxlength);
     return s;
   }
   s++;
     while(*s!=L'\0' && !(*s==DEFAULT_CLOSE && nest==0) && maxlength > 0) {
       if(*s==DEFAULT_OPEN) {
         nest++;
       } else if(*s==DEFAULT_CLOSE) {
         nest--;
       }
       *arg++ = *s++;
	   maxlength--;
     }
   if (!*s)
     tintin_puts2(rs::rs(1146));
   else
     s++;
   *arg=L'\0';
   return s;
   
}
/**********************************************/
/* get one arg, stop at spaces                */
/* remove quotes                              */
/**********************************************/
wchar_t *get_arg_stop_spaces(wchar_t *s, wchar_t *arg, int maxlength)
{
  int inside=FALSE;
  s=space_out(s);
  
  while(*s && maxlength > 0) {
    if(*s==L'\\') {
		if(*++s) {
			*arg++=*s++;
			maxlength--;
		}
    }
    else if(*s==L'"') {
      s++;
      inside=!inside;
    }

    // else if(*s==L';') {
	else if(*s == ANSI_COMMAND_CHAR && *(s+1)==L'[' && maxlength > 2) { //don't even look into escape sequence!!!
		*arg++ = *s++;
		maxlength--;
		*arg++ = *s++;
		maxlength--;
		do {
			*arg++ = *s++;
			maxlength--;
		} while(*s && (iswdigit(*s) || *s == L';') && maxlength > 0);
	}
	else if( ((!bColon && *s==cCommandDelimiter)||
		      ( bColon && *s==L';'              )) 
			  &&
			  (*(s-1) != L'\\')) {
		if(inside) {
			*arg++=*s++;
			maxlength--;
		}
		else
		break;
    }

    else if(!inside && *s==L' ')
      break;
    else {
      *arg++=*s++;
	  maxlength--;
	}
  }

  *arg=L'\0';
  return s;
}

/*********************************************/
/* spaceout - advance ptr to next none-space */
/* return: ptr to the first none-space       */
/*********************************************/ 
wchar_t *space_out(wchar_t *s)
{
  while(*s && iswspace(*s))
    s++;
  return s;
}

/************************************/
/* send command+argument to the mud */
/************************************/
void write_com_arg_mud(wchar_t* command, wchar_t* argument)
{
    wchar_t outtext[BUFFER_SIZE*2 + 2];

    check_insert_path(command);
	
	wcscpy(outtext, command);
	
    if(*argument) {
      wcscat(outtext, L" "); 
      wcscat(outtext, argument);
    }
	outtext[BUFFER_SIZE-1] = L'\0';
    write_line_mud(outtext);
}


//* en
void prefix_command(wchar_t *arg)
{
 space_out(arg);
 wcscpy(s_prefix,arg);
 wchar_t sn[BUFFER_SIZE];
 swprintf(sn,rs::rs(1255),arg);
 tintin_puts2(sn);
}
//*/en