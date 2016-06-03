#include "stdafx.h"
#include "tintin.h"

extern struct listnode *search_node_with_wild();
extern struct listnode *searchnode_list();

int var_len[10];
const wchar_t *var_ptr[10];


/***********************/
/* the #action command */
/***********************/

static BOOL bDroppedLine;
static BOOL bReplacedLine;
static wchar_t strReplacedLine[BUFFER_SIZE];

//* en
BOOL bContinuedAction = FALSE;
//* /en

LPWSTR DLLEXPORT act_type_to_str(int type)
{
	switch ((ACTION::ActionType)type) {
	default:
	case ACTION::Action_TEXT:
		return L"TEXT";
		break;
	case ACTION::Action_RAW:
		return L"RAW";
		break;
	case ACTION::Action_COLOR:
		return L"COLOR";
		break;
	}
}

BOOL show_actions(wchar_t* left, CGROUP* pGroup)
{
	wchar_t temp[BUFFER_SIZE];
    BOOL bFound = FALSE;

    if ( !left || !*left ) 
        left = L"*";
    
    ACTION_INDEX ind = ActionList.begin();
    while (ind  != ActionList.end() ) {
        ACTION* pac = *ind;
        if ( (!pGroup || (pGroup == pac->m_pGroup) ) && !pac->m_bDeleted &&  match(left, pac->m_strLeft.c_str() ) ) {
            swprintf(temp, rs::rs(1276), 
				act_type_to_str((int)pac->m_InputType),
				pac->m_strLeft.c_str(), 
				pac->m_strRight.c_str(), 
				pac->m_nPriority , 
				pac->m_pGroup->m_strName.c_str() );
            tintin_puts2(temp);
            bFound = TRUE;
        }
        ind++;
    }

    return bFound;
}


void action_command(wchar_t *arg)
{
    wchar_t left[BUFFER_SIZE], right[BUFFER_SIZE], result[BUFFER_SIZE];
    wchar_t pr[BUFFER_SIZE];
    int priority = 5;

	enum ACTION::ActionType type = ACTION::Action_TEXT;

    arg=get_arg_in_braces(arg, left,  STOP_SPACES, sizeof(left)/sizeof(wchar_t) - 1);
	
	if (is_abrev(left, L"text")) {
		type = ACTION::Action_TEXT;
		arg=get_arg_in_braces(arg, left,  STOP_SPACES, sizeof(left)/sizeof(wchar_t) - 1);
	} else if (is_abrev(left, L"raw")) {
		type = ACTION::Action_RAW;
		arg=get_arg_in_braces(arg, left,  STOP_SPACES, sizeof(left)/sizeof(wchar_t) - 1);
	} else if (is_abrev(left, L"color")) {
		type = ACTION::Action_COLOR;
		arg=get_arg_in_braces(arg, left,  STOP_SPACES, sizeof(left)/sizeof(wchar_t) - 1);
	}

    arg=get_arg_in_braces(arg, right, WITH_SPACES, sizeof(right)/sizeof(wchar_t) - 1);
    arg=get_arg_in_braces(arg, pr,    STOP_SPACES, sizeof(pr)/sizeof(wchar_t) - 1);

    // check for priority/group name set
    if ( *pr ) {
        if ( iswdigit(*pr) ) {
            priority = _wtoi(pr);
            arg=get_arg_in_braces(arg, pr, WITH_SPACES, sizeof(pr)/sizeof(wchar_t) - 1);
        }
    }

    if(!*left) {
        tintin_puts2(rs::rs(1003));
    }
    
    if(!*right) {
         if ( !show_actions (left) ) 
             tintin_puts2(rs::rs(1004));
         return;
    }

    // CActionPtr pac;
    ACTION* pac;
    ACTION_INDEX ind = ActionList.begin();
    BOOL bNew = TRUE;
    while (ind  != ActionList.end() ) {
        // CActionPtr pac1 = *ind;
        ACTION* pac1 = *ind;
        if ( !pac1->m_bDeleted && !wcscmp(left, pac1->m_strLeft.c_str()) && !wcscmp(right, pac1->m_strRight.c_str())) {
            bNew = FALSE;
            break;
        }
        ind++;
    }
		
    if ( bNew ) {
        pac= new ACTION();
    } else {
        pac = *ind;
    }

    if ( !pac->SetLeft (left) ) {
        if ( bNew ) {
            // pac.Clear();
            delete pac;
        }
        return;
    }
	pac->m_InputType = type;
    pac->m_strRight = right;
    pac->m_nPriority = priority;
    pac->SetGroup(pr);

    if ( bNew ) 
        ActionList.push_back(pac);
    ActionList.sort(std::greater<ACTION*>());

    if (mesvar[MSG_ACTION]) {
		swprintf(result,rs::rs(1277),
			act_type_to_str((int)type),
			left,
			right,
			priority);
      tintin_puts2(result);
    }
}

