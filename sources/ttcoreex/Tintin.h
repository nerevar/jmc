/**********f***********************************************************/
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
#include <vector>
#include <map>
#include <set>
#include <list>
#include <fstream>
#include <algorithm>

#include "tintinx.h"
#include "ttobjects.h"

#include "mlang.h"

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
#define ANSI_COMMAND_CHAR		L'\x1B'
#define DEFAULT_OPEN L'{' /*character that starts an argument */
#define DEFAULT_CLOSE L'}' /*character that ends an argument */
#define MAX_PATH_LENGTH 200               /* max path lenght */
#define DEFAULT_VERBATIM_CHAR L'\\'        /* if an input starts with this
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
#define DEFAULT_TELNET_MESS FALSE

enum {
    MSG_ALIAS = 0, 
    MSG_ACTION,
    MSG_SUB, 
    MSG_ANTISUB, 
    MSG_HIGH,
    MSG_VAR, 
    MSG_GRP, 
    MSG_HOT,
    MSG_SF,
	MSG_LOG,
	MSG_TELNET,

	MSG_MAXNUM
};

/**************************************************************************/
/* the codes below are used for highlighting text, and is set for the     */
/* codes for VT-100 terminal emulation. If you are using a different      */
/* teminal type, replace the codes below with the correct codes and       */
/* change the codes set up in highlight.c                                 */
/**************************************************************************/
#define DEFAULT_BEGIN_COLOR L"["
#define DEFAULT_END_COLOR L"[0m"
/*************************************************************************/
/* The text below is checked for. If it trickers then echo is turned off */
/* echo is turned back on the next time the user types a return          */
/*************************************************************************/

#define DEFAULT_GROUP_NAME L"default" 
/**************************************************************************/ 
/* The stuff below here shouldn't be modified unless you know what you're */
/* doing........                                                          */
/**************************************************************************/ 

/************************ structures *********************/
struct listnode {
  struct listnode *next;
  wchar_t *left, *right, *pr;
  wchar_t group[32];
};


struct completenode {
  struct completenode *next;
  wchar_t *strng;
};

extern int verbatim;
extern int echo;
extern int mesvar[MSG_MAXNUM];
extern int acnum,subnum,hinum,antisubnum;
extern int verbose;
extern int presub;
extern int togglesubs;
extern int speedwalk;
extern wchar_t verbatim_char;    
extern BOOL bPathing;

//vls-begin// #logadd + #logpass
extern BOOL bLogPassedLine;
//vls-end//

extern std::vector<unsigned int> vEnabledTelnetOptions;
extern BOOL DLLEXPORT bTelnetDebugEnabled;

//* en
extern BOOL bContinuedAction; //next
extern BOOL bDaaMessage;      //daa
extern BOOL bMultiSub;        //multisub
extern BOOL bColon;           //colon
extern BOOL bSosExact;        //sos
extern int iWaitState;        //wait
extern wchar_t cCommentChar;        
extern const int sTimersV;
typedef struct {
	BOOL isActive;
	int  startV;
	int  endV;
	int  counter;
	int  step;
	int  delay;
	int  capacity;
	wchar_t command[BUFFER_SIZE];
} sTimer;
extern DWORD dwSTime;
extern wchar_t race_format[BUFFER_SIZE];
extern wchar_t race_last[BUFFER_SIZE];

extern wchar_t sLogName[BUFFER_SIZE];
extern wchar_t sOutputLogName[MAX_OUTPUT][BUFFER_SIZE];
//* /en

extern struct completenode *complete_head;
extern int tick_size;
extern int ticker_interrupted;
extern DWORD dwTime0;
extern HWND hwndMAIN;

extern HANDLE hConnThread;

extern std::map< UINT, std::wstring > CPNames;
extern std::map< std::wstring, UINT > CPIDs;

//---------------------------------------------------------

extern std::map<int, TIMER*> TIMER_LIST;

extern BOOL bMultiAction, bMultiHighlight;
extern struct listnode *common_subs;
extern struct listnode *common_antisubs, *common_pathdirs, *common_path;
extern wchar_t vars[10][BUFFER_SIZE]; /* the %0, %1, %2,....%9 variables */
extern ofstream hLogFile;
//vls-begin// multiple output
extern ofstream hOutputLogFile[MAX_OUTPUT];
//vls-end//
extern int path_length;
//extern int old_more_coming,more_coming;
extern int more_coming;
extern wchar_t last_line[BUFFER_SIZE];
extern int ignore;

