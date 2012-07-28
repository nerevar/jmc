/* Autoconf patching by David Hedbor, neotron@lysator.liu.se */
/*********************************************************************/
/* file: substitute.c - functions related to the substitute command  */
/*                             TINTIN III                            */
/*          (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t             */
/*                     coded by peter unold 1992                     */
/*********************************************************************/
#include "stdafx.h"
#include "tintin.h"

extern char *get_arg_in_braces();
extern struct listnode *search_node_with_wild();
extern struct listnode *searchnode_list();

//* en:multisub
BOOL bMultiSub = FALSE;
//* en


/***************************/
/* the #substitute command */
/***************************/
void parse_sub(char *arg)
{
  char left[BUFFER_SIZE], right[BUFFER_SIZE], result[BUFFER_SIZE];
  struct listnode *mysubs, *ln;
  mysubs=common_subs;
  arg=get_arg_in_braces(arg, left,STOP_SPACES);
  arg=get_arg_in_braces(arg, right,WITH_SPACES);

  if(!*left) {
    tintin_puts2(rs::rs(1166));
    show_list(mysubs);
  }

  else if(*left && !*right) {
    if ((ln=search_node_with_wild(mysubs,left))!=NULL) {
      while((mysubs=search_node_with_wild(mysubs, left))!=NULL) {
        shownode_list(mysubs);
      }
    }
    else
      if (mesvar[MSG_SUB])
        tintin_puts2(rs::rs(1167));
  }

  else {
    if((ln=searchnode_list(mysubs, left))!=NULL) {
      deletenode_list(mysubs, ln);
      subnum--;
    }
    insertnode_list(mysubs, left, right, "0", ALPHA);
    subnum++;
    if (strcmp(right,".")!=0)
      sprintf(result, rs::rs(1168),right,left);
    else
      sprintf(result, rs::rs(1169), left);
    if (mesvar[MSG_SUB])
      tintin_puts2(result);
  }
}


/*****************************/
/* the #unsubstitute command */
/*****************************/

void unsubstitute_command(char *arg)
{
  char left[BUFFER_SIZE] ,result[BUFFER_SIZE];
  struct listnode *mysubs, *ln;
  struct listnode TempPointerHolder;
  int flag;
  flag=FALSE;
  mysubs=common_subs;
  arg=get_arg_in_braces(arg,left,WITH_SPACES);
  TempPointerHolder.next = common_subs->next;
  while ((ln=search_node_with_wild(&TempPointerHolder, left))!=NULL) {
    if (mesvar[MSG_SUB]) {
      if (*(ln->right)=='.' && !*(ln->right+1))
        sprintf(result, rs::rs(1170), ln->left);
      else
        sprintf(result, rs::rs(1171), ln->left);
      tintin_puts2(result);
    }
    TempPointerHolder.next=ln->next;
    deletenode_list(mysubs, ln);
    subnum--;
    flag=TRUE;
    if ( TempPointerHolder.next == NULL ) 
        break;
  }
  if (!flag && mesvar[MSG_SUB])
    tintin_puts2(rs::rs(1172));
}


void do_one_sub(char *line)
{
  struct listnode *ln;
  ln=common_subs;

    while((ln=ln->next)) 
        if(check_one_action(line,ln->left)) {
            // check its gag 
            if ( *ln->right == '.' && *(ln->right+1) == 0 ) {
                line[0] = '.';
                line[1] = 0;
                return;
            }
            BOOL bAnchored = FALSE;
            char pattern[BUFFER_SIZE],SubStr[BUFFER_SIZE],result[BUFFER_SIZE];
            prepare_actionalias(ln->left , pattern);
            prepare_actionalias(ln->right , SubStr);
            int pattern_len = strlen(pattern);
            if ( pattern_len == 0 ) 
                continue;

            result[0] = 0;
            char* line1 = line;
            char* ptr, *res = result;
            while ( (ptr = strstr(line1, pattern)) != NULL ) {
                strncpy(res, line1, ptr-line1 );
                res += ptr-line1;
                strcpy(res, SubStr);
                res += strlen(res);
                line1 += pattern_len + (ptr-line1);
            
            }
            if ( *line1 ) {
                strcat(result, line1);
            }
            strcpy(line, result);
            if(!bMultiSub)
			 return;
        
        }
}

//vls-begin// subst page
void DLLEXPORT RemoveSubst(char* pattern)
{
    struct listnode* ln;
    if((ln=searchnode_list(common_subs, pattern))!=NULL) {
      deletenode_list(common_subs, ln);
      subnum--;
    }
}

LPSTR DLLEXPORT SetSubst(char* text, char* pattern)
{
    struct listnode *ln;

    if ( !pattern || !*pattern) 
        return NULL;

    if((ln=searchnode_list(common_subs, pattern))!=NULL) {
        deletenode_list(common_subs, ln);
        subnum--;
    }

    if( text  ) {
        insertnode_list(common_subs, pattern, text, "0", ALPHA);
        subnum++;
        ln = searchnode_list(common_subs, pattern);
        return ln->left;
    }

    return NULL;
}

void DLLEXPORT SetSubstPattern(LPCSTR strOldPattern, LPCSTR strNewPattern)
{
    struct listnode *ln;
    char text[BUFFER_SIZE];
    if (!strOldPattern || !strcmp(strOldPattern, strNewPattern)) return;

    if((ln=searchnode_list(common_subs, (char *)strOldPattern))!=NULL) {
        strcpy(text, ln->left);
        deletenode_list(common_subs, ln);
        subnum--;
        SetSubst(text, (char*)strNewPattern);
    }
}

void DLLEXPORT GetSubstList(int* size)
{
    if (size)
        *size = subnum-1;
}

LPSTR DLLEXPORT GetSubst(int pos)
{
    listnode* ln = common_subs;
    pos++;
    int i = 0;
    while (ln) {
        if (i == pos) {
            return ln->left;
        }
        i++;
        ln = ln->next;
    }
    return NULL;
}

LPSTR DLLEXPORT GetSubstText(char* pattern)
{
    if (!pattern)
        return NULL;

    char p[BUFFER_SIZE];
    strcpy(p, pattern);
    listnode* ln = searchnode_list(common_subs, p);
    if (ln)
        return ln->right;
    return NULL;
}
//vls-end//