/*************************/
/* the #unaction command */
/*************************/
void unaction_command(wchar_t* arg)
{
    wchar_t left[BUFFER_SIZE] ,result[BUFFER_SIZE];
    BOOL bFound = FALSE;

    arg=get_arg_in_braces(arg, left, WITH_SPACES, sizeof(left)/sizeof(wchar_t) - 1);

    ACTION_INDEX ind = ActionList.begin();
    while (ind  != ActionList.end() ) {
        // CActionPtr  pac = *ind;
        ACTION* pac = *ind;
        if ( !pac->m_bDeleted && match(left, pac->m_strLeft.c_str() ) ){
            if (mesvar[MSG_ACTION]) {
                swprintf(result,rs::rs(1006),pac->m_strLeft.c_str() );
                tintin_puts2(result);
                bFound = TRUE;
            }
            pac->m_bDeleted = TRUE;
            bDelayedActionDelete = TRUE;
/*            ind++;
            ActionList.remove(pac);
            delete pac;
*/       } 
          ind++;
    }
    if (!bFound && mesvar[MSG_ACTION]) {
        swprintf(result, rs::rs(1007), left);
        tintin_puts2(result);
    }
}


/**************************************************************************/
/* run throught each of the commands on the right side of an alias/action */
/* expression, call substitute_text() for all commands but #alias/#action */
/**************************************************************************/
void prepare_actionalias(const wchar_t *string, wchar_t *result, int maxlength)
{
  wchar_t arg[BUFFER_SIZE];
  *result='\0';
  substitute_vars(string,arg,sizeof(arg)/sizeof(wchar_t));
  substitute_myvars(arg, result, maxlength);
}

