#include "stdafx.h"
#include "ttcoreex.h"
#include "tintin.h"
#include "JmcObj.h"
#include "cmds.h"

#include <time.h>

#include <windows.h>

//-------------------------
//* en
BOOL bDaaMessage = FALSE;
BOOL bColon = FALSE;
int iWaitState =0;
wchar_t mQueue[BUFFER_SIZE];
wchar_t cCommentChar = '#';

const int sTimersV = 16;
sTimer sTimers[sTimersV];

//* /en

extern DIRECT_OUT_FUNC DirectOutputFunction;
extern CLEAR_WINDOW_FUNC ClearWindowFunction;

extern SOCKET BCASTSocket;
extern WORD DLLEXPORT wBCastUdpPort;

extern wchar_t LoopBackBuffer[BUFFER_SIZE];
extern LoopBackCount;

GET_WNDSIZE_FUNC GetWindowSize = 0;
SET_WNDSIZE_FUNC SetWindowSize = 0;

void DLLEXPORT InitWindowSizeFunc(GET_WNDSIZE_FUNC GetWndFunc, SET_WNDSIZE_FUNC SetWndFunc)
{
    GetWindowSize = GetWndFunc;
	SetWindowSize = SetWndFunc;
}

/****************************/
/* the cr command           */
/****************************/
void cr_command(wchar_t*arg)
{
    write_line_mud(L"\n");
}

/****************************/
/* the verbatim command,    */
/* used as a toggle         */
/****************************/
void verbatim_command(wchar_t * arg)
{
    wchar_t flag[BUFFER_SIZE];

//* en
//* due to an error in tintin.h (messed up STOP_SPACES and WITH_SPACES)
//    arg = get_arg_in_braces(arg,flag,STOP_SPACES,sizeof(flag)/sizeof(wchar_t)-1);
    arg = get_arg_in_braces(arg,flag,WITH_SPACES,sizeof(flag)/sizeof(wchar_t)-1);
//* /en

    if ( *flag == 0 ) 
        verbatim=!verbatim;
    else {
        if ( !wcsicmp(flag, L"on" ) )
            verbatim = TRUE;
        else 
            verbatim = FALSE;
    }

    if (verbatim) 
        tintin_puts2(rs::rs(1095));
    else 
        tintin_puts2(rs::rs(1096)); 
}


/*********************/
/* the #bell command */
/*********************/
void bell_command(wchar_t*arg)
{
    MessageBeep(MB_OK);
}



/*********************/
/* the #char command */
/*********************/
void char_command(wchar_t *arg)
{
    wchar_t strng[80];
//* en//    get_arg_in_braces(arg,arg,WITH_SPACES,sizeof(arg)/sizeof(wchar_t)-1);
//vls-begin// bugfix
// now checks valid char again
///*  if(ispunct(*arg)); {
//    cCommandChar= *arg;
//    swprintf(strng, rs::rs(1098), cCommandChar);
//    tintin_puts2(strng);
//  }
//  else
//    tintin_puts2("#SPECIFY A PROPER COMMAND-CHAR! SOMETHING LIKE # OR /!");
//*/
//    cCommandChar= *arg;
//    swprintf(strng, rs::rs(1098), cCommandChar);
//    tintin_puts2(strng);
  if(ispunct(*arg)) {
      cCommandChar= *arg;
      swprintf(strng, rs::rs(1098), cCommandChar);
      tintin_puts2(strng);
  }
  else
      tintin_puts2(rs::rs(1245));
//vls-end//
}


/*********************/
/* the #echo command */
/*********************/
void echo_command(wchar_t* arg)
{
    wchar_t flag[BUFFER_SIZE];
//* en
//* due to an error in tintin.h (messed up STOP_SPACES and WITH_SPACES)
//    arg = get_arg_in_braces(arg,flag,STOP_SPACES,sizeof(flag)/sizeof(wchar_t)-1);
    arg = get_arg_in_braces(arg,flag,WITH_SPACES,sizeof(flag)/sizeof(wchar_t)-1);
//* /en

    if ( *flag == 0 ) 
        echo=!echo;
    else {
        if ( !wcsicmp(flag, L"on" ) )
            echo= TRUE;
        else 
            echo= FALSE;
    }
  
    if(echo)
        tintin_puts2(rs::rs(1099));
    else
        tintin_puts2(rs::rs(1100));
}


/***********************/
/* the #ignore command */
/***********************/
void ignore_command(wchar_t* arg)
{
    wchar_t flag[BUFFER_SIZE];
//* en
//* due to an error in tintin.h (messed up STOP_SPACES and WITH_SPACES)
//    arg = get_arg_in_braces(arg,flag,STOP_SPACES,sizeof(flag)/sizeof(wchar_t)-1);
    arg = get_arg_in_braces(arg,flag,WITH_SPACES,sizeof(flag)/sizeof(wchar_t)-1);
//* /en

    if ( *flag == 0 ) 
        ignore=!ignore;
    else {
        if ( !wcsicmp(flag, L"on" ) )
            ignore = TRUE;
        else 
            ignore = FALSE;
    }

    if(ignore)
      tintin_puts2(rs::rs(1101));
    else
      tintin_puts2(rs::rs(1102));
}

