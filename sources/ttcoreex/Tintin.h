/*********************************************************************/
/* file: tintin.h - the include file for tintin++                    */
/*                             TINTIN ++                             */
/*          (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t             */
/*                    modified by Bill Reiss 1993                    */
/*                     coded by peter unold 1992                     */
/*********************************************************************/

//vls-begin// bugfix
#ifndef TINTIN_H
#define TINTIN_H
//vls-end//

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

//vls-begin// #play
#include <mmsystem.h>
//vls-end//

#include "../recore/pcre.h"
#include <string>
#include <map>
#include <list>
#include <fstream>

#include "tintinx.h"
#include "ttobjects.h"

using namespace std;

// for the get_arg_in_braces
//* en
//* MESSED UP
//#define WITH_SPACES 0
//#define STOP_SPACES 1
#define STOP_SPACES 0
#define WITH_SPACES 1
//* /en


/***********************************************/
/* Some default values you might wanna change: */
/***********************************************/
#define ALPHA 1
#define PRIORITY 0
#define CLEAN 0
#define END 1
#define DEFAULT_OPEN '{' /*character that starts an argument */
#define DEFAULT_CLOSE '}' /*character that ends an argument */
#define MAX_PATH_LENGTH 200               /* max path lenght */
#define DEFAULT_VERBATIM_CHAR '\\'        /* if an input starts with this
                                             char, it will be sent 'as is'
                                             to the MUD */
#define DEFAULT_ECHO FALSE                /* echo */         
#define DEFAULT_IGNORE FALSE              /* ignore */
#define DEFAULT_SPEEDWALK TRUE           /* speedwalk */
#define DEFAULT_PRESUB TRUE               /* presub before actions */
#define DEFAULT_TOGGLESUBS FALSE          /* turn subs on and off FALSE=ON*/

#define DEFAULT_ALIAS_MESS TRUE           /* messages for responses */
#define DEFAULT_ACTION_MESS TRUE          /* when setting/deleting aliases, */
#define DEFAULT_SUB_MESS TRUE             /* actions, etc. may be set to */
#define DEFAULT_ANTISUB_MESS TRUE         /* default either on or off */
#define DEFAULT_HIGHLIGHT_MESS TRUE       /* TRUE=ON FALSE=OFF */
#define DEFAULT_VARIABLE_MESS TRUE        /* might want to turn off these */
#define DEFAULT_GROUP_MESS TRUE
#define DEFAULT_HOTKEY_MESS TRUE
#define DEFAULT_LOG_MESS TRUE

enum {
    MSG_ALIAS = 0, 
    MSG_ACTION,
    MSG_SUB, 
    MSG_ANTISUB, 
    MSG_HIGH,
    MSG_VAR, 
    MSG_GRP, 
//vls-begin// script files
//    MSG_HOT
    MSG_HOT,
    MSG_SF,
	MSG_LOG
//vls-end//
};

/**************************************************************************/
/* the codes below are used for highlighting text, and is set for the     */
/* codes for VT-100 terminal emulation. If you are using a different      */
/* teminal type, replace the codes below with the correct codes and       */
/* change the codes set up in highlight.c                                 */
/**************************************************************************/
#define DEFAULT_BEGIN_COLOR "["
#define DEFAULT_END_COLOR "[0m"
/*************************************************************************/
/* The text below is checked for. If it trickers then echo is turned off */
/* echo is turned back on the next time the user types a return          */
/*************************************************************************/
#define PROMPT_FOR_PW_TEXT "assword:"

#define DEFAULT_GROUP_NAME "default" 

/**************************************************************************/ 
/* The stuff below here shouldn't be modified unless you know what you're */
/* doing........                                                          */
/**************************************************************************/ 

/************************ structures *********************/
struct listnode {
  struct listnode *next;
  char *left, *right, *pr;
  char group[32];
};


struct completenode {
  struct completenode *next;
  char *strng;
};


/// ATTENTION ----------------------------------------
#undef isspace
#define isspace(c) (c < 127 && ((c >= 0x9 && c<=0xD) || c == 0x20 ))