/*************************************************************************/
/* copy the arg text into the result-space, but substitute the variables */
/* %0..%9 with the real variables                                        */
/*************************************************************************/
void substitute_vars(const wchar_t *arg, wchar_t *result, int maxlength)
{
  int nest=0;
  int numands,n;
  wchar_t *ptr;
  maxlength--; //reserve for null-terminator
  while(*arg && maxlength > 0) {

    if(*arg=='%') { /* substitute variable */
      numands=0;
      while(*(arg+numands)=='%') 
        numands++;
      if (iswdigit(*(arg+numands)) && numands==(nest+1)) {
         n=*(arg+numands)-'0';

		 int len = wcslen(vars[n]);
		 if( len > maxlength )
			len = maxlength;
         wcsncpy(result,vars[n],len);
         arg += numands+1;
         result += len;
		 maxlength -= len;
      }
      else {
		int len = numands + 1;
		if( len > maxlength )
			len = maxlength;
        wcsncpy(result,arg,len);
        arg += numands+1;
        result += len;
		maxlength -= len;
      }
    }
    if(*arg=='$') { /* substitute variable */
      numands=0;
      while(*(arg+numands)=='$') 
        numands++;
      if (iswdigit(*(arg+numands)) && numands==(nest+1)) {
         n=*(arg+numands)-'0';
         ptr=vars[n];
         while (*ptr) {
           // DELIMITER if (*ptr==';')
		   if (*ptr==cCommandDelimiter)
             ptr++;
           else if (maxlength > 0) {
             *result++=*ptr++;
			 --maxlength;
		   } else
			   ptr++;
         }
         arg += numands+1;
      }
      else {
		int len = numands;
		if( len > maxlength )
			len = maxlength;
        wcsncpy(result,arg,len);
        arg += numands;
        result += len;
		maxlength -= len;
      }
    }
    else if (*arg==DEFAULT_OPEN) {
      nest++;
	  if (maxlength > 0) {
		*result++=*arg++;
		--maxlength;
	  } else
		  arg++;
    }
    else if (*arg==DEFAULT_CLOSE) {
      nest--;
	  if (maxlength > 0) {
		*result++=*arg++;
		--maxlength;
	  } else
		  arg++;
    }
    else if (*arg=='\\' && nest==0) {
		while(*arg=='\\') {
			if (maxlength > 0) {
				*result++=*arg++;
				--maxlength;
			} else
				arg++;
		}
      if(*arg=='%') {
		if (maxlength > 0) {
			result--;
			*result++=*arg++;
			*result++=*arg++;
			--maxlength;
		} else {
			arg++;
			arg++;
		}
      }
    }
    else if (maxlength > 0) {
      *result++=*arg++;
	  --maxlength;
	} else
	  arg++;
  }
  *result++='\0';
}

static const wchar_t color_codes[8] = {L'd', L'r', L'g', L'y', L'b', L'p', L'c', L'w'};
static std::map <wchar_t, std::wstring> color_commands;

void remove_ansi_codes(const wchar_t *input, wchar_t *output)
{
    while (*input)  {
        switch ( *input ) {
        case ANSI_COMMAND_CHAR:
            while ( *input && *input != L'm' ) 
                input++;
            if ( *input ) 
                input++;
            break;
        default:
            *output++ = *input++;
            break;

        }
    } 
    *output = 0;
}

void convert_ansi_to_colored(const wchar_t *input, wchar_t *output, int maxlength, int &initial_state)
{
	int state = initial_state; //37;
	int last_printed = -1;
	while ((*input) && maxlength) {
		if (input[0] == ANSI_COMMAND_CHAR && input[1] == L'[') {
			input += 2;
			int cmd = 0, cmdlen = 0, next_state = state;
			while (*input) {
				if (iswdigit(*input)) {
					cmd = cmd * 10 + (*input - L'0');
					cmdlen++;
				} else if (cmdlen > 0) {
					//parse cmd
					if (cmd == 0) {
						next_state = 37;
					} else if (cmd == 1) {
						next_state |= 0x100;
					} else if (cmd >= 30 && cmd <= 37) {
						next_state = (next_state & 0x100) | cmd;
					}
					cmd = cmdlen = 0;
				}
				if (iswalpha(*input)) {
					if (*input == L'm') //color command
						state = next_state;
					else
						next_state = state;
					input++;
					break;
				}
				input++;
			}
			continue;
		}
		if ((*input) < 0 || iswprint(*input) || (*input) == L'\n') {
			if (!iswspace(*input) && last_printed != state && maxlength > 2) {
				(*output++) = L'&';
				
				int index = (state & 0xFF) - 30;
				if (index < 0 || index >= sizeof(color_codes)/sizeof(wchar_t))
					index = sizeof(color_codes)/sizeof(wchar_t) - 1;

				if(state >> 8)
					(*output++) = toupper(color_codes[index]);
				else
					(*output++) = color_codes[index];
				
				maxlength -= 2;
				last_printed = state;
			}
			if (maxlength > 1 && *input == L'&') {
				(*output++) = *input;
				maxlength--;
			}
			(*output++) = *input;
			maxlength--;
		}
		input++;
	}
	*output = '\0';
	initial_state = state;
}