/**********************/
/* the #presub command*/
/**********************/
void presub_command(wchar_t* arg)
{
    wchar_t flag[BUFFER_SIZE];
//* en
//* due to an error in tintin.h (messed up STOP_SPACES and WITH_SPACES)
//    arg = get_arg_in_braces(arg,flag,STOP_SPACES,sizeof(flag)/sizeof(wchar_t)-1);
    arg = get_arg_in_braces(arg,flag,WITH_SPACES,sizeof(flag)/sizeof(wchar_t)-1);
//* /en

    if ( *flag == 0 ) 
        presub=!presub;
    else {
        if ( !wcsicmp(flag, L"on" ) )
            presub= TRUE;
        else 
            presub= FALSE;
    }
  
    if(presub)
        tintin_puts2(rs::rs(1103));
    else
        tintin_puts2(rs::rs(1104));
}

/**************************/
/* the #togglesubs command*/
/**************************/
void togglesubs_command(wchar_t* arg)
{
    wchar_t flag[BUFFER_SIZE];
//* en
//* due to an error in tintin.h (messed up STOP_SPACES and WITH_SPACES)
//    arg = get_arg_in_braces(arg,flag,STOP_SPACES,sizeof(flag)/sizeof(wchar_t)-1);
    arg = get_arg_in_braces(arg,flag,WITH_SPACES,sizeof(flag)/sizeof(wchar_t)-1);
//* /en

    if ( *flag == 0 ) 
        togglesubs=!togglesubs;
    else {
        if ( !wcsicmp(flag, L"on" ) )
            togglesubs = TRUE;
        else 
            togglesubs = FALSE;
    }
    
    if(togglesubs)
        tintin_puts2(rs::rs(1105));
    else 
        tintin_puts2(rs::rs(1106));
}

/***********************/
/* the #showme command */
/***********************/
void showme_command(wchar_t *arg)
{ 
    wchar_t left[BUFFER_SIZE], right[BUFFER_SIZE];
    wchar_t result[BUFFER_SIZE], strng[BUFFER_SIZE];
    
    arg=get_arg_in_braces(arg,left,WITH_SPACES,sizeof(left)/sizeof(wchar_t)-1);
    arg=get_arg_in_braces(arg,right,WITH_SPACES,sizeof(right)/sizeof(wchar_t)-1);

    if ( !right[0] ) {  // no colors
        prepare_actionalias(left,strng, sizeof(strng)/sizeof(wchar_t)); 
    } else {
        prepare_actionalias(right,result, sizeof(result)/sizeof(wchar_t)); 
        add_codes(result, strng, left);
    }

	if (bLogAsUserSeen) {
		log(processLine(strng));
		log(L"\n");
		add_line_to_scrollbuffer(strng);
	}
    tintin_puts2(strng);
}
/***********************/
/* the #loop command   */
/***********************/
void loop_command(wchar_t* arg)
{
  wchar_t //*pt,
 	   express[BUFFER_SIZE] = L"\0",
	   command[BUFFER_SIZE] = L"\0";
  int a1 = 0,
	  a2 = 0,
	  a3 = 0,
	  a4 = 0;

  arg=get_arg_in_braces(arg,express,STOP_SPACES,sizeof(express)/sizeof(wchar_t)-1);

  substitute_myvars(express, command, sizeof(command)/sizeof(wchar_t));
  wcscpy(express, command);

  arg=get_arg_in_braces(arg,command,WITH_SPACES,sizeof(command)/sizeof(wchar_t)-1);

  

  if(iswdigit(*express))
  {
	  swscanf(express,L"%d,%d|%d:%d",&a1,&a2,&a3,&a4);
	  swscanf(express,L"%d,%d:%d|%d",&a1,&a2,&a4,&a3);
	  swscanf(express,L"%d:%d|%d",&a1,&a4,&a3);
	  swscanf(express,L"%d|%d:%d",&a1,&a3,&a4);
  }
  else
	tintin_puts2(rs::rs(1107));


  do_cycle(a1, a2, (a3>0 && a3<=(1+(a1>a2)?(a1-a2):(a2-a1)))?a3:1, a4, command);
  //              step must be positive and no more than abs(a1-a2) 
  
}
/************************/
/* the #message command */
/************************/
void message_command(wchar_t *arg)
{
  int mestype;
//vls-begin// script files
//  char ms[8][20], tpstr[80];
//* en:logs
//  char ms[9][20], tpstr[80];
  wchar_t ms[MSG_MAXNUM][20], tpstr[80];
//*/en
//vls-end//
    wchar_t type[BUFFER_SIZE], flag[BUFFER_SIZE];

  arg = get_arg_in_braces(arg,type,STOP_SPACES,sizeof(type)/sizeof(wchar_t)-1);
  arg = get_arg_in_braces(arg,flag,STOP_SPACES,sizeof(flag)/sizeof(wchar_t)-1);

  memset(ms, 0, sizeof(ms));
  
  const wchar_t messages[] = L"aliases actions substitutes antisubstitutes highlights variables groups hotkeys uses logs telnet oob mapper";
  swscanf(messages, L"%ls %ls %ls %ls %ls %ls %ls %ls %ls %ls %ls %ls %ls",
	  ms[0],ms[1],ms[2],ms[3],ms[4],ms[5],ms[6], ms[7], ms[8], ms[9], ms[10], ms[11], ms[12]);
 
  mestype=0;
  while (!is_abrev(type,ms[mestype]) && mestype< MSG_MAXNUM ) 
    mestype++;
  if (mestype==MSG_MAXNUM)
    tintin_puts2(rs::rs(1108));
  else 
  {
      if ( !*flag ) 
            mesvar[mestype]=!mesvar[mestype];
      else 
          if ( !wcsicmp(flag , L"on" ) )
              mesvar[mestype]=1;
          else 
              mesvar[mestype]=0;
    wchar_t* t1 = new wchar_t[16];
    wcscpy(t1, mesvar[mestype] ? rs::rs(1125) : rs::rs(1126));
    
    swprintf(tpstr,rs::rs(1109),
        ms[mestype],t1);
    delete[]t1;
    tintin_puts2(tpstr);
  }
}

