/* Autoconf patching by David Hedbor, neotron@lysator.liu.se */
/*********************************************************************/
/* file: llist.c - linked-list datastructure                         */
/*                             TINTIN III                            */
/*          (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t             */
/*                     coded by peter unold 1992                     */
/*********************************************************************/
#include "stdafx.h"
#include "tintin.h"

void insertnode_list(struct listnode *listhead,const wchar_t *ltext, const wchar_t *rtext,const wchar_t *prtext,int mode);

/***************************************/
/* init list - return: ptr to listhead */
/***************************************/
struct listnode *init_list()
{
  struct listnode *listhead;

  if((listhead=(struct listnode *)(malloc(sizeof(struct listnode))))==NULL) {
    return NULL;
  }
  listhead->next=NULL;
  return(listhead);
}

/************************************************/
/* kill list - run throught list and free nodes */
/************************************************/ 
void kill_list(struct listnode *nptr)
{
  struct listnode *nexttodel;

  nexttodel=nptr->next;
  free(nptr);

  for(nptr=nexttodel; nptr; nptr=nexttodel) {
    nexttodel=nptr->next;
    free(nptr->left);
    free(nptr->right);
    free(nptr->pr);
    free(nptr);
  }
}
/********************************************************************
**   This function will clear all lists associated with a session  **
********************************************************************/
void killall_command(wchar_t*arg){KillAll(CLEAN,L"1");};
void kickall_command(wchar_t*arg){KillAll(CLEAN,L"0");};

void KillAll(int mode, wchar_t *arg)
{
	int Show = 0;
	if ( arg && *arg != L'0') {
		Show = 1;
	}

    GROUP_INDEX gind;
    // !!!  delete groups here, GROUP should remove EVERY object in it, but later
    gind = GroupList.begin();
    while ( gind != GroupList.end () ) {
        delete gind->second;
        gind ++;
    }
    GroupList.clear ();

    VAR_INDEX vind = VarList.begin();
    while (vind != VarList.end() ) {
        delete vind->second;
        vind++;
    }
    VarList.clear ();

    HOTKEY_INDEX hind = HotkeyList.begin();
    while ( hind != HotkeyList.end() ) {
        delete hind->second;
        hind++;
    }
    HotkeyList.clear ();

//vls-begin// script files
    SCRIPTFILE_INDEX sfind = ScriptFileList.begin();
    while ( sfind != ScriptFileList.end() ) {
        delete (*sfind);
        sfind++;
    }
    ScriptFileList.clear();
//vls-end//

switch (mode) {
  case CLEAN:
	GroupList[DEFAULT_GROUP_NAME] = new CGROUP(DEFAULT_GROUP_NAME);

    kill_list(common_subs);
    common_subs=init_list();
    kill_list(common_antisubs);
    common_antisubs=init_list();
    /* CHANGED to kill path stuff as well */
    kill_list(common_path);
    common_path=init_list();
    kill_list(common_pathdirs);
    common_pathdirs=init_pathdir_list();

	if ( Show ) 
		tintin_puts2(rs::rs(1094));
    acnum = subnum = hinum = antisubnum = 0;
   break;

  case END:
    kill_list(common_subs);
    kill_list(common_antisubs);
    kill_list(common_path);
    kill_list(common_pathdirs);
   break;
  } /* Switch */
 
}
/***********************************************/
/* make a copy of a list - return: ptr to copy */
/***********************************************/
struct listnode *copy_list(struct listnode *sourcelist, int mode)
{
  struct listnode *resultlist;

  resultlist=init_list();
  while((sourcelist=sourcelist->next))
    insertnode_list(resultlist, sourcelist->left, sourcelist->right, 
    sourcelist->pr, mode);
  
  return(resultlist);
} 

/*****************************************************************/
/* create a node containing the ltext, rtext fields and stuff it */
/* into the list - in lexicographical order, or by numerical     */
/* priority (dependent on mode) - Mods by Joann Ellsworth 2/2/94 */
/*****************************************************************/
void insertnode_list(struct listnode *listhead,const wchar_t *ltext, const wchar_t *rtext,const wchar_t *prtext,int mode)
{
  struct listnode *nptr, *nptrlast, *newnode;

  if((newnode=(struct listnode *)(malloc(sizeof(struct listnode))))==NULL) {
    return ;
/*???
    fprintf(stderr, "couldn't alloc listhead\n");
    exit(1);
*/
  }
  newnode->left=(wchar_t *)malloc((wcslen(ltext)+1)*sizeof(wchar_t));
  newnode->right=(wchar_t *)malloc((wcslen(rtext)+1)*sizeof(wchar_t));
  newnode->pr=(wchar_t *)malloc((wcslen(prtext)+1)*sizeof(wchar_t));
  wcscpy(newnode->left, ltext);
  wcscpy(newnode->right, rtext);
  wcscpy(newnode->pr, prtext);

  nptr=listhead;
  switch (mode) {
     case PRIORITY:
        while((nptrlast=nptr) && (nptr=nptr->next)) {
            if(wcscmp(prtext, nptr->pr)<0) {
                newnode->next=nptr;
                nptrlast->next=newnode;
                return;
            } else 
            if (wcscmp(prtext, nptr->pr)==0) {
	            while ((nptrlast) && (nptr) && 
	              (wcscmp(prtext, nptr->pr)==0)) {
                    if(wcscmp(ltext, nptr->left)<=0) {
                          newnode->next=nptr;
                          nptrlast->next=newnode;
                          return;
                    }
                    nptrlast=nptr;
                    nptr=nptr->next;
	            }
                    nptrlast->next=newnode;
                    newnode->next=nptr;
                return;
            }

        } 
        nptrlast->next=newnode;
        newnode->next=NULL;
     	return;
     	break;

     case ALPHA:
        while((nptrlast=nptr) &&   (nptr=nptr->next)) {
          if(wcscmp(ltext, nptr->left)<=0) {
            newnode->next=nptr;
            nptrlast->next=newnode;
            return;
          }
        }
     nptrlast->next=newnode;
     newnode->next=NULL;
     return;
     break;
  } /*  Switch  */
}