void convert_colored_to_ansi(const wchar_t *input, wchar_t *output, int maxlength)
{
	int i;
	if (color_commands.size() == 0) {
		wchar_t buf[64];
		for (i = 0; i < sizeof(color_codes)/sizeof(wchar_t); i++) {
			swprintf(buf, L"%c[0;3%dm", ANSI_COMMAND_CHAR, i);
			color_commands[color_codes[i]] = buf;
			swprintf(buf, L"%c[1;3%dm", ANSI_COMMAND_CHAR, i);
			color_commands[toupper(color_codes[i])] = buf;
		}
		color_commands[L'&'] = L"&";
	}

	std::map <wchar_t, std::wstring>::iterator it;
	while (*input && maxlength) {
		if (*input != L'&' || (it = color_commands.find(*(input + 1))) == color_commands.end()) {
			*output++ = *input++;
			maxlength--;
		} else {
			input += 2;
			int len = it->second.length();
			if (len > maxlength)
				len = maxlength;
			for (i = 0; i < len; i++)
				*output++ = it->second[i];
			maxlength -= len;
		}
	}
	*output = L'\0';
}

/**********************************************/
/* check actions                              */
/**********************************************/
void check_all_actions(wchar_t *line, bool multiline)
{
    wchar_t strng[BUFFER_SIZE];

	static wchar_t uncolored[MULTILINE_BUFFER_SIZE];
	static wchar_t colored[MULTILINE_BUFFER_SIZE];

	static int singleline_state = 37;
	static int multiline_state = 37;
	
	uncolored[0] = L'\0';
	colored[0] = L'\0';

    // Do check 

    bDroppedLine = bReplacedLine = FALSE;
	strReplacedLine[0] = L'\0';
  

    ACTION_INDEX ind = ActionList.begin();
    while (ind  != ActionList.end() ) {
        // CActionPtr pac = *ind;
        ACTION* pac = *ind;
        if(pac->m_pGroup->m_bEnabled && !pac->m_bDeleted && pac->m_PCRE.m_bMultiline == (multiline ? TRUE : FALSE)) {
			wchar_t *input;

			switch (pac->m_InputType) {
			default:
			case ACTION::Action_RAW:
				input = line;
				break;
			case ACTION::Action_TEXT:
				if (!uncolored[0])
					remove_ansi_codes(line, uncolored);
				input = uncolored;
				break;
			case ACTION::Action_COLOR:
				if (!colored[0])
					convert_ansi_to_colored(line, colored,  sizeof(colored)/sizeof(wchar_t) - 1,
					                        multiline ? multiline_state : singleline_state);
				input = colored;
				break;
			}

			int offset = 0;
			while (check_one_action(input, pac, &offset)) {
				wchar_t buffer[BUFFER_SIZE];
				prepare_actionalias(pac->m_strRight.c_str(), buffer, sizeof(buffer)/sizeof(wchar_t));
				if(echo ) { 
					swprintf(strng, rs::rs(1008), buffer);
					tintin_puts2(strng);
				}
				parse_input(buffer, TRUE);
				if ( bDroppedLine ) {
					wcscpy(line, L".");
					return;
				}
				if ( bReplacedLine ) {
					wcscpy(line, strReplacedLine);
					bReplacedLine = FALSE;
				}
				if ( !bMultiAction && !bContinuedAction) 
					return;
				bContinuedAction = FALSE;
				if (!pac->m_bGlobal)
					break;
			}
		}
        ind++;
     }
   
}

int match_a_string(const wchar_t *line, const wchar_t *mask)
{
  const wchar_t *lptr, *mptr;
  lptr=line;
  mptr=mask;
  while (*lptr && *mptr && !(*mptr==L'%' && iswdigit(*(mptr+1)))) {
    if (*lptr++!=*mptr++)
      return -1;
  }
  if (!*mptr || (*mptr==L'%' && iswdigit(*(mptr+1)))) {
    return (int)(lptr-line);
  }
  return -1;
}