/**************************/
/* the #speedwalk command */
/**************************/
void speedwalk_command(wchar_t* arg)
{
//* en//    wchar_t flag[BUFFER_SIZE];
//* en
//* due to an error in tintin.h (messed up STOP_SPACES and WITH_SPACES)
//    arg = get_arg_in_braces(arg,flag,STOP_SPACES,sizeof(flag)/sizeof(wchar_t)-1);
//* en//    arg = get_arg_in_braces(arg,flag,WITH_SPACES,sizeof(flag)/sizeof(wchar_t)-1);
//* /en
//* en//    if ( *flag == 0 ) 
    if ( *arg == 0 ) 
        speedwalk=!speedwalk;
    else {
//* en//        if ( !wcsicmp(flag, "on" ) )
        if ( !wcsicmp(arg, L"on" ) )
            speedwalk = TRUE;
        else 
            speedwalk = FALSE;
    }
    
    if(speedwalk)
        tintin_puts2(rs::rs(1112));
    else
        tintin_puts2(rs::rs(1113));
}


/********************/
/* the #zap command */
/********************/
void zap_command(wchar_t *arg)
{
    if ( WaitForSingleObject(hConnThread , 0 ) == WAIT_TIMEOUT ) {
        TerminateThread(hConnThread, 0);
        CloseHandle(hConnThread);
        tintin_puts2(rs::rs(1114));
        MUDSocket = NULL;
		memset(&MUDAddress, 0, sizeof(MUDAddress));
        return;
    }
    if ( MUDSocket == NULL ) {
        tintin_puts2(rs::rs(1115));
        return;
    }
    
    cleanup_session();
  
    tintin_puts(rs::rs(1116));
    pJmcObj->Fire_Disconnected();
    // newactive_session();
    return ;
}


void display_info(wchar_t*arg)
{
    wchar_t buf[BUFFER_SIZE];

	tintin_puts2(rs::rs(1117));
    swprintf(buf,rs::rs(1118), ActionList.size());
    tintin_puts2(buf);
    swprintf(buf,rs::rs(1119), AliasList.size());
    tintin_puts2(buf);
    swprintf(buf,rs::rs(1120), count_list(common_subs));
    tintin_puts2(buf);
    swprintf(buf,rs::rs(1121), count_list(common_antisubs));
    tintin_puts2(buf);
    swprintf(buf,rs::rs(1122), VarList.size());
    tintin_puts2(buf);
    swprintf(buf,rs::rs(1123), HlightList.size());
    tintin_puts2(buf);
    swprintf(buf,rs::rs(1247), HotkeyList.size());
    tintin_puts2(buf);
    wchar_t* t1 = new wchar_t[16];
    wchar_t* t2 = new wchar_t[16];
    wcscpy(t1, echo ? rs::rs(1125) : rs::rs(1126));
    wcscpy(t2, speedwalk ? rs::rs(1125) : rs::rs(1126));
    swprintf(buf,rs::rs(1124), t1 , t2);
    tintin_puts2(buf);
    
    wchar_t* t3 = new wchar_t[16];
    wcscpy(t1, togglesubs ? rs::rs(1125) : rs::rs(1126));
    wcscpy(t2, ignore ? rs::rs(1125) : rs::rs(1126));
    wcscpy(t3, presub ? rs::rs(1125) : rs::rs(1126));
    swprintf(buf,rs::rs(1129), t1, t2, t3);
    tintin_puts2(buf);
    delete[]t1;
    delete[]t2;
    delete[]t3;
}

// format #status N {text} [{color}]
void status_command(wchar_t* arg)
{
    wchar_t left[BUFFER_SIZE], right[BUFFER_SIZE], color[BUFFER_SIZE];
    
    arg=get_arg_in_braces(arg,left,STOP_SPACES,sizeof(left)/sizeof(wchar_t)-1);
    arg=get_arg_in_braces(arg,right,WITH_SPACES,sizeof(right)/sizeof(wchar_t)-1);
    arg=get_arg_in_braces(arg,color,WITH_SPACES,sizeof(color)/sizeof(wchar_t)-1);

    if ( !*left || !iswdigit(*left) ) {
        tintin_puts2(rs::rs(1136));
        return;
    }

    int statNum = _wtoi(left);
    if ( statNum < 1 || statNum > 5 ) {
        tintin_puts2(rs::rs(1137));
        return;
    }

    wchar_t* dest = strInfo1;
    switch ( statNum ) {
    case 1:
        dest = strInfo1;
        break;
    case 2:
        dest = strInfo2;
        break;
    case 3:
        dest = strInfo3;
        break;
    case 4:
        dest = strInfo4;
        break;
    case 5:
        dest = strInfo5;
        break;
    default:
        break;
    };

    wchar_t buff[BUFFER_SIZE];
    substitute_myvars (right, buff, sizeof(buff)/sizeof(wchar_t));

    EnterCriticalSection(&secStatusSection);
    if ( *color ) {
        add_codes(buff, dest, color);
    } else 
        wcscpy(dest, buff);
    LeaveCriticalSection(&secStatusSection);
}