/*****************************/
/* delete a node from a list */
/*****************************/
void deletenode_list(struct listnode *listhead, struct listnode *nptr)
{
  struct listnode *lastnode=listhead;

  while((listhead=listhead->next)) {
    if(listhead==nptr) {
      lastnode->next=listhead->next;
      free(listhead->left);
      free(listhead->right);
      free(listhead->pr);
      free(listhead);
      return;
    }
    lastnode=listhead;
  }
  return;
}

/********************************************************/
/* search for a node containing the ltext in left-field */
/* return: ptr to node on succes / NULL on failure      */
/********************************************************/
struct listnode *searchnode_list(struct listnode *listhead, const wchar_t* cptr)
{
  int i;
  while((listhead=listhead->next)) {
    if((i=wcscmp(listhead->left, cptr))==0)
      return listhead;
    /* CHANGED to fix bug when list isn't alphabetically sorted
    else if(i>0)
      return NULL;
    */
  }
  return NULL;
}
/********************************************************/
/* search for a node that has cptr as a beginning       */
/* return: ptr to node on succes / NULL on failure      */
/* Mods made by Joann Ellsworth - 2/2/94                */
/********************************************************/
struct listnode *searchnode_list_begin(struct listnode *listhead, const wchar_t* cptr, int mode)
{
  int i;
  switch (mode) {
    case PRIORITY:
       while((listhead=listhead->next)) {
         if((i=wcsncmp(listhead->left, cptr, wcslen(cptr)))==0 &&
           (*(listhead->left+wcslen(cptr))==L' ' ||
           *(listhead->left+wcslen(cptr))==L'\0'))
           return listhead;
       }
       return NULL;
       break;

    case ALPHA:
       while((listhead=listhead->next)) {
         if((i=wcsncmp(listhead->left, cptr, wcslen(cptr)))==0 &&
           (*(listhead->left+wcslen(cptr))==L' ' ||
           *(listhead->left+wcslen(cptr))==L'\0'))
           return listhead;
         else if (i>0)
           return NULL;
       }
       return NULL;
       break;
    }
    return NULL;
}

/************************************/
/* show contens of a node on screen */
/************************************/
void shownode_list(struct listnode *nptr)
{
  wchar_t temp[BUFFER_SIZE];
  swprintf(temp, L"{%s}={%s}", nptr->left, nptr->right);
  tintin_puts2(temp);
}

/************************************/
/* list contens of a list on screen */
/************************************/
void show_list(struct listnode *listhead)
{
  while((listhead=listhead->next))
    shownode_list(listhead);
}

struct listnode *search_node_with_wild(struct listnode *listhead, const wchar_t* cptr)
{
  /* int i; */
  while((listhead=listhead->next)) {
    /* CHANGED to fix silly globbing behavior
    if(check_one_node(listhead->left, cptr))
    */
    if(match(cptr, listhead->left))
      return listhead;
  }
  return NULL;
}

int check_one_node(wchar_t *text, wchar_t *action)
{
  wchar_t *temp, temp2[BUFFER_SIZE], *tptr;
  while (*text && *action) {
    if (*action==L'*') {
      action++;
      temp=action;
      tptr=temp2;
      while(*temp && *temp !=L'*')
        *tptr++= *temp++;
      *tptr=L'\0';
      if (wcslen(temp2)==0) 
        return TRUE;
      while(wcsncmp(temp2,text,wcslen(temp2))!=0 && *text)
        text++;
    }
    else {
      temp=action;
      tptr=temp2;
      while (*temp && *temp !=L'*')
        *tptr++= *temp++;
      *tptr=L'\0';
      if(wcsncmp(temp2,text,wcslen(temp2))!=0)
        return FALSE;
      else {
        text+=wcslen(temp2);
        action+=wcslen(temp2);
      }
    }
  }
  if (*text)
    return FALSE;
  else if ((*action==L'*' && !*(action+1)) || !*action)
    return TRUE;
  return FALSE;
}

/*********************************************************************/
/* create a node containint the ltext, rtext fields and place at the */
/* end of a list - as insertnode_list(), but not alphabetical        */
/*********************************************************************/
void addnode_list(struct listnode *listhead, const wchar_t* ltext, const wchar_t* rtext, const wchar_t* prtext)
{
  struct listnode *newnode;

  if((newnode=(struct listnode *)malloc(sizeof(struct listnode)))==NULL) {
    return ;
/*???
    fprintf(stderr, "couldn't alloc listhead\n");
    exit(1);
*/
  }
  newnode->left=(wchar_t *)malloc((wcslen(ltext)+1)*sizeof(wchar_t));
  newnode->right=(wchar_t *)malloc((wcslen(rtext)+1)*sizeof(wchar_t));
  newnode->pr=(wchar_t *)malloc((wcslen(prtext)+1)*sizeof(wchar_t));
  newnode->next=NULL;
  wcscpy(newnode->left, ltext);
  wcscpy(newnode->right, rtext);
  wcscpy(newnode->pr, prtext);
  while (listhead->next!=NULL) (listhead=listhead->next);
  listhead->next=newnode;
}

int count_list(struct listnode *listhead)
{ 
	int count=0;
	struct listnode *nptr;

   nptr=listhead;
   while (nptr=nptr->next)
	++count;
   return(count);
}



