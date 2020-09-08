#include "stdafx.h"
#include "tintin.h"

struct listnode *searchnode_list();
struct listnode *search_node_with_wild();

/***************************/
/* the #substitute command */
/***************************/
void parse_antisub(wchar_t *arg)
{
  wchar_t left[BUFFER_SIZE], result[BUFFER_SIZE];
  struct listnode *myantisubs, *ln;
  myantisubs=common_antisubs;
  arg=get_arg_in_braces(arg,left,WITH_SPACES,sizeof(left)/sizeof(wchar_t)-1);

  if(!*left) {
    tintin_puts2(rs::rs(1020));
    show_list(myantisubs);
  }
  else {

    if((ln=searchnode_list(myantisubs, left))!=NULL){
      deletenode_list(myantisubs, ln);
      antisubnum--;
    }
    insertnode_list(myantisubs, left, left, L"0", ALPHA);
    antisubnum++;
    if (mesvar[MSG_ANTISUB]) {
      swprintf(result, rs::rs(1021), left);
      tintin_puts2(result);
    }
  }
}


void unantisubstitute_command(wchar_t *arg)
{
  wchar_t left[BUFFER_SIZE] ,result[BUFFER_SIZE];
  struct listnode *myantisubs, *ln, *temp;
  int flag;
  flag=FALSE;
  myantisubs=common_antisubs;
  temp=myantisubs;
  arg=get_arg_in_braces(arg,left,WITH_SPACES,sizeof(left)/sizeof(wchar_t)-1);
  while ((ln=search_node_with_wild(temp, left))!=NULL) {
    if (mesvar[MSG_ANTISUB]) {
      swprintf(result, rs::rs(1022), ln->left);
      tintin_puts2(result);
    }
    deletenode_list(myantisubs, ln);
    antisubnum--;
    flag=TRUE;
    temp=ln;
  }
  if (!flag && mesvar[MSG_ANTISUB])
    tintin_puts2(rs::rs(1023));
}




int do_one_antisub(wchar_t *line)
{
  struct listnode *ln;
  ln=common_antisubs;

  while((ln=ln->next)) 
      if(check_one_action(line,ln->left))
	return TRUE;
  return FALSE;
}