void tabadd_command(wchar_t* arg)
{
    wchar_t word[BUFFER_SIZE];
//* en
//* due to an error in tintin.h (messed up STOP_SPACES and WITH_SPACES)
//    get_arg_in_braces(arg,word,STOP_SPACES,sizeof(word)/sizeof(wchar_t)-1);
    get_arg_in_braces(arg,word,STOP_SPACES,sizeof(word)/sizeof(wchar_t)-1);
//* /en
    if ( !*word ) {
        tintin_puts2(rs::rs(1138));
        return;
    }

    HGLOBAL hg = GlobalAlloc(GHND, wcslen(word)+2);
    wchar_t* p = (wchar_t*)GlobalLock(hg);
    wcscpy(p, word);
    GlobalUnlock (hg);

    PostMessage(hwndMAIN, WM_USER+200, 0, (LPARAM)hg);
    wchar_t msg[BUFFER_SIZE];
    swprintf(msg,rs::rs(1139), word);
    tintin_puts2(msg);
}

void tabdel_command(wchar_t* arg)
{
    wchar_t word[BUFFER_SIZE];
    get_arg_in_braces(arg,word,STOP_SPACES,sizeof(word)/sizeof(wchar_t)-1);
    if ( !*word ) {
        tintin_puts2(rs::rs(1140));
        return;
    }

    HGLOBAL hg = GlobalAlloc(GHND, wcslen(word)+2);
    wchar_t* p = (wchar_t*)GlobalLock(hg);
    wcscpy(p, word);
    GlobalUnlock (hg);

    PostMessage(hwndMAIN, WM_USER+201, 0, (LPARAM)hg);
    wchar_t msg[BUFFER_SIZE];
    swprintf(msg,rs::rs(1141), word);
    tintin_puts2(msg);
}

LONG  DLLEXPORT GetCommandsList(wchar_t *List)
{
	int i, ret = 0;

	for (i = 0; i < JMC_CMDS_NUM; i++) {
		ret += 1 + wcslen(jmc_cmds[i].alias) + 1;
		if (List)
			List += swprintf(List, L"%lc%ls ", cCommandChar, jmc_cmds[i].alias);
	}

	return ret;
}

//vls-begin// #quit
void quit_command(wchar_t *arg) {
 PostMessage(hwndMAIN, WM_USER+400, 0, 0);
}
//vls-end//

void loopback_command(wchar_t *arg)
{
	wchar_t result[BUFFER_SIZE], strng[BUFFER_SIZE];
    
	arg=get_arg_in_braces(arg,strng,WITH_SPACES,sizeof(strng)/sizeof(wchar_t)-1);

	prepare_actionalias(strng,result, sizeof(result)/sizeof(wchar_t)); 
	
	//tintin_puts(result);
	int len = wcslen(result);
	if (LoopBackCount + len + 2 > sizeof(LoopBackBuffer)/sizeof(wchar_t))
		len = sizeof(LoopBackBuffer)/sizeof(wchar_t) - LoopBackCount - 2;
	memcpy(&LoopBackBuffer[LoopBackCount], result, len*sizeof(wchar_t));
	LoopBackCount += len;
	LoopBackBuffer[LoopBackCount++] = L'\n';
	LoopBackBuffer[LoopBackCount] = L'\0';
}

void broadcast_command(wchar_t *arg)
{
	wchar_t temp[BUFFER_SIZE], result[BUFFER_SIZE];

	arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp)/sizeof(wchar_t)-1);
	prepare_actionalias(temp, result, sizeof(result)/sizeof(wchar_t)); 

	if (is_abrev(result, L"disable")) {
		bBCastEnabled = FALSE;
		result[0] = L'\0';
	} else if (is_abrev(result, L"enable")) {
		bBCastEnabled = TRUE;
		result[0] = L'\0';
	} else if (is_abrev(result, L"filterip")) {
		arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp)/sizeof(wchar_t)-1);
		if (is_abrev(temp, L"on"))
			bBCastFilterIP = TRUE;
		else if (is_abrev(temp, L"off"))
			bBCastFilterIP = FALSE;
		result[0] = L'\0';
	} else if (is_abrev(result, L"filterport")) {
		arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp)/sizeof(wchar_t)-1);
		if (is_abrev(temp, L"on"))
			bBCastFilterPort = TRUE;
		else if (is_abrev(temp, L"off"))
			bBCastFilterPort = FALSE;
		result[0] = L'\0';
	} else if (is_abrev(result, L"port")) {
		arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp)/sizeof(wchar_t)-1);
		int port;
		if (swscanf(temp, L"%u", &port) > 0)
			wBCastUdpPort = port;
		result[0] = L'\0';
	} else if (is_abrev(result, L"send")) {
		if( BCASTSocket == INVALID_SOCKET ) {
			tintin_puts2(rs::rs(1263));
		} else {
			arg = get_arg_in_braces(arg, temp, WITH_SPACES, sizeof(temp)/sizeof(wchar_t)-1);
			prepare_actionalias(temp, result, sizeof(result)/sizeof(wchar_t)); 
			
			struct sockaddr_in local;

			local.sin_family = AF_INET;
			local.sin_addr.S_un.S_addr = htonl(INADDR_BROADCAST);
			local.sin_port = htons(wBCastUdpPort);

			int len = wcslen(result) * sizeof(wchar_t), sent;

			sent = sendto(BCASTSocket, (const char*)result, len, 0, (const sockaddr*)&local, sizeof(local));
			if(sent != len) {
				wchar_t msg[BUFFER_SIZE];
				swprintf(msg,rs::rs(1264), sent, len);
				tintin_puts(msg);
			}
		}
	}

	if (!result[0]) {
		wchar_t on_str[BUFFER_SIZE], off_str[BUFFER_SIZE];
		wcscpy(on_str, rs::rs(1125));
		wcscpy(off_str, rs::rs(1126));
		swprintf(temp, rs::rs(1302), 
			bBCastEnabled ? on_str : off_str,
			bBCastFilterIP ? on_str : off_str,
			bBCastFilterPort ? on_str : off_str,
			wBCastUdpPort);
		tintin_puts2(temp);
		reopen_bcast_socket();
	}
}

