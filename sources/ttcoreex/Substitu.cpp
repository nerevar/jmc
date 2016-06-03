/* Autoconf patching by David Hedbor, neotron@lysator.liu.se */
/*********************************************************************/
/* file: substitute.c - functions related to the substitute command  */
/*                             TINTIN III                            */
/*          (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t             */
/*                     coded by peter unold 1992                     */
/*********************************************************************/
#include "stdafx.h"
#include "tintin.h"

extern wchar_t *get_arg_in_braces();
extern struct listnode *search_node_with_wild();
extern struct listnode *searchnode_list();

//* en:multisub
BOOL bMultiSub = FALSE;
//* en


/***************************/
/* the #substitute command */
/***************************/
void parse_sub(wchar_t *arg)
{
  wchar_t left[BUFFER_SIZE], right[BUFFER_SIZE], result[BUFFER_SIZE];
  struct listnode *mysubs, *ln;
  mysubs=common_subs;
  arg=get_arg_in_braces(arg,left,STOP_SPACES,sizeof(left)/sizeof(wchar_t)-1);
  arg=get_arg_in_braces(arg,right,WITH_SPACES,sizeof(right)/sizeof(wchar_t)-1);

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
    insertnode_list(mysubs, left, right, L"0", ALPHA);
    subnum++;
    if (wcscmp(right,L".")!=0)
      swprintf(result, rs::rs(1168),right,left);
    else
      swprintf(result, rs::rs(1169), left);
    if (mesvar[MSG_SUB])
      tintin_puts2(result);
  }
}


/*****************************/
/* the #unsubstitute command */
/*****************************/

void unsubstitute_command(wchar_t *arg)
{
  wchar_t left[BUFFER_SIZE] ,result[BUFFER_SIZE];
  struct listnode *mysubs, *ln;
  struct listnode TempPointerHolder;
  int flag;
  flag=FALSE;
  mysubs=common_subs;
  arg=get_arg_in_braces(arg,left,WITH_SPACES,sizeof(left)/sizeof(wchar_t)-1);
  TempPointerHolder.next = common_subs->next;
  while ((ln=search_node_with_wild(&TempPointerHolder, left))!=NULL) {
    if (mesvar[MSG_SUB]) {
      if ( !wcscmp(ln->right, L".") )
        swprintf(result, rs::rs(1170), ln->left);
      else
        swprintf(result, rs::rs(1171), ln->left);
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


void do_one_sub(wchar_t *line)
{
  struct listnode *ln;
  ln=common_subs;

    while((ln=ln->next)) 
        if(check_one_action(line,ln->left)) {
            // check its gag 
            if ( !wcscmp(ln->right, L".") ) {
				wcscpy(line, L".");
                return;
            }
            BOOL bAnchored = FALSE;
            wchar_t pattern[BUFFER_SIZE],SubStr[BUFFER_SIZE],result[BUFFER_SIZE];
            prepare_actionalias(ln->left , pattern, sizeof(pattern)/sizeof(wchar_t));
            prepare_actionalias(ln->right , SubStr, sizeof(SubStr)/sizeof(wchar_t));
            int pattern_len = wcslen(pattern);
			int sublen = wcslen(SubStr);
            if ( pattern_len == 0 ) 
                continue;

            result[0] = 0;
            wchar_t* line1 = line;
            wchar_t* ptr, *res = result;
			int rest = sizeof(result)/sizeof(wchar_t) - 1;
            while ( (ptr = wcsstr(line1, pattern)) != NULL ) {
				int to_copy = ptr - line1;
				if (to_copy > rest)
					to_copy = rest;
                wcsncpy(res, line1, to_copy );
				res[to_copy] = L'\0';
                res += to_copy;
				rest -= to_copy;

				to_copy = sublen;
				if (to_copy > rest)
					to_copy = rest;
                wcsncpy(res, SubStr, to_copy);
				res[to_copy] = L'\0';
                res += to_copy;
				rest -= to_copy;

                line1 += pattern_len + (ptr-line1);
            
            }
            if ( *line1 ) {
                wcscat(result, line1);
            }
            wcscpy(line, result);
            if(!bMultiSub)
			 return;
        
        }
}

//vls-begin// subst page
void DLLEXPORT RemoveSubst(const wchar_t* pattern)
{
    struct listnode* ln;
    if((ln=searchnode_list(common_subs, pattern))!=NULL) {
      deletenode_list(common_subs, ln);
      subnum--;
    }
}

LPWSTR DLLEXPORT SetSubst(const wchar_t* text, const wchar_t* pattern)
{
    struct listnode *ln;

    if ( !pattern || !*pattern) 
        return NULL;

    if((ln=searchnode_list(common_subs, pattern))!=NULL) {
        deletenode_list(common_subs, ln);
        subnum--;
    }

    if( text  ) {
        insertnode_list(common_subs, pattern, text, L"0", ALPHA);
        subnum++;
        ln = searchnode_list(common_subs, pattern);
        return ln->left;
    }

    return NULL;
}

void DLLEXPORT SetSubstPattern(const wchar_t* strOldPattern, const wchar_t* strNewPattern)
{
    struct listnode *ln;
    wchar_t text[BUFFER_SIZE];
    if (!strOldPattern || !wcscmp(strOldPattern, strNewPattern)) return;

    if((ln=searchnode_list(common_subs, (wchar_t *)strOldPattern))!=NULL) {
        wcscpy(text, ln->left);
        deletenode_list(common_subs, ln);
        subnum--;
        SetSubst(text, (wchar_t*)strNewPattern);
    }
}

void DLLEXPORT GetSubstList(int* size)
{
    if (size)
        *size = subnum-1;
}

LPWSTR DLLEXPORT GetSubst(int pos)
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

LPWSTR DLLEXPORT GetSubstText(const wchar_t* pattern)
{
    if (!pattern)
        return NULL;

    wchar_t p[BUFFER_SIZE];
    wcscpy(p, pattern);
    listnode* ln = searchnode_list(common_subs, p);
    if (ln)
        return ln->right;
    return NULL;
}
//vls-end//