int check_one_action(const wchar_t* line, const wchar_t* action)
{
  int i; 
  if (check_a_action(line,action)) {
    for(i=0; i<10; i++) {
      if (var_len[i]!=-1) {
        wcsncpy(vars[i], var_ptr[i], var_len[i]);
        *(vars[i]+var_len[i])='\0'; 
      }
    }
    return TRUE;
  }       
  else
    return FALSE;
}


int check_one_action(const wchar_t* line, ACTION* action, int *offset)
{
    // first check for action need to be recompiled !
    if ( !action->m_PCRE.m_pPcre ) {
        wchar_t left[BUFFER_SIZE];
        if ( action->m_bRecompile ) 
            substitute_myvars(action->m_strLeft.c_str(),left, sizeof(left)/sizeof(wchar_t));
        
        int i; 
        if (check_a_action(line,action->m_bRecompile ? left: action->m_strLeft.c_str())) {
            for(i=0; i<10; i++) {
                if (var_len[i]!=-1) {
                    wcsncpy(vars[i], var_ptr[i], var_len[i]);
                    *(vars[i]+var_len[i])='\0'; 
                }
            }
            return TRUE;
        } else
            return FALSE;
    }
    
	int start_offset = offset ? (*offset) : 0;
    int offsets[33];

	{
		//char *test_ansi = (char*)(line);
		//int test_ansi_len = strlen(test_ansi);
		//int captured = pcre_exec(action->m_PCRE.m_pPcre , action->m_PCRE.m_pExtra , 
		//	test_ansi, test_ansi_len, start_offset, 0, offsets, 33);
		wchar_t *test = (wchar_t*)(line);
		int test_len = wcslen(test);
		int captured = pcre16_exec(action->m_PCRE.m_pPcre , action->m_PCRE.m_pExtra , 
			test, test_len, start_offset, 0, offsets, 33);
		if ( captured > 0 ) { // copy strings to vars[] array
			int i;
			for ( i = 0 ; i < 10 ; i++ ) 
				vars[i][0] = 0;

			for ( i = 1 ; i < captured ; i++ ) {
				if ( offsets[i*2] >= 0 ) {
					int size = offsets[i*2+1]-offsets[i*2];
					wcsncpy(vars[i-1], line+offsets[i*2], size);
					*(vars[i-1]+size)='\0'; 
				}
			}
			if (offset)
				*offset = offsets[1];
			return TRUE;   
		}
	}
    return FALSE;
}
/******************************************************************/
/* check if a text triggers an action and fill into the variables */
/* return TRUE if triggered                                       */
/******************************************************************/
int check_a_action(const wchar_t *line, const wchar_t *action)
{   
  wchar_t result[BUFFER_SIZE];
  const wchar_t *temp2, *tptr, *lptr, *lptr2;
  int  i,flag_anchor, len, flag;
  for (i=0; i<10; i++) var_len[i]=-1;
  flag_anchor=FALSE;
  lptr=line;
  substitute_myvars(action,result, sizeof(result)/sizeof(wchar_t));
  tptr=result;
  if(*tptr=='^') {
    tptr++;
    flag_anchor=TRUE;
    /* CHANGED to fix a bug with #action {^%0 foo}
     * Thanks to Spencer Sun for the bug report (AND fix!)
    if (*tptr!=*line)
      return FALSE;
    */
  }
  if (flag_anchor) {
    if ((len=match_a_string(lptr, tptr))==-1)
      return FALSE;
    lptr+=len;
    tptr+=len;
  }
  else {
    flag=TRUE;
    len=-1;
    while(*lptr && flag) {
      if((len=match_a_string(lptr, tptr))!=-1) {
        flag=FALSE;
      }
      else
        lptr++;
    }
    if (len!=-1) {
      lptr+=len;
      tptr+=len;
    }
    else
      return FALSE;
  }
  while(*lptr && *tptr) {
    temp2=tptr+2;
    if (!*temp2) {
      var_len[*(tptr+1)-48]=wcslen(lptr);
      var_ptr[*(tptr+1)-48]=lptr;
      return TRUE;
    }
    lptr2=lptr;
    flag=TRUE;
    len=-1;
    while(*lptr2 && flag) {
      if ((len=match_a_string(lptr2, temp2))!=-1) {
        flag=FALSE;
      }
      else 
        lptr2++;
    }
    if (len!=-1) {
      var_len[*(tptr+1)-48]=lptr2-lptr;
      var_ptr[*(tptr+1)-48]=lptr;
      lptr=lptr2+len;
      tptr=temp2+len;
    }
    else {
      return FALSE;
    }
  }
  if (*tptr)
    return FALSE;
  else
    return TRUE;
}       
    