extern SOCKET MUDSocket;
extern sockaddr_in MUDAddress;

extern LONG DLLEXPORT lPingMUD;
extern LONG DLLEXPORT lPingProxy;

typedef enum {
	TLS_DISABLED = 0,
	TLS_SSL3,
	TLS_TLS1,
	TLS_TLS1_1,
	TLS_TLS1_2
} TLSType;
extern TLSType DLLEXPORT lTLSType;
extern wstring DLLEXPORT strCAFile;

typedef struct {
	DWORD timestamp;
	wstring line;
} ScrollLineRec;
extern ScrollLineRec *pScrollLinesBuffer;
extern int ScrollBufferCapacity, ScrollBufferBegin, ScrollBufferEnd;
void add_line_to_scrollbuffer(const wchar_t *line);

extern UINT DLLEXPORT uBroadcastMessage;

/* ------ Extern functions implemented in exe file ----- */
/* typedef BOOL (CALLBACK* DLGPROC)(HWND, UINT, WPARAM, LPARAM); */


extern HMODULE hMlang;
extern HRESULT (__stdcall *fConvertINetMultiByteToUnicode)(LPDWORD, DWORD, LPCSTR, LPINT, LPWSTR, LPINT);
extern void DLLEXPORT utf16le_to_utf16be(wchar_t *dst, const wchar_t *src, int count);
extern int DLLEXPORT read_file_contents(const wchar_t *FilePath, wchar_t *Buffer, int Capacity);
extern int DLLEXPORT write_file_contents(const wchar_t *FilePath, const wchar_t *Buffer, int Length);
void codepage_command(wchar_t *arg);

void tintin_puts2(const wchar_t *cptr);

//vls-begin// multiple output
void tintin_puts3(const wchar_t *cptr, int wnd); // place to output window number wnd
//vls-end//

/* void EndApplication(); Finish programm imeidiatly ! */
void write_line_mud(const wchar_t *line);
void  connect_mud(wchar_t *host, wchar_t *port);
void ShowError (wchar_t* strError);