void srandom_command(wchar_t *arg)
{
    wchar_t seed_str[BUFFER_SIZE];
    
    arg = get_arg_in_braces(arg,seed_str,STOP_SPACES,sizeof(seed_str)/sizeof(wchar_t)-1);
	int seed;
	if (wcslen(seed_str) == 0) {
		seed = (int)time(NULL);
	} else {
		seed = _wtoi(seed_str);
	}

	srand(seed);

	wchar_t msg[BUFFER_SIZE];
	swprintf(msg,rs::rs(1265), seed);
	tintin_puts2(msg);
}

void random_command(wchar_t *arg)
{
	wchar_t var[BUFFER_SIZE], bound1[BUFFER_SIZE], bound2[BUFFER_SIZE], result[BUFFER_SIZE];
	
	arg = get_arg_in_braces(arg,var,STOP_SPACES,sizeof(var)/sizeof(wchar_t)-1);
	arg = get_arg_in_braces(arg,bound1,STOP_SPACES,sizeof(bound1)/sizeof(wchar_t)-1);
	arg = get_arg_in_braces(arg,bound2,STOP_SPACES,sizeof(bound2)/sizeof(wchar_t)-1);
  
	substitute_vars(bound1, result, sizeof(result)/sizeof(wchar_t));
	substitute_myvars(result, bound1, sizeof(bound1)/sizeof(wchar_t));

	substitute_vars(bound2, result, sizeof(result)/sizeof(wchar_t));
	substitute_myvars(result, bound2, sizeof(bound2)/sizeof(wchar_t));

	int minval = 0, maxval = 100;

	if (wcslen(bound1) > 0 && wcslen(bound2) > 0) {
		minval = _wtoi(bound1);
		maxval = _wtoi(bound2);
	} else if (wcslen(bound1) > 0) {
		maxval = _wtoi(bound1);
	}

	int period = maxval - minval;
	int val = minval;

	if (period > 1) {
		int rmax = RAND_MAX * RAND_MAX;
		int max = (rmax / period) * period;
		do {
			val = rand() + RAND_MAX * rand();
		} while (val >= max);
		val %= period;
		val += minval;
	}

	swprintf(result, L"%d", val);

    VAR_INDEX ind = VarList.find(var);
    VAR* pvar;
    if ( ind != VarList.end() ) {
        pvar = ind->second;
        pvar->m_strVal = result;
    }
    else {
        pvar = new VAR(result);
        VarList[var] = pvar;
    }
}

void bar_command(wchar_t *arg)
{
	wchar_t var[BUFFER_SIZE], length[BUFFER_SIZE], 
		fill_left[BUFFER_SIZE], fill_right[BUFFER_SIZE],
		value[BUFFER_SIZE], maximum[BUFFER_SIZE],
		temp[BUFFER_SIZE];
	
	arg = get_arg_in_braces(arg,temp,STOP_SPACES,sizeof(temp)/sizeof(wchar_t)-1);
	prepare_actionalias(temp, var,sizeof(var)/sizeof(wchar_t)-1);

	arg = get_arg_in_braces(arg,temp,STOP_SPACES,sizeof(temp)/sizeof(wchar_t)-1);
	prepare_actionalias(temp, length,sizeof(length)/sizeof(wchar_t)-1);

	arg = get_arg_in_braces(arg,temp,STOP_SPACES,sizeof(temp)/sizeof(wchar_t)-1);
	prepare_actionalias(temp, fill_left,sizeof(fill_left)/sizeof(wchar_t)-1);

	arg = get_arg_in_braces(arg,temp,STOP_SPACES,sizeof(temp)/sizeof(wchar_t)-1);
	prepare_actionalias(temp, fill_right,sizeof(fill_right)/sizeof(wchar_t)-1);

	arg = get_arg_in_braces(arg,temp,STOP_SPACES,sizeof(temp)/sizeof(wchar_t)-1);
	prepare_actionalias(temp, value,sizeof(value)/sizeof(wchar_t)-1);

	arg = get_arg_in_braces(arg,temp,STOP_SPACES,sizeof(temp)/sizeof(wchar_t)-1);
	prepare_actionalias(temp, maximum,sizeof(maximum)/sizeof(wchar_t)-1);

	if (wcslen(maximum) == 0 || 
		wcslen(fill_left) == 0 || 
		wcslen(fill_right) == 0) {
		tintin_puts2(rs::rs(1283));
		return;
	}

	if (!is_all_digits(length) ||
		!is_all_digits(value) ||
		!is_all_digits(maximum)) {
		tintin_puts2(rs::rs(1283));
		return;
	}

	int len = min(_wtoi(length), sizeof(value)/sizeof(wchar_t)-1);
	int val = _wtoi(value);
	int maxv = _wtoi(maximum);

	val = max(0, val);
	maxv = max(1, maxv);
	val = min(val, maxv);

	wstring bar = L"";
	/*
	 len(left)*cnt / length = value / maximum
	 cnt = value * length / (maximum * len(left))
	 */
	int cnt_left = (val * len) / (maxv * wcslen(fill_left));
	for (int i = 0; i < cnt_left; i++)
		bar += fill_left;
	while (bar.length() + wcslen(fill_right) <= len)
		bar += fill_right;

	wcscpy(value, bar.c_str());
	
	VAR_INDEX ind = VarList.find(var);
    VAR* pvar;
    if ( ind != VarList.end() ) {
        pvar = ind->second;
        pvar->m_strVal = value;
    }
    else {
        pvar = new VAR(value);
        VarList[var] = pvar;
    }
}