extern int verbatim;
extern int echo;
//vls-begin// script files
//extern int mesvar[8];
extern int mesvar[10];
//vls-end//
extern int acnum,subnum,hinum,antisubnum;
extern int verbose;
extern int presub;
extern int togglesubs;
extern int speedwalk;
extern char verbatim_char;    
extern BOOL bPathing;

//vls-begin// #logadd + #logpass
extern BOOL bLogPassedLine;
//vls-end//

//* en
extern BOOL bContinuedAction; //next
extern BOOL bDaaMessage;      //daa
extern BOOL bMultiSub;        //multisub
extern BOOL bColon;           //colon
extern BOOL bSosExact;        //sos
extern int iWaitState;        //wait
extern char cCommentChar;        
extern const sTimersV;
typedef struct {
	BOOL isActive;
	int  startV;
	int  endV;
	int  counter;
	int  step;
	int  delay;
	int  capacity;
	char command[BUFFER_SIZE];
} sTimer;
extern DWORD dwSTime;
extern char race_format[BUFFER_SIZE];
extern char race_last[BUFFER_SIZE];

extern char sLogName[BUFFER_SIZE];
extern char sOutputLogName[MAX_OUTPUT][BUFFER_SIZE];
//* /en

extern struct completenode *complete_head;
extern tick_size;
extern int ticker_interrupted;
extern DWORD dwTime0;
extern HWND hwndMAIN;

extern HANDLE hConnThread;

//---------------------------------------------------------

extern std::map<int, TIMER*> TIMER_LIST;

extern BOOL bMultiAction, bMultiHighlight;
extern struct listnode *common_subs;
extern struct listnode *common_antisubs, *common_pathdirs, *common_path;
extern char vars[10][BUFFER_SIZE]; /* the %0, %1, %2,....%9 variables */
extern ofstream hLogFile;
//vls-begin// multiple output
extern ofstream hOutputLogFile[MAX_OUTPUT];
//vls-end//
extern int path_length;
extern int old_more_coming,more_coming;
extern char last_line[BUFFER_SIZE];
extern int ignore;

extern SOCKET MUDSocket;

/* ------ Extern functions implemented in exe file ----- */
/* typedef BOOL (CALLBACK* DLGPROC)(HWND, UINT, WPARAM, LPARAM); */


void tintin_puts2(char *cptr);

//vls-begin// multiple output
//void tintin_puts3(char *cptr); // place to output window
void tintin_puts3(char *cptr, int wnd); // place to output window number wnd
//vls-end//

/* void EndApplication(); Finish programm imeidiatly ! */
void write_line_mud(char *line);
void  connect_mud(char *host, char *port);
void ShowError (char* strError);

