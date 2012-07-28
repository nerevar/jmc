#include "stdafx.h"
#include "tintin.h"

extern struct listnode *search_node_with_wild();
extern struct listnode *searchnode_list();

int var_len[10];
char *var_ptr[10];

/***********************/
/* the #action command */
/***********************/

static BOOL bDroppedLine;

//* en
BOOL bContinuedAction = FALSE;
//* /en


BOOL show_actions(char* left, CGROUP* pGroup)
{
    BOOL bFound = FALSE;

    if ( !left || !*left ) 
        left = "*";
    
    ACTION_INDEX ind = ActionList.begin();
    while (ind  != ActionList.end() ) {
        ACTION* pac = *ind;
        if ( (!pGroup || (pGroup == pac->m_pGroup) ) && !pac->m_bDeleted &&  match(left, (char*)pac->m_strLeft.data() ) ){
            char temp[BUFFER_SIZE];
            sprintf(temp, rs::rs(1002), (char*)pac->m_strLeft.data(), (char*)(pac->m_strRight.data()), pac->m_nPriority , (char*)pac->m_pGroup->m_strName.data() );
            tintin_puts2(temp);
            bFound = TRUE;
        }
        ind++;
    }

    return bFound;
}


void action_command(char *arg)
{
    char left[BUFFER_SIZE], right[BUFFER_SIZE], result[BUFFER_SIZE];
    char pr[BUFFER_SIZE];
    int priority = 5;

    arg=get_arg_in_braces(arg, left,  STOP_SPACES);
    arg=get_arg_in_braces(arg, right, WITH_SPACES);
    arg=get_arg_in_braces(arg, pr,    STOP_SPACES);
    // check for priority/group name set
    if ( *pr ) {
        if ( isdigit(*pr) ) {
            priority = atoi(pr);
            arg=get_arg_in_braces(arg, pr, WITH_SPACES);
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
        if ( !pac1->m_bDeleted && !strcmp(left, pac1->m_strLeft.data()) ) {
            bNew = FALSE;
            break;
        }
        ind++;
    }
		
    if ( bNew ) {
        pac= new ACTION;
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
    pac->m_strRight = right;
    pac->m_nPriority = priority;
    pac->SetGroup(pr);

    if ( bNew ) 
        ActionList.push_back(pac);
    ActionList.sort(std::greater<ACTION*>());

    if (mesvar[MSG_ACTION]) {
      sprintf(result,rs::rs(1005),left,right,priority);
      tintin_puts2(result);
    }
}

/*************************/
/* the #unaction command */
/*************************/
void unaction_command(char* arg)
{
    char left[BUFFER_SIZE] ,result[BUFFER_SIZE];
    BOOL bFound = FALSE;

    arg=get_arg_in_braces(arg, left, WITH_SPACES);

    ACTION_INDEX ind = ActionList.begin();
    while (ind  != ActionList.end() ) {
        // CActionPtr  pac = *ind;
        ACTION* pac = *ind;
        if ( !pac->m_bDeleted && match(left, (char*)pac->m_strLeft.data() ) ){
            if (mesvar[MSG_ACTION]) {
                sprintf(result,rs::rs(1006),(char*)pac->m_strLeft.data () );
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
        sprintf(result, rs::rs(1007), left);
        tintin_puts2(result);
    }
}


/**************************************************************************/
/* run throught each of the commands on the right side of an alias/action */
/* expression, call substitute_text() for all commands but #alias/#action */
/**************************************************************************/
void prepare_actionalias(char *string, char *result)
{

  char arg[BUFFER_SIZE];
  *result='\0';
  substitute_vars(string,arg);
  substitute_myvars(arg, result);
}

/*************************************************************************/
/* copy the arg text into the result-space, but substitute the variables */
/* %0..%9 with the real variables                                        */
/*************************************************************************/
void substitute_vars(char *arg, char *result)
{
  int nest=0;
  int numands,n;
  char *ptr;
  while(*arg) {

    if(*arg=='%') { /* substitute variable */
      numands=0;
      while(*(arg+numands)=='%') 
        numands++;
      if (isdigit(*(arg+numands)) && numands==(nest+1)) {
         n=*(arg+numands)-'0';
         strcpy(result,vars[n]);
         arg=arg+numands+1;
         result+=strlen(vars[n]);
      }
      else {
        strncpy(result,arg,numands+1);
        arg+=numands+1;
        result+=numands+1;
      }
    }
    if(*arg=='$') { /* substitute variable */
      numands=0;
      while(*(arg+numands)=='$') 
        numands++;
      if (isdigit(*(arg+numands)) && numands==(nest+1)) {
         n=*(arg+numands)-'0';
         ptr=vars[n];
         while (*ptr) {
           // DELIMITER if (*ptr==';')
		   if (*ptr==cCommandDelimiter)
             ptr++;
           else
             *result++=*ptr++;
         }
         arg=arg+numands+1;
      }
      else {
        strncpy(result,arg,numands);
        arg+=numands;
        result+=numands;
      }
    }
    else if (*arg==DEFAULT_OPEN) {
      nest++;
      *result++=*arg++;
    }
    else if (*arg==DEFAULT_CLOSE) {
      nest--;
      *result++=*arg++;
    }
    else if (*arg=='\\' && nest==0) {
      while(*arg=='\\')
        *result++=*arg++;
      if(*arg=='%') {
        result--;
        *result++=*arg++;
	*result++=*arg++;
      }
    }
    else
      *result++=*arg++;
  }
  *result='\0';
}



/**********************************************/
/* check actions                              */
/**********************************************/
void check_all_actions(char *line1)
{
    // Have to remove all ESC characters before parsing 
    // But how ??
    static char temp[512]=PROMPT_FOR_PW_TEXT;
    char strng[BUFFER_SIZE];
    char line[4096];
    char* ptr = line1;
    char* res = line;
    while (*ptr)  {
        switch ( *ptr ) {
        case 0x1B:
            while ( *ptr && *ptr != 'm' ) 
                ptr++;
            if ( *ptr ) 
                ptr++;
            break;
        default:
            *res++ = *ptr++;
            break;

        }
    } 
    *res = 0;

    // Do check 

    if( check_one_action(line, temp)/*!strnicmp(line, temp, strlen(temp))*/ ){
        bPasswordEcho = FALSE;
    } else 
        bPasswordEcho = TRUE;

    bDroppedLine = FALSE;
  

    ACTION_INDEX ind = ActionList.begin();
    while (ind  != ActionList.end() ) {
        // CActionPtr pac = *ind;
        ACTION* pac = *ind;
        if(pac->m_pGroup->m_bEnabled && !pac->m_bDeleted && check_one_action(line, pac )) {
            char buffer[BUFFER_SIZE];
            prepare_actionalias((char*)pac->m_strRight.data(), buffer);
            if(echo ) { 
                sprintf(strng, rs::rs(1008), buffer);
                tintin_puts2(strng);
            }
            parse_input(buffer);

            if ( bDroppedLine ) {
                line1[0] = '.';
                line1[1] = 0;
                return;
            }
//* en
//            if ( !bMultiAction ) 
            if ( !bMultiAction && !bContinuedAction) 
//* /en
                return;
//* en
			bContinuedAction = FALSE;
//* /en
        }
        ind++;
     }
   
}

int match_a_string(char *line, char *mask)
{
  char *lptr, *mptr;
  lptr=line;
  mptr=mask;
  while (*lptr && *mptr && !(*mptr=='%' && isdigit(*(mptr+1)))) {
    if (*lptr++!=*mptr++)
      return -1;
  }
  if (!*mptr || (*mptr=='%' && isdigit(*(mptr+1)))) {
    return (int)(lptr-line);
  }
  return -1;
}

int check_one_action(char* line, char* action)
{
  int i; 
  if (check_a_action(line,action)) {
    for(i=0; i<10; i++) {
      if (var_len[i]!=-1) {
        strncpy(vars[i], var_ptr[i], var_len[i]);
        *(vars[i]+var_len[i])='\0'; 
      }
    }
    return TRUE;
  }       
  else
    return FALSE;
}


int check_one_action(char *line, ACTION* action)
{
    // first check for action need to be recompiled !
    if ( action->m_strRegex.length () == 0  ) {
        char left[BUFFER_SIZE];
        if ( action->m_bRecompile ) 
            substitute_myvars((char*)action->m_strLeft.data(),left);
        
        int i; 
        if (check_a_action(line,action->m_bRecompile ? left: (char*)action->m_strLeft.data())) {
            for(i=0; i<10; i++) {
                if (var_len[i]!=-1) {
                    strncpy(vars[i], var_ptr[i], var_len[i]);
                    *(vars[i]+var_len[i])='\0'; 
                }
            }
            return TRUE;
        } else
            return FALSE;
    }
    
    int offsets[33];
    int captured = pcre_exec(action->m_pPcre , action->m_pExtra , 
        line, strlen(line), 0, 0, offsets, 33);
    if ( captured > 0 ) { // copy strings to vars[] array
        for ( int i = 0 ; i < 10 ; i++ ) 
            vars[i][0] = 0;

        for ( i = 1 ; i < captured ; i++ ) {
            if ( offsets[i*2] >= 0 ) {
                int size = offsets[i*2+1]-offsets[i*2];
                strncpy(vars[i-1], line+offsets[i*2], size);
                *(vars[i-1]+size)='\0'; 
            }
        }
        return TRUE;   
    }
    return FALSE;
}
/******************************************************************/
/* check if a text triggers an action and fill into the variables */
/* return TRUE if triggered                                       */
/******************************************************************/
int check_a_action(char *line, char *action)
{   
  char result[BUFFER_SIZE];
  char *temp2, *tptr, *lptr, *lptr2;
  int  i,flag_anchor, len, flag;
  for (i=0; i<10; i++) var_len[i]=-1;
  flag_anchor=FALSE;
  lptr=line;
  substitute_myvars(action,result);
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
      var_len[*(tptr+1)-48]=strlen(lptr);
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
    


void DLLEXPORT RemoveAction(char* name) 
{
    ACTION_INDEX ind = ActionList.begin();
    while (ind  != ActionList.end() ) {
        ACTION* pac = *ind;
        if ( !pac->m_bDeleted && !strcmp(name, (char*)pac->m_strLeft.data()) ){
            delete pac;
            ActionList .erase (ind);
            return;
        }
        ind++;
    }
}

PACTION DLLEXPORT SetAction(char* name, char* text, int priority, char* group) 
{
    ACTION* pac = new ACTION;

    if ( !pac->SetLeft (name) ) {
        delete pac;
        return NULL;
    }
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

PACTION DLLEXPORT GetAction(char* name)
{
    ACTION_INDEX ind = ActionList.begin();
    while (ind  != ActionList.end() ) {
        ACTION* pac = *ind;
        if ( !pac->m_bDeleted && !strcmp(name, (char*)pac->m_strLeft.data()) ){
            return pac;
        }
        ind++;
    }
    return NULL;
}


void DLLEXPORT SetActionPattern(PACTION pAct, LPCSTR strText)
{
    pAct->SetLeft ((LPSTR)strText);
    ActionList.sort(std::greater<ACTION*>());
}

//* en
void drop_command(char *arg)
{
    bDroppedLine = TRUE;
}
void nodrop_command(char *arg)
{
    bDroppedLine = FALSE;
}

void next_command(char*arg)
{
    bContinuedAction = TRUE;
}

void gag_command(char*arg)
{
	char command[BUFFER_SIZE];
	if (*arg!='{') {
      strcpy(command,arg);
      strcpy(arg,"{");
      strcat(arg,command);
      strcat(arg,"} ");
    }
    strcat(arg, " .");
    parse_sub(arg);
}
//* /en