void sync_command(wchar_t *arg)
{
	ReadMud();
}

void hidewindow_command(wchar_t *arg)
{
    PostMessage(hwndMAIN, WM_USER+410, 0, 0);
}

void restorewindow_command(wchar_t *arg)
{
    PostMessage(hwndMAIN, WM_USER+420, 0, 0);
}

void systray_command(wchar_t *arg)
{
	wchar_t cmd[BUFFER_SIZE];
	get_arg_in_braces(arg,cmd,STOP_SPACES,sizeof(cmd)/sizeof(wchar_t)-1);
	
	if ( is_abrev(cmd, L"hide") ) {
		PostMessage(hwndMAIN, WM_USER+430, 0, 0);
	} else if ( is_abrev(cmd, L"show") ) {
		PostMessage(hwndMAIN, WM_USER+440, 0, 0);
	} else {
        tintin_puts2(rs::rs(1259));
        return;
	}
}

//vls-begin// #reloadscripts
void reloadscripts_command(wchar_t *arg)
{
    PostMessage(hwndMAIN, WM_USER+300, 0, 0);
}
//vls-end//

//vls-begin// #run
void run_command(wchar_t *arg)
{
    wchar_t cmd[BUFFER_SIZE];
    wchar_t params[BUFFER_SIZE];

    arg = get_arg_in_braces(arg,cmd,STOP_SPACES,sizeof(cmd)/sizeof(wchar_t)-1);
    arg = get_arg_in_braces(arg,params,WITH_SPACES,sizeof(params)/sizeof(wchar_t)-1);

    if ( !*cmd ) {
        tintin_puts2(rs::rs(1229));
        return;
    }
    ShellExecute(NULL, NULL, cmd, params[0] ? params : NULL, NULL, SW_SHOW);
}
//vls-end//

//vls-begin// #play
void play_command(wchar_t *arg)
{
//* en//    char wave[MAX_PATH+2];
//* en
//    get_arg_in_braces(arg,wave,STOP_SPACES,sizeof(wave)/sizeof(wchar_t)-1);
//* en//    get_arg_in_braces(arg,wave,WITH_SPACES,sizeof(wave)/sizeof(wchar_t)-1);
//* /en
//* en//    if ( !*wave ) {
    if ( !*arg ) {
        tintin_puts2(rs::rs(1230));
        return;
    }
    wchar_t fn[MAX_PATH+2];
//* en//    MakeAbsolutePath(fn, wave, szBASE_DIR);
    MakeAbsolutePath(fn, arg, szBASE_DIR);
    PlaySound(fn, NULL, SND_ASYNC | SND_FILENAME);
}
//vls-end//

//vls-begin// #flash
void flash_command(wchar_t *arg)
{
	wchar_t param[BUFFER_SIZE];

	arg = get_arg_in_braces(param, arg, STOP_SPACES, sizeof(param) / sizeof(wchar_t) - 1);

	if (is_abrev(param, L"nopopup")) {
		//FLASHWINFO finfo;
		FlashWindow(hwndMAIN, TRUE);
	} else {
		SetForegroundWindow(hwndMAIN);
	}
}
//vls-end//

//* en
void daa_command(wchar_t *arg)
{
	if(wcslen(arg)<1)
	{
        tintin_puts2(rs::rs(1246));
		return;
	}
    bDaaMessage = TRUE;
    write_line_mud(arg);
}

void colon_command(wchar_t *arg)
{
	wchar_t cmd[BUFFER_SIZE];
	get_arg_in_braces(arg,cmd,STOP_SPACES,sizeof(cmd)/sizeof(wchar_t)-1);
    if ( is_abrev(cmd, L"leave") )
        bColon = FALSE;
    else 
	if ( is_abrev(cmd, L"replace") )
        bColon = TRUE;
	else 
        bColon = !bColon;
    
}

int is_workable_timer(int id)
{
	return (sTimers[id].startV||sTimers[id].endV) 
		 && sTimers[id].capacity 
		 && sTimers[id].step 
		 && sTimers[id].command[0];
}