BOOL DLLEXPORT RemoveAction(ACTION* pac)
{
    ACTION_INDEX ind = ActionList.begin();
    while (ind  != ActionList.end() ) {
        ACTION* pactodel = *ind;
        if (pactodel == pac){
			delete pactodel;
            ActionList .erase (ind);
            return true;
		}
        ind++;
    }
	return false;
}

void DLLEXPORT SetActionText(ACTION* pac, const wchar_t* text)
{

    pac->m_strRight = text;

}

PACTION DLLEXPORT SetAction(ACTION::ActionType type, const wchar_t* name, const wchar_t* text, int priority, const wchar_t* group) 
{
    ACTION* pac = new ACTION();

    if ( !pac->SetLeft (name) ) {
        delete pac;
        return NULL;
    }
	pac->m_InputType = type;
    pac->m_strRight = text;
    pac->m_nPriority = priority;
    pac->SetGroup(group);

    ActionList.push_back(pac);
    ActionList.sort(std::greater<ACTION*>());

    return pac;    
}

PPACTION DLLEXPORT GetActionsList(int* size)
{
    *size = ActionList.size();

    ACTION_INDEX ind = ActionList.begin();
    int i = 0;
    while (ind != ActionList.end() ) {
        JMCObjRet[i++] = *ind;
        ind++;
    }
    return (PPACTION)JMCObjRet;
}

PACTION DLLEXPORT GetAction(wchar_t* name)
{
    ACTION_INDEX ind = ActionList.begin();
    while (ind  != ActionList.end() ) {
        ACTION* pac = *ind;
        if ( !pac->m_bDeleted && !wcscmp(name, pac->m_strLeft.c_str()) ){
            return pac;
        }
        ind++;
    }
    return NULL;
}


void DLLEXPORT SetActionPattern(PACTION pAct, const wchar_t* strText)
{
    pAct->SetLeft (strText);
    ActionList.sort(std::greater<ACTION*>());
}

//* en
void drop_command(wchar_t *arg)
{
    bDroppedLine = TRUE;
}
void nodrop_command(wchar_t *arg)
{
    bDroppedLine = FALSE;
}
void replace_command(wchar_t *arg)
{
    wchar_t left[BUFFER_SIZE];
    
    arg = get_arg_in_braces(arg, left, WITH_SPACES, sizeof(left)/sizeof(wchar_t) - 1);

	prepare_actionalias(left, strReplacedLine, sizeof(strReplacedLine)/sizeof(wchar_t));   

	bReplacedLine = TRUE;
}

void next_command(wchar_t*arg)
{
    bContinuedAction = TRUE;
}

void gag_command(wchar_t*arg)
{
	wchar_t command[BUFFER_SIZE];
	if (*arg!=L'{') {
      wcscpy(command,arg);
      wcscpy(arg,L"{");
      wcscat(arg,command);
      wcscat(arg,L"} ");
    }
    wcscat(arg, L" .");
    parse_sub(arg);
}
//* /en