/*==================    Internal functions  ========================*/
void show_list(struct listnode *listhead);
void show_list_action(struct listnode *listhead);
void shownode_list(struct listnode *nptr);
void shownode_list_action(struct listnode *nptr);
struct listnode *searchnode_list( struct listnode *listhead, char *cptr);
void deletenode_list(struct listnode *listhead, struct listnode *nptr);
void insertnode_list(struct listnode *listhead, char *ltext, char *rtext,char *prtext, int mode);
int count_list(struct listnode *listhead);
struct listnode *search_node_with_wild(struct listnode *listhead,char *cptr);
struct listnode *searchnode_list_begin(struct listnode *listhead, char* cptr, int mode);
void kill_list(struct listnode *nptr);
void addnode_list(struct listnode *listhead, char *ltext, char *rtext, char *prtext);
int match(char *regex, char *string);
char *mystrdup(char *s);
char *space_out(char* s);
void add_codes(char *line, char *result, char *htype, BOOL bAddTAil= TRUE);
void tintin_puts(char *cptr);
void substitute_myvars(char *arg, char *result);
int check_one_action(char *line, ACTION *action);
int check_one_action(char* line, char* action);
void parse_input(char *input);
int check_a_action(char *line, char *action);
BOOL show_aliases(char* left = NULL, CGROUP* pGroup= NULL);
char *get_arg_in_braces( char *s, char *arg, int flag);
int is_abrev( char *s1, char *s2);
void prepare_actionalias( char *string, char *result);
void substitute_vars(char *arg, char *result);
int eval_expression(char *arg);
int conv_to_ints(char *arg);
void cleanup_session(void);
int is_speedwalk_dirs(char *cp);
BOOL show_high(CGROUP* pGroup = NULL);
void KillAll(int mode, char* arg);
//////////////////AND NOW TINTIN COMMANDS////////////
void killall_command(char*arg);
void kickall_command(char*arg);
void gag_command(char*arg);
void parse_antisub(char *arg);
void alias_command(char *arg);
void char_command( char *arg);
void verbatim_command(char* arg);
void bell_command(char *arg);
void cr_command(char *arg);
void echo_command(char* arg);
void end_command(char *command);
void help_command(char *arg);
void parse_high(char *arg);
void if_command(char *line);
void ignore_command(char* arg);
void display_info(char* arg);
void log_command(char *arg);
void loop_command(char *arg);
void map_command(char *arg);
void math_command(char *line);
void mark_command(char *arg);
void message_command(char *arg);
void pathdir_command(char *arg);
void presub_command(char* arg);
void return_command(char*arg);
void showme_command(char *arg);
void speedwalk_command(char* arg);
void read_file(char *arg);
void parse_sub(char *arg);
void tickoff_command(char* arg);
void tickon_command(char* arg);
void tickset_command(char* arg);
void ticksize_command(char *arg);
void tick_command(char* arg);
void ticksize_command(char *arg);
void tolower_command(char *arg);
void togglesubs_command(char* arg);
void toupper_command(char *arg);
void unaction_command(char *arg);
void savepath_command(char *arg);
void unalias_command(char *arg);
void unantisubstitute_command(char *arg);
void unsubstitute_command(char *arg);
void unpath_command(char* arg);
void var_command(char *arg);
void unvar_command(char *arg);
void check_insert_path(char *command);
void path_command(char* arg);
void action_command(char *arg);
void unhighlight_command(char *arg);
void script_command(char *arg);
void write_command(char *filename);
void output_command(char* arg);
void zap_command(char* arg);
void group_command(char* arg);
void tabadd_command(char* arg);
void tabdel_command(char* arg);
//vls-begin// #quit
void quit_command(char *arg);
//vls-begin// #reloadscripts
void reloadscripts_command(char *arg);
//vls-begin// script files
void use_command(char *arg);
void unuse_command(char *arg);
//vls-begin// #system
void systemexec_command(char *arg);
void systemlist_command(char *arg);
void systemkill_command(char *arg);
//vls-begin// #run
void run_command(char *arg);
//vls-begin// #play
void play_command(char *arg);
//vls-end//
//vls-begin// multiple output
void woutput_command(char *arg);
void wshow_command(char *arg);
void wlog_command(char *arg);
void wname_command(char *arg);
//vls-end//
//vls-begin// #logadd + #logpass
void logadd_command(char *arg);
void logpass_command(char *arg);
//vls-end//
//vls-begin// #flash
// Misc.cpp
void flash_command(char *arg);
//vls-end//
//* en
// action.cpp
void next_command(char* arg);
// misc.cpp
void daa_command(char *arg);
void colon_command(char *arg);
void do_cycle(int b1, int b2, int step, int delay, char *command);
void wt_command(char *arg);
void comment_command(char *arg);
void MultiSub_command(char *arg);
void update_timers(int nTime);
void break_timer_command(char *arg);
void continue_timer_command(char *arg);
void tm_list_command(char *arg);
void help_command(char *arg);
void clean_command(char *arg);
void winamp_command(char *arg);
// text.cpp
void spit_command(char *arg);
void grab_command(char *arg);
// path.cpp
void race_command(char *arg);
// files.cpp
void sos_command(char *arg);
// ttcoreex.cpp
void wdock_command(char *arg);
void wpos_command(char *arg);
// parse.cpp
void prefix_command(char *arg);
//* /en
void check_all_actions(char *line);
void do_one_sub(char *line);
void do_one_high(char *line);
void read_command(char* filename);
void connect_command(char* arg);
void drop_command(char* arg);
void nodrop_command(char* arg);
void MultiactionCommand(char* arg);
void MultiHlightCommand(char* arg);
void SetHotKey(char* arg);
void Unhotkey(char* arg);
void nope_command(char *arg);
void abort_timer_command(char*arg);
void autoreconnect_command(char*arg);