void do_cycle(int b1, int b2, int step, int delay, wchar_t *command)
{
    wchar_t result[BUFFER_SIZE];
    int flag, counter;
    flag=1;
    counter=b1;
	if(step<1)
		step = 1;
    if(delay==0)
	{//non-timer, just run cycle
	  while(flag==1) 
	  {
        swprintf(vars[0], L"%d", counter);
        substitute_vars(command,result, sizeof(result)/sizeof(wchar_t));
        parse_input(result, TRUE);
        if (b1<b2) {
          counter+=step;
          if (counter>b2)
            flag=0;
		}
        else {
           counter-=step;
           if (counter<b2)
             flag=0;
		}
	  }
	}
	else
	{// code for timers
      BOOL found = FALSE;
	  int fort = -1;
	  for(int i = 0; i<sTimersV;i++)
		  if(sTimers[i].isActive == FALSE && !is_workable_timer(i))
		  {
			  found = TRUE;
			  fort = i;
			  break;
		  }
      if(!found)
		  tintin_puts2(rs::rs(1250));
	  else
	  {
		sTimers[fort].isActive   =    TRUE;
  		sTimers[fort].startV     =      b1;
		sTimers[fort].endV       =      b2;
		sTimers[fort].step       =    step;
		sTimers[fort].delay      = delay-1;
		sTimers[fort].counter    =      b1;
		sTimers[fort].capacity   =   delay;
		wchar_t *ptr1=command;
		wchar_t *ptr2=sTimers[fort].command;
		for(;*ptr1&&*ptr1!=L'\0';)
			*ptr2++=*ptr1++;
		*ptr2++=L'\0';

	  }


	};

}

void wt_command(wchar_t *arg)
{
	if(is_abrev(arg,L"cancel"))
	{
		mQueue[0]=L'\0';
		iWaitState = 0;
		return;
	}

	if(iswdigit(*arg))
	  iWaitState=_wtoi(arg);
	if(!iWaitState && wcslen(mQueue))
	{
	  parse_input(mQueue);
	  mQueue[0]=L'\0';
	}

}

void comment_command(wchar_t *arg)
{
  if(ispunct(*arg)) {
      cCommentChar = *arg;
  }
  else
      tintin_puts2(rs::rs(1245));
}

void MultiSub_command(wchar_t* arg)
{
    wchar_t status[BUFFER_SIZE];
    arg=get_arg_in_braces(arg,status,STOP_SPACES,sizeof(status)/sizeof(wchar_t)-1);

    if (*status) 
	{
        if(!wcsicmp(L"on", status))
            bMultiSub = TRUE;
        else 
		{
          if(!wcsicmp(L"off", status))
            bMultiSub = FALSE;
          else 
            bMultiSub = !bMultiSub;
		}	
    } 
	else
	  bMultiSub = ! bMultiSub;
     
}

void clear_timer(int id)
{
	  sTimers[id].isActive   =FALSE;
	  sTimers[id].startV     =   0 ;
	  sTimers[id].endV       =   0 ;
	  sTimers[id].step       =   0 ;
	  sTimers[id].delay      =   0 ;
	  sTimers[id].counter    =   0 ;
	  sTimers[id].capacity   =   0 ;
	  sTimers[id].command[0] = L'\0';

}

void update_timers(int nTime)
{
	static int pTime = 0;
	wchar_t result[BUFFER_SIZE];
    if(pTime != nTime)
	{
	  for(int i=0; i<sTimersV;i++)
		if(sTimers[i].isActive)
        {

		  sTimers[i].delay++;
		  if(sTimers[i].delay>=sTimers[i].capacity)
		  {
            swprintf(vars[0], L"%d", sTimers[i].counter);
            substitute_vars(sTimers[i].command,result, sizeof(result)/sizeof(wchar_t));
			parse_input(result);
			sTimers[i].delay = 0;
            if (sTimers[i].startV<sTimers[i].endV) 
			{
              sTimers[i].counter+=sTimers[i].step;
              if(sTimers[i].counter>sTimers[i].endV)
                sTimers[i].isActive = FALSE;
		    }
            else 
			{
              sTimers[i].counter-=sTimers[i].step;
              if(sTimers[i].counter<sTimers[i].endV)
                sTimers[i].isActive = FALSE;
			}
			if(!(sTimers[i].isActive))
			{
				clear_timer(i);
			}
		  }
		}
      pTime = nTime;
	}
}


void break_timer_command(wchar_t *arg)
{

	wchar_t uid[BUFFER_SIZE];
	int tID = -1;
	arg = get_arg_in_braces(arg,uid,STOP_SPACES,sizeof(uid)/sizeof(wchar_t)-1);

	if(!wcsicmp(uid,L"all"))
	{
	    wchar_t bf[BUFFER_SIZE];
	    for(int i=0;i<sTimersV;i++)
		{
		    swprintf(bf,L"%d",i);
		    break_timer_command(bf);
		}
		return;
	}

	if( iswdigit(uid[0]))
		tID = _wtoi(uid);
	if( tID < 0 || tID >= sTimersV)
		return;
	if(sTimers[tID].isActive)
 	   sTimers[tID].isActive = FALSE;
	else
	{
		clear_timer(tID);
	}
}


void continue_timer_command(wchar_t *arg)
{

	wchar_t uid[BUFFER_SIZE];
	int tID = -1;

	arg = get_arg_in_braces(arg,uid,STOP_SPACES,sizeof(uid)/sizeof(wchar_t)-1);
	if(!wcsicmp(uid,L"all"))
	{
	    wchar_t bf[BUFFER_SIZE];
	    for(int i=0;i<sTimersV;i++)
		{
		    swprintf(bf,L"%d",i);
		    continue_timer_command(bf);
		}
		return;
	}

	if( iswdigit(uid[0]))
		tID = _wtoi(uid);
	if( tID < 0 || tID > sTimersV)
		return;

	if(!sTimers[tID].isActive && is_workable_timer(tID))
 	    sTimers[tID].isActive = TRUE;
}

