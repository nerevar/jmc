/* Autoconf patching by David Hedbor, neotron@lysator.liu.se */
/*********************************************************************/
/* file: session.c.c - funtions related to sessions                  */
/*                             TINTIN III                            */
/*          (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t             */
/*                     coded by peter unold 1992                     */
/*********************************************************************/
#include "stdafx.h"
#include "tintin.h"
#include "telnet.h"

void show_session();

extern char *get_arg_in_braces();
extern char *space_out();
extern char *mystrdup();
extern struct listnode *copy_list();
extern struct listnode *init_list();


/**********************************/
/* find a new session to activate */
/**********************************/
void  newactive_session()
{

    common_subs=init_list();
    common_antisubs=init_list();
    common_pathdirs=init_pathdir_list();
    common_path=init_list();
    mesvar[MSG_ALIAS]=DEFAULT_ALIAS_MESS;
    mesvar[MSG_ACTION]=DEFAULT_ACTION_MESS;
    mesvar[MSG_SUB]=DEFAULT_SUB_MESS;
    mesvar[MSG_ANTISUB]=DEFAULT_ANTISUB_MESS;
    mesvar[MSG_HIGH]=DEFAULT_HIGHLIGHT_MESS;
    mesvar[MSG_VAR]=DEFAULT_VARIABLE_MESS;
    mesvar[MSG_GRP]=DEFAULT_GROUP_MESS;
    mesvar[MSG_HOT]=DEFAULT_HOTKEY_MESS;
    mesvar[MSG_LOG]=DEFAULT_LOG_MESS;
}



/*****************************************************************************/
/* cleanup after session died. if session=activesession, try find new active */
/*****************************************************************************/
void cleanup_session(void)
{
    proxy_close(MUDSocket);
    MUDSocket = NULL;
	memset(&MUDAddress, 0, sizeof(MUDAddress));
    // bTickStatus=FALSE;
}

void connect_command(char* arg)
{
  char *host, *port;

  State = 0;

  if ( MUDSocket ) {
        tintin_puts2(rs::rs(1162));
        return;
  }

  if ( WaitForSingleObject(hConnThread , 0 ) == WAIT_TIMEOUT ) {
        tintin_puts2(rs::rs(1162));
        return;
  }
  
  port=host=space_out(mystrdup(arg));

  if(!*host) {
    tintin_puts2(rs::rs(1164));
    return ;
  }

  while(*port && !isspace(*port))
    port++;
  *port++='\0';
  port=space_out(port);

  if(!*port) {
    tintin_puts2(rs::rs(1165));
    return ;
  }
  connect_mud(host, port);
}