// VARIABLES:
void variable_value_input(char *arg);
void variable_value_date(char *arg);
void variable_value_year(char *arg);
void variable_value_month(char *arg);
void variable_value_day(char *arg);
void variable_value_time(char *arg);
void variable_value_hour(char *arg);
void variable_value_minute(char *arg);
void variable_value_second(char *arg);
void variable_value_millisecond(char *arg);
void variable_value_timestamp(char *arg);
void variable_value_color_default(char *arg);

BOOL show_actions(char* left = NULL, CGROUP* pGroup = NULL);
int do_one_antisub(char *line);
extern int SocketFlags;
extern unsigned char State;
int do_telnet_protecol(unsigned char* cpsource, char* cpdest, int size);
//vls-begin// multiple output
void StopLogging();
void log(string st);
void log(int wnd, string st);
string processLine(char *strInput);
string processTEXT(string strInput);
string processRMA(string strInput);
string processHTML(string strInput);
//vls-end//
struct listnode *init_list(void);
struct listnode *init_pathdir_list(void);
void newactive_session();
void status_command(char* arg);
void ParseScriptlet2(BSTR bstrScriptlet);

//-----------------------------------------------
typedef std::map <std::string, ALIAS*> ALIASLIST ;
typedef ALIASLIST::iterator ALIAS_INDEX ;
extern ALIASLIST AliasList;

typedef std::map <std::string, CGROUP*> GROUPLIST ;
typedef GROUPLIST ::iterator GROUP_INDEX ;
extern GROUPLIST  GroupList;

namespace std
{
    struct greater<ACTION*> : public binary_function<ACTION*, ACTION*, bool> {
    bool operator()(const ACTION*& x, const ACTION*& y) const{return x->m_nPriority < y->m_nPriority; };
    };
}


// typedef list<CActionPtr> ACTIONLIST;
typedef std::list<ACTION*> ACTIONLIST;
typedef ACTIONLIST::iterator ACTION_INDEX;
extern ACTIONLIST ActionList;

extern BOOL bDelayedActionDelete;

typedef std::map <std::string, VAR*> VARLIST;
typedef VARLIST::iterator VAR_INDEX ;
extern VARLIST  VarList;

typedef std::map <std::string, HLIGHT*> HLIGHTLIST ;
typedef HLIGHTLIST::iterator HLIGHT_INDEX ;
extern HLIGHTLIST HlightList;

typedef std::map <WORD, CHotKey*>::iterator HOTKEY_INDEX ;
extern std::map <WORD, CHotKey*> HotkeyList;

//vls-begin// script files
typedef std::list<CScriptFile *> SCRIPTFILELIST;
typedef SCRIPTFILELIST::iterator SCRIPTFILE_INDEX;
extern SCRIPTFILELIST ScriptFileList;   // nodes.cpp
//vls-end//

extern void* JMCObjRet[1000];

// --CHANGED by Anton Likhtarov for resource support
#include "ressup.h"
// --END

//* en:JMC functions struct. look cmds.h
const JMC_CMDS_NUM=112;
typedef struct jmc_cmd 
	{
	char*alias;
	void (*jmcfn)(char*);
	char*hlpfile;
	} jmc_cmdi;
//*/en


const JMC_SPECIAL_VARIABLES_NUM = 12;
typedef struct jmc_special_variable_struct {
	char *name;
	void (*jmcfn)(char*);
} jmc_special_variable;


//vls-begin// bugfix
//#include "cmds.h"
#endif
//vls-end//