/*==================    Internal functions  ========================*/
void show_list(struct listnode *listhead);
void show_list_action(struct listnode *listhead);
void shownode_list(struct listnode *nptr);
void shownode_list_action(struct listnode *nptr);
struct listnode *searchnode_list( struct listnode *listhead, const wchar_t *cptr);
void deletenode_list(struct listnode *listhead, struct listnode *nptr);
void insertnode_list(struct listnode *listhead, const wchar_t *ltext, const wchar_t *rtext,const wchar_t *prtext, int mode);
int count_list(struct listnode *listhead);
struct listnode *search_node_with_wild(struct listnode *listhead, const wchar_t *cptr);
struct listnode *searchnode_list_begin(struct listnode *listhead, const wchar_t* cptr, int mode);
void kill_list(struct listnode *nptr);
void addnode_list(struct listnode *listhead, const wchar_t *ltext, const wchar_t *rtext, const wchar_t *prtext);
int match(const wchar_t *regex, const wchar_t *string);
wchar_t *space_out(wchar_t* s);
void add_codes(const wchar_t *line, wchar_t *result, const wchar_t *htype, BOOL bAddTAil= TRUE);
void remove_ansi_codes(const wchar_t *input, wchar_t *output);
void convert_ansi_to_colored(const wchar_t *input, wchar_t *output, int maxlength, int &initial_state);
void convert_colored_to_ansi(const wchar_t *input, wchar_t *output, int maxlength);
void tintin_puts(const wchar_t *cptr);
bool is_allowed_symbol(wchar_t arg);
void substitute_myvars(const wchar_t *arg, wchar_t *result, int maxlength);
int check_one_action(const wchar_t* line, ACTION *action, int *offset = NULL);
int check_one_action(const wchar_t* line, const wchar_t* action);
void parse_input(wchar_t *input, BOOL bExecuteNow = FALSE);
void free_parse_stack();
int check_a_action(const wchar_t *line, const wchar_t *action);
BOOL show_aliases(wchar_t* left = NULL, CGROUP* pGroup= NULL);
wchar_t *get_arg_in_braces( wchar_t *s, wchar_t *arg, int flag, int maxlength);
int is_all_digits(const wchar_t *number);
int is_abrev(const wchar_t *s1, const wchar_t *s2);
void prepare_actionalias(const wchar_t *string, wchar_t *result, int maxlength);
void substitute_vars(const wchar_t *arg, wchar_t *result, int maxlength);
int eval_expression(wchar_t *arg);
int conv_to_ints(wchar_t *arg);
void cleanup_session(void);
int is_speedwalk_dirs(wchar_t *cp);
BOOL show_high(CGROUP* pGroup = NULL);
void KillAll(int mode, wchar_t* arg);
//////////////////AND NOW TINTIN COMMANDS////////////
void killall_command(wchar_t*arg);
void kickall_command(wchar_t*arg);
void gag_command(wchar_t*arg);
void parse_antisub(wchar_t *arg);
void alias_command(wchar_t *arg);
void char_command( wchar_t *arg);
void verbatim_command(wchar_t* arg);
void bell_command(wchar_t *arg);
void cr_command(wchar_t *arg);
void echo_command(wchar_t* arg);
void end_command(wchar_t *command);
void help_command(wchar_t *arg);
void parse_high(wchar_t *arg);
void if_command(wchar_t *line);
void strcmp_command(wchar_t *line);
void ignore_command(wchar_t* arg);
void display_info(wchar_t* arg);
void log_command(wchar_t *arg);
void loop_command(wchar_t *arg);
void map_command(wchar_t *arg);
void math_command(wchar_t *line);
void mark_command(wchar_t *arg);
void message_command(wchar_t *arg);
void pathdir_command(wchar_t *arg);
void presub_command(wchar_t* arg);
void return_command(wchar_t*arg);
void showme_command(wchar_t *arg);
void speedwalk_command(wchar_t* arg);
void read_file(wchar_t *arg);
void parse_sub(wchar_t *arg);
void tickoff_command(wchar_t* arg);
void tickon_command(wchar_t* arg);
void tickset_command(wchar_t* arg);
void ticksize_command(wchar_t *arg);
void tick_command(wchar_t* arg);
void ticksize_command(wchar_t *arg);
void tolower_command(wchar_t *arg);
void togglesubs_command(wchar_t* arg);
void toupper_command(wchar_t *arg);
void unaction_command(wchar_t *arg);
void savepath_command(wchar_t *arg);
void unalias_command(wchar_t *arg);
void unantisubstitute_command(wchar_t *arg);
void unsubstitute_command(wchar_t *arg);
void unpath_command(wchar_t* arg);
void var_command(wchar_t *arg);
void unvar_command(wchar_t *arg);
void check_insert_path(wchar_t *command);
void path_command(wchar_t* arg);
void action_command(wchar_t *arg);
void unhighlight_command(wchar_t *arg);
void script_command(wchar_t *arg);
void write_command(wchar_t *filename);
void output_command(wchar_t* arg);
void zap_command(wchar_t* arg);
void group_command(wchar_t* arg);
void tabadd_command(wchar_t* arg);
void tabdel_command(wchar_t* arg);
//vls-begin// #quit
void quit_command(wchar_t *arg);
void hidewindow_command(wchar_t *arg);
void restorewindow_command(wchar_t *arg);
void systray_command(wchar_t *arg);
//vls-begin// #reloadscripts
void reloadscripts_command(wchar_t *arg);
//vls-begin// script files
void use_command(wchar_t *arg);
void unuse_command(wchar_t *arg);
//vls-begin// #system
void systemexec_command(wchar_t *arg);
void systemlist_command(wchar_t *arg);
void systemkill_command(wchar_t *arg);
//vls-begin// #run
void run_command(wchar_t *arg);
//vls-begin// #play
void play_command(wchar_t *arg);
//vls-end//
//vls-begin// multiple output
void woutput_command(wchar_t *arg);
void wshow_command(wchar_t *arg);
void wlog_command(wchar_t *arg);
void wname_command(wchar_t *arg);
//vls-end//
//vls-begin// #logadd + #logpass
void logadd_command(wchar_t *arg);
void logpass_command(wchar_t *arg);
//vls-end//
//vls-begin// #flash
// Misc.cpp
void flash_command(wchar_t *arg);
//vls-end//
//* en
// action.cpp
void next_command(wchar_t* arg);
// misc.cpp
void daa_command(wchar_t *arg);
void colon_command(wchar_t *arg);
void do_cycle(int b1, int b2, int step, int delay, wchar_t *command);
void wt_command(wchar_t *arg);
void comment_command(wchar_t *arg);
void MultiSub_command(wchar_t *arg);
void update_timers(int nTime);
void break_timer_command(wchar_t *arg);
void continue_timer_command(wchar_t *arg);
void tm_list_command(wchar_t *arg);
void help_command(wchar_t *arg);
void clean_command(wchar_t *arg);
void winamp_command(wchar_t *arg);
// text.cpp
void spit_command(wchar_t *arg);
void grab_command(wchar_t *arg);
// path.cpp
void race_command(wchar_t *arg);
// files.cpp
void sos_command(wchar_t *arg);
// ttcoreex.cpp
void wdock_command(wchar_t *arg);
void wpos_command(wchar_t *arg);
void wsize_command(wchar_t *arg);
// parse.cpp
void prefix_command(wchar_t *arg);
//* /en
void check_all_actions(wchar_t *line, bool multiline);
void do_one_sub(wchar_t *line);
void do_one_high(wchar_t *line);
void read_command(wchar_t* filename);
void connect_command(wchar_t* arg);
void drop_command(wchar_t* arg);
void nodrop_command(wchar_t* arg);
void replace_command(wchar_t* arg);
void MultiactionCommand(wchar_t* arg);
void MultiHlightCommand(wchar_t* arg);
void SetHotKey(wchar_t* arg);
void Unhotkey(wchar_t* arg);
void nope_command(wchar_t *arg);
void abort_timer_command(wchar_t*arg);
void autoreconnect_command(wchar_t*arg);

