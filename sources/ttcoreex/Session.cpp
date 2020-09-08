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

extern wchar_t *get_arg_in_braces();
extern wchar_t *space_out();
extern struct listnode *copy_list();
extern struct listnode *init_list();


/**********************************/
/* find a new session to activate */
/**********************************/
void  newactive_session()
{
	if (common_subs)
		kill_list(common_subs);
	if (common_antisubs)
		kill_list(common_antisubs);
	if (common_pathdirs)
		kill_list(common_pathdirs);
	if (common_path)
		kill_list(common_path);

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
	mesvar[MSG_TELNET]=DEFAULT_TELNET_MESS;
	mesvar[MSG_MUD_OOB]=DEFAULT_OOB_MESS;
	mesvar[MSG_MAPPER]=DEFAULT_MAPPER_MESS;
}



/*****************************************************************************/
/* cleanup after session died. if session=activesession, try find new active */
/*****************************************************************************/
void cleanup_session(void)
{
	tls_close(MUDSocket);
    proxy_close(MUDSocket);
    MUDSocket = NULL;
	memset(&MUDAddress, 0, sizeof(MUDAddress));
    // bTickStatus=FALSE;
}

void connect_command(wchar_t* arg)
{
  wchar_t host[BUFFER_SIZE], port[BUFFER_SIZE];

  State = 0;

  if ( MUDSocket ) {
        tintin_puts2(rs::rs(1162));
        return;
  }

  if ( WaitForSingleObject(hConnThread , 0 ) == WAIT_TIMEOUT ) {
        tintin_puts2(rs::rs(1162));
        return;
  }

  arg = get_arg_in_braces(arg,host,STOP_SPACES,sizeof(host)/sizeof(wchar_t)-1);
  arg = get_arg_in_braces(arg,port,STOP_SPACES,sizeof(port)/sizeof(wchar_t)-1);
  
  if(!*host) {
    tintin_puts2(rs::rs(1164));
    return ;
  }

  if(!*port) {
    tintin_puts2(rs::rs(1165));
    return ;
  }
  connect_mud(host, port);
}