void tm_list_command(wchar_t*arg)
{
	wchar_t bf[BUFFER_SIZE];
	tintin_puts2(rs::rs(1248));
	for(int i=0;i<sTimersV;i++)
	{
	  if(sTimers[i].isActive == FALSE)
		  continue;
		  swprintf(bf,rs::rs(1249),
			  i,sTimers[i].startV,sTimers[i].endV,sTimers[i].step,sTimers[i].counter,sTimers[i].capacity,
			  sTimers[i].command);
	  tintin_puts2(bf);
	}

}

void help_command(wchar_t* arg)
{
  wchar_t command[BUFFER_SIZE];
  wchar_t file[BUFFER_SIZE] = L"\0";
  wchar_t buffer[BUFFER_SIZE] = L"\0";
  bool found = FALSE;

    arg=get_arg_in_braces(arg,command,STOP_SPACES,sizeof(command)/sizeof(wchar_t)-1);

    for(int i = 0; i<JMC_CMDS_NUM; i++)
	{
		CharLower(command);	
	    if(command[0] == *jmc_cmds[i].alias && is_abrev(command, jmc_cmds[i].alias))
		{
			wcscpy(file, (*jmc_cmds[i].hlpfile)?jmc_cmds[i].hlpfile:jmc_cmds[i].alias);
		    found = TRUE;
			break;
		}
	}

    if(!found) 
	{
		wcscpy(buffer,L"  \0");
		for(int i=0;i<JMC_CMDS_NUM;i++)
		{
			if(!((i+1)%8))
				wcscat(buffer,L"\n  ");
			wcscat(buffer,jmc_cmds[i].alias);
			if((i+1) && ((i+1)<JMC_CMDS_NUM))
				wcscat(buffer,L", ");
		}
		tintin_puts2(rs::rs(1251));
		tintin_puts2(buffer);
	    return;
	}
	tintin_puts2(rs::rs(1252));
    swprintf(buffer,L"{help/%ls.jht} {%lcshowme {-\?- %%0}} {s}",file, cCommandChar);
    spit_command(buffer);
    return;
}

void clean_command(wchar_t *arg)
{
    PostMessage(hwndMAIN, WM_USER+600, 0, 0);
}

/***********************/
/* the #clear command  */
/***********************/
void clear_command(wchar_t *arg)
{
	ClearWindowFunction(0);
}
/***********************/
/* the #wclear command */
/***********************/
void wclear_command(wchar_t *arg)
{
	wchar_t number[BUFFER_SIZE];
    int wnd = MAX_OUTPUT;

    arg=get_arg_in_braces(arg,number,STOP_SPACES,sizeof(number)/sizeof(wchar_t)-1);
    
    if (!is_all_digits(number) || !swscanf(number, L"%d", &wnd) || wnd < 0 || wnd >= MAX_OUTPUT) {
        tintin_puts2(rs::rs(1261));
        return;
    }

	ClearWindowFunction(wnd + 1);
}

// these are commands for WM_COMMAND to winamp
// you can add other easily
#define max_wamp 20
struct wamp_cmd{wchar_t*id;WORD msg;} 
	   wamp_cmds[] = {
		{L"previous",   44},
		{L"next",       48},
		{L"play",       45},
		{L"pause",      46},
		{L"stop",       47},
		{L"fadeout",   147},
		{L"stopafter", 157},
		{L"rewind",    144},
		{L"forward",   148},
		{L"start",     154},
		{L"end",       158},
		{L"elapsed",    37},
		{L"remaining",  38},
		{L"ontop",      19},
		{L"doublesize",165},
		{L"volume+",    58},
		{L"volume-",    59},
		{L"repeat",     22},
		{L"shuffle",    23},
		{L"close",       1}
/*		{L"",    0}
		{L"",    0}
		{L"",    0}
		{L"",    0}
		{L"",    0}
*/
		};

void winamp_command(wchar_t*arg)
{
  wchar_t comm[BUFFER_SIZE] = L"\0";
  wchar_t mode[BUFFER_SIZE] = L"\0";

  DWORD mean = 0;

  HWND wamp = FindWindow(L"Winamp v1.x",NULL);

	if(!wamp)
	{
	    tintin_puts2(rs::rs(1253));
		//return;
	}

    arg=get_arg_in_braces(arg,comm,STOP_SPACES,sizeof(comm)/sizeof(wchar_t)-1);
    
	wchar_t bf[BUFFER_SIZE];
	int i;
	for(i=0;i<max_wamp;i++)
		if(is_abrev(comm, wamp_cmds[i].id))
		{
			SendMessage(wamp,WM_COMMAND,40000+wamp_cmds[i].msg,0);
			return;
		}

	wcscpy(bf,L"  \0");
	for(i=0;i<max_wamp;)
	{
		++i;
		if(!(i%8))
			wcscat(bf,L"\n  ");
		wcscat(bf,wamp_cmds[i-1].id);
		if(i<max_wamp)
			wcscat(bf,L", ");
	}
	tintin_puts2(rs::rs(1254));
	tintin_puts2(bf);
    return;
}


void nope_command(wchar_t*arg){return;};

void abort_timer_command(wchar_t*arg)
{
	break_timer_command(arg);
	break_timer_command(arg);
}

void autoreconnect_command(wchar_t*arg)
{
   wchar_t bf[BUFFER_SIZE];
   if(is_abrev(arg,L"on"))
	   bAutoReconnect = TRUE;
   else if(is_abrev(arg,L"off"))
	   bAutoReconnect = FALSE;
   else 
	   bAutoReconnect = !bAutoReconnect;
   swprintf(bf,rs::rs(1257),bAutoReconnect?L"ON":L"OFF");
   tintin_puts2(bf);

}

//* /en