void clear_command(wchar_t *arg);
void wclear_command(wchar_t *arg);

void loopback_command(wchar_t *arg);
void broadcast_command(wchar_t *arg);

void srandom_command(wchar_t *arg);
void random_command(wchar_t *arg);

void bar_command(wchar_t *arg);

void sync_command(wchar_t *arg);

//Proxy support
void proxy_command(wchar_t *arg);
int proxy_connect(int, const struct sockaddr *, int);
int proxy_close(int);

//SSL/TLS support
void secure_command(wchar_t *arg);
int tls_open(SOCKET sock);
int tls_send(SOCKET sock, const char *buffer, int length);
int tls_recv(SOCKET sock, char *buffer, int maxlength);
int tls_close(SOCKET sock);


//Telnet routines
int get_telnet_option_num(const wchar_t *name);
void get_telnet_option_name(unsigned int num, wchar_t *buf);
void send_telnet_command(unsigned char command, unsigned char option = 0);
void send_telnet_subnegotiation(unsigned char option, const wchar_t *output, int length, bool raw_bytes);
void telnet_command(wchar_t *arg);
void promptend_command(wchar_t *arg);

//Out-of-band (GMCP/MSDP) support
typedef struct {
	set<wstring> submodules;
} oob_module_info;
extern map < wstring, oob_module_info > oob_modules;

void reset_oob();
void start_gmcp();
void start_msdp();
void start_mssp();
int parse_gmcp(const wchar_t *gmcp);
int parse_msdp(const wchar_t *msdp, int length);
int parse_mssp(const wchar_t *mssp, int length);
wstring convert_msdp2gmcp(const wchar_t *msdp, int length);
wstring convert_gmcp2msdp(const wchar_t *gmcp, int length);
wstring convert_mssp2gmcp(const wchar_t *mssp, int length);
int get_oob_variable(const wchar_t *varname, wchar_t *value, int maxlength);
void oob_command(wchar_t *arg);


