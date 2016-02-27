#include "stdafx.h"
#include "ttcoreex.h"
#include "tintin.h"
#include "JmcObj.h"
#include "cmds.h"

//-------------------------
//* en
BOOL bDaaMessage = FALSE;
BOOL bColon = FALSE;
int iWaitState =0;
char mQueue[BUFFER_SIZE];
//char *queue = mQueue;
char cCommentChar;

const int sTimersV = 16;
sTimer sTimers[sTimersV];

//* /en

extern DIRECT_OUT_FUNC DirectOutputFunction;
extern CLEAR_WINDOW_FUNC ClearWindowFunction;


/****************************/
/* the cr command           */
/****************************/
void cr_command(char*arg)
{
    write_line_mud("\n");
}

/****************************/
/* the verbatim command,    */
/* used as a toggle         */
/****************************/
void verbatim_command(char * arg)
{
    char flag[BUFFER_SIZE];

//* en
//* due to an error in tintin.h (messed up STOP_SPACES and WITH_SPACES)
//    arg = get_arg_in_braces(arg,flag,STOP_SPACES);
    arg = get_arg_in_braces(arg,flag,WITH_SPACES);
//* /en

    if ( *flag == 0 ) 
        verbatim=!verbatim;
    else {
        if ( !strcmpi(flag, "on" ) )
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
void bell_command(char*arg)
{
    MessageBeep(MB_OK);
}



/*********************/
/* the #char command */
/*********************/
void char_command(char *arg)
{
    char strng[80];
//* en//    get_arg_in_braces(arg, arg, WITH_SPACES);
//vls-begin// bugfix
// now checks valid char again
///*  if(ispunct(*arg)); {
//    cCommandChar= *arg;
//    sprintf(strng, rs::rs(1098), cCommandChar);
//    tintin_puts2(strng);
//  }
//  else
//    tintin_puts2("#SPECIFY A PROPER COMMAND-CHAR! SOMETHING LIKE # OR /!");
//*/
//    cCommandChar= *arg;
//    sprintf(strng, rs::rs(1098), cCommandChar);
//    tintin_puts2(strng);
  if(ispunct(*arg)) {
      cCommandChar= *arg;
      sprintf(strng, rs::rs(1098), cCommandChar);
      tintin_puts2(strng);
  }
  else
      tintin_puts2(rs::rs(1245));
//vls-end//
}


/*********************/
/* the #echo command */
/*********************/
void echo_command(char* arg)
{
    char flag[BUFFER_SIZE];
//* en
//* due to an error in tintin.h (messed up STOP_SPACES and WITH_SPACES)
//    arg = get_arg_in_braces(arg,flag,STOP_SPACES);
    arg = get_arg_in_braces(arg,flag,WITH_SPACES);
//* /en

    if ( *flag == 0 ) 
        echo=!echo;
    else {
        if ( !strcmpi(flag, "on" ) )
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
void ignore_command(char* arg)
{
    char flag[BUFFER_SIZE];
//* en
//* due to an error in tintin.h (messed up STOP_SPACES and WITH_SPACES)
//    arg = get_arg_in_braces(arg,flag,STOP_SPACES);
    arg = get_arg_in_braces(arg,flag,WITH_SPACES);
//* /en

    if ( *flag == 0 ) 
        ignore=!ignore;
    else {
        if ( !strcmpi(flag, "on" ) )
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
void presub_command(char* arg)
{
    char flag[BUFFER_SIZE];
//* en
//* due to an error in tintin.h (messed up STOP_SPACES and WITH_SPACES)
//    arg = get_arg_in_braces(arg,flag,STOP_SPACES);
    arg = get_arg_in_braces(arg,flag,WITH_SPACES);
//* /en

    if ( *flag == 0 ) 
        presub=!presub;
    else {
        if ( !strcmpi(flag, "on" ) )
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
void togglesubs_command(char* arg)
{
    char flag[BUFFER_SIZE];
//* en
//* due to an error in tintin.h (messed up STOP_SPACES and WITH_SPACES)
//    arg = get_arg_in_braces(arg,flag,STOP_SPACES);
    arg = get_arg_in_braces(arg,flag,WITH_SPACES);
//* /en

    if ( *flag == 0 ) 
        togglesubs=!togglesubs;
    else {
        if ( !strcmpi(flag, "on" ) )
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
void showme_command(char *arg)
{ 
    char left[BUFFER_SIZE], right[BUFFER_SIZE];
    char result[BUFFER_SIZE], strng[BUFFER_SIZE];
    
    arg=get_arg_in_braces(arg, left, WITH_SPACES);
    arg=get_arg_in_braces(arg, right, WITH_SPACES);

    if ( !right[0] ) {  // no colors
        prepare_actionalias(left,strng); 
    } else {
        prepare_actionalias(right,result); 
        add_codes(result, strng, left);
    }
    
	if (bLogAsUserSeen) {
		log(processLine(strng));
		log("\n");
	}
    tintin_puts2(strng);
}
/***********************/
/* the #loop command   */
/***********************/
void loop_command(char* arg)
{
  char //*pt,
 	   express[BUFFER_SIZE] = "\0",
	   command[BUFFER_SIZE] = "\0";
  int a1 = 0,
	  a2 = 0,
	  a3 = 0,
	  a4 = 0;

  arg=get_arg_in_braces(arg,express, STOP_SPACES);
  arg=get_arg_in_braces(arg,command, WITH_SPACES);

  if(isdigit(*express))
  {
	  sscanf(express,"%d,%d|%d:%d",&a1,&a2,&a3,&a4);
	  sscanf(express,"%d,%d:%d|%d",&a1,&a2,&a4,&a3);
	  sscanf(express,"%d:%d|%d",&a1,&a4,&a3);
	  sscanf(express,"%d|%d:%d",&a1,&a3,&a4);
  }
  else
	tintin_puts2(rs::rs(1107));


  do_cycle(a1, a2, (a3>0 && a3<=(1+(a1>a2)?(a1-a2):(a2-a1)))?a3:1, a4, command);
  //              step must be positive and no more than abs(a1-a2) 
  
}
/************************/
/* the #message command */
/************************/
void message_command(char *arg)
{
  int mestype;
//vls-begin// script files
//  char ms[8][20], tpstr[80];
//* en:logs
//  char ms[9][20], tpstr[80];
  char ms[10][20], tpstr[80];
//*/en
//vls-end//
    char type[BUFFER_SIZE], flag[BUFFER_SIZE];

  arg = get_arg_in_braces(arg,type, STOP_SPACES);
  arg = get_arg_in_braces(arg,flag, STOP_SPACES);

//vls-begin// script files
//  sscanf("aliases actions substitutes antisubstitutes highlights variables groups hotkeys",
//    "%s %s %s %s %s %s %s %s",ms[0],ms[1],ms[2],ms[3],ms[4],ms[5],ms[6], ms[7]);
//* en:logs
//  sscanf("aliases actions substitutes antisubstitutes highlights variables groups hotkeys uses",
//    "%s %s %s %s %s %s %s %s %s",ms[0],ms[1],ms[2],ms[3],ms[4],ms[5],ms[6], ms[7], ms[8]);
  sscanf("aliases actions substitutes antisubstitutes highlights variables groups hotkeys uses logs",
    "%s %s %s %s %s %s %s %s %s %s",ms[0],ms[1],ms[2],ms[3],ms[4],ms[5],ms[6], ms[7], ms[8], ms[9]);
//*/en
//vls-end//
  
  
  mestype=0;
  while (!is_abrev(type,ms[mestype]) && mestype< sizeof(mesvar)/sizeof(int) ) 
    mestype++;
  if (mestype==sizeof(mesvar)/sizeof(int))
    tintin_puts2(rs::rs(1108));
  else 
  {
      if ( !*flag ) 
            mesvar[mestype]=!mesvar[mestype];
      else 
          if ( !strcmpi(flag , "on" ) )
              mesvar[mestype]=1;
          else 
              mesvar[mestype]=0;
    char* t1 = new char[16];
    strcpy(t1, mesvar[mestype] ? rs::rs(1125) : rs::rs(1126));
    
    sprintf(tpstr,rs::rs(1109),
        ms[mestype],t1);
    delete[]t1;
    tintin_puts2(tpstr);
  }
}

/**************************/
/* the #speedwalk command */
/**************************/
void speedwalk_command(char* arg)
{
//* en//    char flag[BUFFER_SIZE];
//* en
//* due to an error in tintin.h (messed up STOP_SPACES and WITH_SPACES)
//    arg = get_arg_in_braces(arg,flag,STOP_SPACES);
//* en//    arg = get_arg_in_braces(arg,flag,WITH_SPACES);
//* /en
//* en//    if ( *flag == 0 ) 
    if ( *arg == 0 ) 
        speedwalk=!speedwalk;
    else {
//* en//        if ( !strcmpi(flag, "on" ) )
        if ( !strcmpi(arg, "on" ) )
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
void zap_command(char *arg)
{
    if ( WaitForSingleObject(hConnThread , 0 ) == WAIT_TIMEOUT ) {
        TerminateThread(hConnThread, 0);
        CloseHandle(hConnThread);
        tintin_puts2(rs::rs(1114));
        MUDSocket = NULL;
        return;
    }
    if ( MUDSocket == NULL ) {
        tintin_puts2(rs::rs(1115));
        return;
    }
    
    cleanup_session();
  
    tintin_puts2(rs::rs(1116));
    pJmcObj->Fire_Disconnected();
    // newactive_session();
    return ;
}


void display_info(char*arg)
{
    char buf[BUFFER_SIZE];

	tintin_puts2(rs::rs(1117));
    sprintf(buf,rs::rs(1118), ActionList.size());
    tintin_puts2(buf);
    sprintf(buf,rs::rs(1119), AliasList.size());
    tintin_puts2(buf);
    sprintf(buf,rs::rs(1120), count_list(common_subs));
    tintin_puts2(buf);
    sprintf(buf,rs::rs(1121), count_list(common_antisubs));
    tintin_puts2(buf);
    sprintf(buf,rs::rs(1122), VarList.size());
    tintin_puts2(buf);
    sprintf(buf,rs::rs(1123), HlightList.size());
    tintin_puts2(buf);
    sprintf(buf,rs::rs(1247), HotkeyList.size());
    tintin_puts2(buf);
    char* t1 = new char[16];
    char* t2 = new char[16];
    strcpy(t1, echo ? rs::rs(1125) : rs::rs(1126));
    strcpy(t2, speedwalk ? rs::rs(1125) : rs::rs(1126));
    sprintf(buf,rs::rs(1124), t1 , t2);
    tintin_puts2(buf);
    
    char* t3 = new char[16];
    strcpy(t1, togglesubs ? rs::rs(1125) : rs::rs(1126));
    strcpy(t2, ignore ? rs::rs(1125) : rs::rs(1126));
    strcpy(t3, presub ? rs::rs(1125) : rs::rs(1126));
    sprintf(buf,rs::rs(1129), t1, t2, t3);
    tintin_puts2(buf);
    delete[]t1;
    delete[]t2;
    delete[]t3;
}

// format #status N {text} [{color}]
void status_command(char* arg)
{
    char left[BUFFER_SIZE], right[BUFFER_SIZE], color[BUFFER_SIZE];
    
    arg=get_arg_in_braces(arg, left,  STOP_SPACES);
    arg=get_arg_in_braces(arg, right, WITH_SPACES);
    arg=get_arg_in_braces(arg, color, WITH_SPACES);

    if ( !*left || !isdigit(*left) ) {
        tintin_puts2(rs::rs(1136));
        return;
    }

    int statNum = atoi(left);
    if ( statNum < 1 || statNum > 5 ) {
        tintin_puts2(rs::rs(1137));
        return;
    }

    char* dest = strInfo1;
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

    char buff[BUFFER_SIZE];
    substitute_myvars (right, buff);

    EnterCriticalSection(&secStatusSection);
    if ( *color ) {
        add_codes(buff, dest, color, FALSE);
        strcat(dest, buff);
    } else 
        strcpy(dest, buff);
    LeaveCriticalSection(&secStatusSection);
}

void tabadd_command(char* arg)
{
    char word[BUFFER_SIZE];
//* en
//* due to an error in tintin.h (messed up STOP_SPACES and WITH_SPACES)
//    get_arg_in_braces(arg, word, STOP_SPACES);
    get_arg_in_braces(arg, word, STOP_SPACES);
//* /en
    if ( !*word ) {
        tintin_puts2(rs::rs(1138));
        return;
    }

    HGLOBAL hg = GlobalAlloc(GHND, strlen(word)+2);
    char* p = (char*)GlobalLock(hg);
    strcpy(p, word);
    GlobalUnlock (hg);

    PostMessage(hwndMAIN, WM_USER+200, 0, (LPARAM)hg);
    char msg[BUFFER_SIZE];
    sprintf(msg,rs::rs(1139), word);
    tintin_puts(msg);

}

void tabdel_command(char* arg)
{
    char word[BUFFER_SIZE];
    get_arg_in_braces(arg, word, STOP_SPACES);
    if ( !*word ) {
        tintin_puts2(rs::rs(1140));
        return;
    }

    HGLOBAL hg = GlobalAlloc(GHND, strlen(word)+2);
    char* p = (char*)GlobalLock(hg);
    strcpy(p, word);
    GlobalUnlock (hg);

    PostMessage(hwndMAIN, WM_USER+201, 0, (LPARAM)hg);
    char msg[BUFFER_SIZE];
    sprintf(msg,rs::rs(1141), word);
    tintin_puts(msg);
}

//vls-begin// #quit
void quit_command(char *arg)
{
    PostMessage(hwndMAIN, WM_USER+400, 0, 0);
}
//vls-end//

void hidewindow_command(char *arg)
{
    PostMessage(hwndMAIN, WM_USER+410, 0, 0);
}

void restorewindow_command(char *arg)
{
    PostMessage(hwndMAIN, WM_USER+420, 0, 0);
}

void systray_command(char *arg)
{
	char cmd[BUFFER_SIZE];
	get_arg_in_braces(arg, cmd, STOP_SPACES);
	
	if ( cmd[0] == 'h' && is_abrev(cmd, "hide") ) {
		PostMessage(hwndMAIN, WM_USER+430, 0, 0);
	} else if ( cmd[0] == 's' && is_abrev(cmd, "show") ) {
		PostMessage(hwndMAIN, WM_USER+440, 0, 0);
	} else {
        tintin_puts2(rs::rs(1259));
        return;
	}
}

//vls-begin// #reloadscripts
void reloadscripts_command(char *arg)
{
    PostMessage(hwndMAIN, WM_USER+300, 0, 0);
}
//vls-end//

//vls-begin// #run
void run_command(char *arg)
{
    char cmd[BUFFER_SIZE];
    char params[BUFFER_SIZE];

    arg = get_arg_in_braces(arg, cmd,    STOP_SPACES);
    arg = get_arg_in_braces(arg, params, WITH_SPACES);

    if ( !*cmd ) {
        tintin_puts2(rs::rs(1229));
        return;
    }
    ShellExecute(NULL, NULL, cmd, params[0] ? params : NULL, NULL, SW_SHOW);
}
//vls-end//

//vls-begin// #play
void play_command(char *arg)
{
//* en//    char wave[MAX_PATH+2];
//* en
//    get_arg_in_braces(arg, wave, STOP_SPACES);
//* en//    get_arg_in_braces(arg, wave, WITH_SPACES);
//* /en
//* en//    if ( !*wave ) {
    if ( !*arg ) {
        tintin_puts2(rs::rs(1230));
        return;
    }
    char fn[MAX_PATH+2];
//* en//    MakeAbsolutePath(fn, wave, szBASE_DIR);
    MakeAbsolutePath(fn, arg, szBASE_DIR);
    PlaySound(fn, NULL, SND_ASYNC | SND_FILENAME);
}
//vls-end//

//vls-begin// #flash
void flash_command(char *arg)
{
    SetForegroundWindow(hwndMAIN);
}
//vls-end//

//* en
void daa_command(char *arg)
{
	if(strlen(arg)<1)
	{
        tintin_puts2(rs::rs(1246));
		return;
	}
    bDaaMessage = TRUE;
    write_line_mud(arg);
}

void colon_command(char *arg)
{
	char cmd[BUFFER_SIZE];
	get_arg_in_braces(arg,cmd,STOP_SPACES);
    if ( cmd[0] == 'l' && is_abrev(cmd, "leave") )
        bColon = FALSE;
    else 
	if ( cmd[0] == 'r' && is_abrev(cmd, "replace") )
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

void do_cycle(int b1, int b2, int step, int delay, char *command)
{
    char result[BUFFER_SIZE];
    int flag, counter;
    flag=1;
    counter=b1;
	if(step<1)
		step = 1;
    if(delay==0)
	{//non-timer, just run cycle
	  while(flag==1) 
	  {
        sprintf(vars[0], "%d", counter);
        substitute_vars(command,result);
        parse_input(result);
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
		char *ptr1=command;
		char *ptr2=sTimers[fort].command;
		for(;*ptr1&&*ptr1!='\0';)
			*ptr2++=*ptr1++;
		*ptr2++='\0';

	  }


	};

}

void wt_command(char *arg)
{
//    char 
//      params[BUFFER_SIZE];

//    arg = get_arg_in_braces(arg, params, STOP_SPACES);
	if(is_abrev(arg,"cancel"))
	{
		mQueue[0]='\0';
		iWaitState = 0;
		return;
	}

	if(isdigit(*arg))
	  iWaitState=atoi(arg);
	if(!iWaitState && strlen(mQueue))
	{
	  parse_input(mQueue);
	  mQueue[0]='\0';
	}

}

void comment_command(char *arg)
{
  if(ispunct(*arg)) {
      cCommentChar = *arg;
  }
  else
      tintin_puts2(rs::rs(1245));
}

void MultiSub_command(char* arg)
{
    char status[BUFFER_SIZE];
    arg=get_arg_in_braces(arg, status, STOP_SPACES);

    if (*status) 
	{
        if(!strcmpi("on", status))
            bMultiSub = TRUE;
        else 
		{
          if(!strcmpi("off", status))
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
	  sTimers[id].command[0] = '\0';

}

void update_timers(int nTime)
{
	static int pTime = 0;
	char result[BUFFER_SIZE];
    if(pTime != nTime)
	{
	  for(int i=0; i<sTimersV;i++)
		if(sTimers[i].isActive)
        {

		  sTimers[i].delay++;
		  if(sTimers[i].delay>=sTimers[i].capacity)
		  {
            sprintf(vars[0], "%d", sTimers[i].counter);
            substitute_vars(sTimers[i].command,result);
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


void break_timer_command(char *arg)
{

	char uid[BUFFER_SIZE];
	int tID = -1;
	arg = get_arg_in_braces(arg, uid, STOP_SPACES);

	if(!strcmpi(uid,"all"))
	{
	    char bf[BUFFER_SIZE];
	    for(int i=0;i<sTimersV;i++)
		{
		    sprintf(bf,"%d",i);
		    break_timer_command(bf);
		}
		return;
	}

	if( isdigit(uid[0]))
		tID = atoi(uid);
	if( tID < 0 || tID > sTimersV)
		return;
	if(sTimers[tID].isActive)
 	   sTimers[tID].isActive = FALSE;
	else
	{
		clear_timer(tID);
	}
}


void continue_timer_command(char *arg)
{

	char uid[BUFFER_SIZE];
	int tID = -1;

	arg = get_arg_in_braces(arg, uid, STOP_SPACES);
	if(!strcmpi(uid,"all"))
	{
	    char bf[BUFFER_SIZE];
	    for(int i=0;i<sTimersV;i++)
		{
		    sprintf(bf,"%d",i);
		    continue_timer_command(bf);
		}
		return;
	}

	if( isdigit(uid[0]))
		tID = atoi(uid);
	if( tID < 0 || tID > sTimersV)
		return;

	if(!sTimers[tID].isActive && is_workable_timer(tID))
 	    sTimers[tID].isActive = TRUE;
}

void tm_list_command(char*arg)
{
	char bf[BUFFER_SIZE];
	tintin_puts2(rs::rs(1248));
	for(int i=0;i<sTimersV;i++)
	{
	  if(sTimers[i].isActive == FALSE)
		  continue;
		  sprintf(bf,rs::rs(1249),
			  i,sTimers[i].startV,sTimers[i].endV,sTimers[i].step,sTimers[i].counter,sTimers[i].capacity,
			  sTimers[i].command);
	  tintin_puts2(bf);
	}

}

void help_command(char* arg)
{
  char command[BUFFER_SIZE];
  char file[BUFFER_SIZE] = "\0";
  char buffer[BUFFER_SIZE] = "\0";
  bool found = FALSE;

    arg=get_arg_in_braces(arg,command,STOP_SPACES);

    for(int i = 0; i<JMC_CMDS_NUM; i++)
	{
		CharLower(command);	
	    if(command[0] == *jmc_cmds[i].alias && is_abrev(command, jmc_cmds[i].alias))
		{
			strcpy(file, (*jmc_cmds[i].hlpfile)?jmc_cmds[i].hlpfile:jmc_cmds[i].alias);
		    found = TRUE;
			break;
		}
	}

    if(!found) 
	{
		strcpy(buffer,"  \0");
		for(i=0;i<JMC_CMDS_NUM;i++)
		{
			if(!((i+1)%8))
				strcat(buffer,"\n  ");
			strcat(buffer,jmc_cmds[i].alias);
			if((i+1) && ((i+1)<JMC_CMDS_NUM))
				strcat(buffer,", ");
		}
		tintin_puts2(rs::rs(1251));
		tintin_puts2(buffer);
	    return;
	}
	tintin_puts2(rs::rs(1252));
    sprintf(buffer,"{help/%s.jht} {#showme {-\?- %%0}} {s}",file);
    spit_command(buffer);
    return;
}

void clean_command(char *arg)
{
    PostMessage(hwndMAIN, WM_USER+600, 0, 0);
}

/***********************/
/* the #clear command  */
/***********************/
void clear_command(char *arg)
{
	ClearWindowFunction(0);
}
/***********************/
/* the #wclear command */
/***********************/
void wclear_command(char *arg)
{
	char number[BUFFER_SIZE];
    int wnd = MAX_OUTPUT;
    int i, ok;

    arg=get_arg_in_braces(arg, number, STOP_SPACES);
    
    // checking first parameter to be all digits
    ok = 1;
    for (i = 0; number[i]; i++) {
        if (number[i] < '0' || number[i] > '9') {
            ok = 0;
            break;
        }
    }

    if (!ok || !sscanf(number, "%d", &wnd) || wnd < 0 || wnd >= MAX_OUTPUT) {
        tintin_puts(rs::rs(1240));
        return;
    }

	ClearWindowFunction(wnd + 1);
}

// these are commands for WM_COMMAND to winamp
// you can add other easily
#define max_wamp 20
struct wamp_cmd{char*id;WORD msg;} 
	   wamp_cmds[] = {
		{"previous",   44},
		{"next",       48},
		{"play",       45},
		{"pause",      46},
		{"stop",       47},
		{"fadeout",   147},
		{"stopafter", 157},
		{"rewind",    144},
		{"forward",   148},
		{"start",     154},
		{"end",       158},
		{"elapsed",    37},
		{"remaining",  38},
		{"ontop",      19},
		{"doublesize",165},
		{"volume+",    58},
		{"volume-",    59},
		{"repeat",     22},
		{"shuffle",    23},
		{"close",       1}
/*		{"",    0}
		{"",    0}
		{"",    0}
		{"",    0}
		{"",    0}
*/
		};

void winamp_command(char*arg)
{
  char comm[BUFFER_SIZE] = "\0";
  char mode[BUFFER_SIZE] = "\0";

  DWORD mean = 0;

  HWND wamp = FindWindow("Winamp v1.x",NULL);

	if(!wamp)
	{
	    tintin_puts2(rs::rs(1253));
		//return;
	}

    arg=get_arg_in_braces(arg, comm, STOP_SPACES);
    
	char bf[BUFFER_SIZE];

	for(int i=0;i<max_wamp;i++)
		if(is_abrev(comm, wamp_cmds[i].id))
		{
			SendMessage(wamp,WM_COMMAND,40000+wamp_cmds[i].msg,0);
			return;
		}

	strcpy(bf,"  \0");
	for(i=0;i<max_wamp;)
	{
		++i;
		if(!(i%8))
			strcat(bf,"\n  ");
		strcat(bf,wamp_cmds[i-1].id);
		if(i<max_wamp)
			strcat(bf,", ");
	}
	tintin_puts2(rs::rs(1254));
	tintin_puts2(bf);
    return;
}


void nope_command(char*arg){return;};

void abort_timer_command(char*arg)
{
	break_timer_command(arg);
	break_timer_command(arg);
}

void autoreconnect_command(char*arg)
{
   char bf[BUFFER_SIZE];
   if(is_abrev(arg,"on"))
	   bAutoReconnect = TRUE;
   else if(is_abrev(arg,"off"))
	   bAutoReconnect = FALSE;
   else 
	   bAutoReconnect = !bAutoReconnect;
   sprintf(bf,rs::rs(1257),bAutoReconnect?"ON":"OFF");
   tintin_puts2(bf);

}

//* /en