// VARIABLES:
void variable_value_input(wchar_t *arg);
void variable_value_date(wchar_t *arg);
void variable_value_year(wchar_t *arg);
void variable_value_month(wchar_t *arg);
void variable_value_day(wchar_t *arg);
void variable_value_time(wchar_t *arg);
void variable_value_hour(wchar_t *arg);
void variable_value_minute(wchar_t *arg);
void variable_value_second(wchar_t *arg);
void variable_value_millisecond(wchar_t *arg);
void variable_value_timestamp(wchar_t *arg);
void variable_value_clockms(wchar_t *arg);
void variable_value_clock(wchar_t *arg);
void variable_value_color_default(wchar_t *arg);
void variable_value_random(wchar_t *arg);
void variable_value_hostname(wchar_t *arg);
void variable_value_hostip(wchar_t *arg);
void variable_value_hostport(wchar_t *arg);
void variable_value_eop(wchar_t *arg);
void variable_value_eol(wchar_t *arg);
void variable_value_esc(wchar_t *arg);
void variable_value_ping(wchar_t *arg);
void variable_value_ping_proxy(wchar_t *arg);
void variable_value_product_name(wchar_t *arg);
void variable_value_product_version(wchar_t *arg);

BOOL show_actions(wchar_t* left = NULL, CGROUP* pGroup = NULL);
int do_one_antisub(wchar_t *line);
extern unsigned int SocketFlags;
extern unsigned char State;
void free_telnet_buffer();
void telnet_push_back(const char *src, int size);
int telnet_more_coming();
int telnet_pop_front(wchar_t *dst, int maxsize);
void reset_telnet_protocol();
void do_telnet_protecol(const char* input, int length, int *used, char* output, int capacity, int *generated);
//vls-begin// multiple output
void StopLogging();
void log(wstring st);
void log(int wnd, wstring st);
wstring processLine(const wchar_t *strInput, DWORD TimeStamp = 0);
wstring processTEXT(wstring strInput);
wstring processRMA(wstring strInput, DWORD TimeStamp);
wstring processHTML(wstring strInput, DWORD TimeStamp);
//vls-end//
struct listnode *init_list(void);
struct listnode *init_pathdir_list(void);
void newactive_session();
void status_command(wchar_t* arg);
void ParseScriptlet2(BSTR bstrScriptlet);

//-----------------------------------------------
typedef map <wstring, ALIAS*> ALIASLIST ;
typedef ALIASLIST::iterator ALIAS_INDEX ;
extern ALIASLIST AliasList;

typedef map <wstring, CGROUP*> GROUPLIST ;
typedef GROUPLIST ::iterator GROUP_INDEX ;
extern GROUPLIST  GroupList;

namespace std
{
	template<>
    struct greater<ACTION*> : public binary_function<ACTION*, ACTION*, bool> {
    bool operator()(ACTION* x, ACTION* y) const{return x->m_nPriority < y->m_nPriority; };
    };
}


// typedef list<CActionPtr> ACTIONLIST;
typedef std::list<ACTION*> ACTIONLIST;
typedef ACTIONLIST::iterator ACTION_INDEX;
extern ACTIONLIST ActionList;

extern BOOL bDelayedActionDelete;

typedef map <wstring, VAR*> VARLIST;
typedef VARLIST::iterator VAR_INDEX ;
extern VARLIST  VarList;

typedef set<CPCRE*> PCRESET;
typedef map <wstring, PCRESET> VARTOPCRE;
extern VARTOPCRE VarPcreDeps;

typedef map <wstring, HLIGHT*> HLIGHTLIST ;
typedef HLIGHTLIST::iterator HLIGHT_INDEX ;
extern HLIGHTLIST HlightList;

typedef map <WORD, CHotKey*>::iterator HOTKEY_INDEX ;
extern map <WORD, CHotKey*> HotkeyList;

//vls-begin// script files
typedef list<CScriptFile *> SCRIPTFILELIST;
typedef SCRIPTFILELIST::iterator SCRIPTFILE_INDEX;
extern SCRIPTFILELIST ScriptFileList;   // nodes.cpp
//vls-end//

extern void* JMCObjRet[1000];

// --CHANGED by Anton Likhtarov for resource support
#include "ressup.h"
// --END

//* en:JMC functions struct. look cmds.h
const int JMC_CMDS_NUM=131;
typedef struct jmc_cmd 
	{
	wchar_t*alias;
	void (*jmcfn)(wchar_t*);
	wchar_t*hlpfile;
	} jmc_cmdi;
//*/en


const int JMC_SPECIAL_VARIABLES_NUM = 25;
typedef struct jmc_special_variable_struct {
	wchar_t *name;
	void (*jmcfn)(wchar_t*);
} jmc_special_variable;


//vls-begin// bugfix
//#include "cmds.h"
#endif
//vls-end//

