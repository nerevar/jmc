#include "stdafx.h"
#include "tintin.h"
#include "files.h"
#include <time.h>
#include <io.h>

//vls-begin// base dir
char DLLEXPORT szBASE_DIR[MAX_PATH];
char DLLEXPORT szSETTINGS_DIR[MAX_PATH];
//vls-begin// #logadd + #logpass
BOOL bLogPassedLine = FALSE;
//vls-end//

struct completenode *complete_head;
void prepare_for_write(char *command, char *left, char *right, char *pr, char* group, char *result);

static BOOL bCurLogHTML = FALSE;
//vls-begin// multiple output
static BOOL bOutputCurLogHTML[MAX_OUTPUT];
GET_OUTPUTNAME_FUNC GetOutputName;
//vls-end//

//* en
BOOL bSosExact = FALSE;
//*/en

static const char* prolog = "<html>"
						"<head>"
							"<LINK REL=\"stylesheet\" TYPE=\"text/css\" HREF=\"styles.css\">"
						"</head>"
						"<body><pre>\r\n";

static const char* epilog = "</pre></body></html>";

static const char* bspan = "<a>";
static const char* espan = "</a>";

// const char* brk = "<br>\r\n";
static const char* brk = "\r\n";
static char cache[256];

static const char* dtable[] = 
{
	"b",	
	"dr",	
	"dg",
	"dy",	
	"db",	
	"dm",	
	"dc",	
	"dw",
};

static const char* ltable[] = 
{
	"b",
	"lr",
	"lg",
	"ly",
	"lb",
	"lm",
	"lc",
	"lw"
};


static void parse(const char* from, const char* to, int& attrib, int& tcolor, int& bcolor)
{
	char* p = new char[to-from+1];

	memcpy(p, from, to-from);
	p[to-from] = '\0';

	char * ptr = strtok(p, "p:;");
	while( ptr != NULL )
	{
		int val = atoi(ptr);
		switch( val )
		{
		case 0:
		case 1:
			attrib = val;
			break;

		case 30:
		case 31:
		case 32:
		case 33:
		case 34:
		case 35:
		case 36:
		case 37:
			tcolor = val;
			break;

		case 40:
		case 41:
		case 42:
		case 43:
		case 44:
		case 45:
		case 46:
		case 47:
			bcolor = val;
			break;
		}

		ptr = strtok(NULL, "p:;");
	}

	delete [] p;
}

static const char* span(int tcolor, int bcolor, int attrib)
{
	static char buf[64];

	switch( attrib )
	{
	case 0:
		sprintf(buf, "<a class=%s>", dtable[ tcolor-30 ]);
		break;

	case 1:
		sprintf(buf, "<a class=%s>", ltable[ tcolor-30 ]);
		break;

	default:
		sprintf(buf, "<a class=%s>", dtable[ tcolor-30 ]);
		break;
	}

	return buf;
}


static int attrib ;
static int tcolor ;
static int bcolor ;

static int new_attrib = attrib;
static int new_tcolor = tcolor;
static int new_bcolor = bcolor;

  
/********************/
/* the #log command */
/********************/
//vls-begin// multiple output
BOOL StartLog(int wnd, char* left, char *right)
{
//vls-begin// multiple output
    HANDLE *hLogFile = wnd < 0 ? &::hLogFile : &hOutputLogFile[wnd];
    BOOL *bCurLogHTML = wnd < 0 ? &::bCurLogHTML : &bOutputCurLogHTML[wnd];
//* en	
	char *logName = wnd<0 ?sLogName : sOutputLogName[wnd];
//*/en	

// all moved from log_command() but:
// 1) hLogFile changed to *hLogFile
// 2) bCurLogHTML changed to *bCurLogHTML
	SYSTEMTIME stl;
    char Timerecord[BUFFER_SIZE];
    BOOL bLogMode = bDefaultLogMode;

//* en	
	if(!strcmpi(left,logName) && strcmpi(right, "overwrite") && strcmpi(right, "html") )
		return FALSE;
//*/en	

    if ( *hLogFile) { // Close log file now opened 
        if ( *bCurLogHTML ) {
            DWORD Written;
            WriteFile(*hLogFile , epilog, strlen(epilog), &Written, NULL);
        }
        CloseHandle(*hLogFile);
        *hLogFile = NULL;
    	strcpy(logName,left);
        if(mesvar[MSG_LOG])
			tintin_puts2(rs::rs(1024));
        if ( !*left) 
            return FALSE;
    }

    if ( !*left) {
        tintin_puts2(rs::rs(1025));
        return FALSE;
    }

    *bCurLogHTML = bHTML;
    if ( *right ) {
        if ( !strcmpi(right, "append") ) { // try to open in append mode 
            bLogMode = TRUE;
            if ( *bCurLogHTML ) 
                tintin_puts2(rs::rs(1026));
        }
        else 
            if ( !strcmpi(right, "overwrite") ) { // try to open in overwrite mode 
                bLogMode = FALSE;
            }
            else {
                if ( !strcmpi(right, "html") ) 
                    *bCurLogHTML = TRUE;
                else {
                    tintin_puts2(rs::rs(1027));
                    return FALSE;
                }
            }
    }


    if ( bLogMode && !*bCurLogHTML ) {
        *hLogFile = CreateFile(left , GENERIC_READ| GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, NULL, NULL );
    }
    else 
        *hLogFile = CreateFile(left , GENERIC_READ| GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, NULL, NULL );

    if ( *hLogFile == INVALID_HANDLE_VALUE ) {
        char buff[128];
        *hLogFile = NULL;
        sprintf(buff,rs::rs(1028),left);
        tintin_puts2(buff);
        return FALSE;
    }
    SetFilePointer(*hLogFile, 0, 0 , FILE_END );

    // Do HTML Log pereference
    if ( *bCurLogHTML ) {
        DWORD Written;
        WriteFile(*hLogFile , prolog , strlen(prolog), &Written, NULL);

	    attrib = 0;
	    tcolor = 37;
	    bcolor = 40;


        const char* pspan = span(tcolor, bcolor, attrib);
        WriteFile(*hLogFile , pspan, strlen(pspan), &Written, NULL);
        strcpy(cache, "");
    }

	GetLocalTime(&stl);
//    sprintf(Timerecord, rs::rs(1029) , stl.wDay, stl.wMonth , 
//        stl.wYear , stl.wHour, stl.wMinute);
//    WriteToLog(Timerecord , strlen(Timerecord) );
    if (wnd < 0)
        sprintf(Timerecord, rs::rs(1029) , stl.wDay, stl.wMonth , stl.wYear , stl.wHour, stl.wMinute);
    else
        sprintf(Timerecord, rs::rs(1242) , wnd, stl.wDay, stl.wMonth , stl.wYear , stl.wHour, stl.wMinute);
    WriteToLog(wnd, Timerecord , strlen(Timerecord) );
	strcpy(logName,left);
    return TRUE;
}
//vls-end//

void log_command(char *arg)
{
//vls-begin// multiple output
//    code moved to StartLog().
    char filename[BUFFER_SIZE], params[BUFFER_SIZE];

    arg=get_arg_in_braces(arg, filename, STOP_SPACES);
    arg=get_arg_in_braces(arg, params,   STOP_SPACES);

    if ( StartLog(-1, filename, params) && mesvar[MSG_LOG]) {
        char msg[BUFFER_SIZE];
        sprintf(msg, rs::rs(1030), filename);
        tintin_puts2(msg);
    }
//vls-end//
}

//vls-begin// #logadd + #logpass
void logadd_command(char *arg)
{
    char msg[BUFFER_SIZE];
	char tmp[BUFFER_SIZE];

	get_arg_in_braces(arg, arg, WITH_SPACES);

    substitute_vars(arg,tmp);
    substitute_myvars(tmp,msg);
    strcat(msg, "\n");

    WriteToLog(-1, msg, strlen(msg));
}

void logpass_command(char *arg)
{
    bLogPassedLine = TRUE;
}
//vls-end//

/***********************************/
/* read and execute a command file */
/***********************************/
static void process_file(FILE* pfile)
{
    char buffer[BUFFER_SIZE], *cptr;

//vls-begin// script files
    EnterCriticalSection(&secReadingConfig);
    ResetEvent(eventReadingHasUse);
    SetEvent(eventReadingConfig);
//vls-end//
    while(fgets(buffer, sizeof(buffer), pfile)) {
        for(cptr=buffer; *cptr && *cptr!='\n'; cptr++);
        *cptr='\0';
        if ( *buffer  ) 
            parse_input(buffer); 
    }
//vls-begin// script files
    ResetEvent(eventReadingConfig);
    if (WaitForSingleObject(eventReadingHasUse, 0) == WAIT_OBJECT_0)
        PostMessage(hwndMAIN, WM_USER+300, 0, 0);
    else if(WaitForSingleObject(eventReadingFirst, 0) != WAIT_OBJECT_0)
        PostMessage(hwndMAIN, WM_USER+300, 0, 0);
    SetEvent(eventReadingFirst);
    LeaveCriticalSection(&secReadingConfig);
//vls-end//
}

//vls-begin// bugifx
//void read_command(char *filename)
void read_command(char *arg)
//vls-end//
{
    FILE *myfile ;
    char message[BUFFER_SIZE];
    // int flag = TRUE;

//vls-begin// bugifx
//    get_arg_in_braces(filename,filename, 1);
    char filename[BUFFER_SIZE];
    get_arg_in_braces(arg, filename, WITH_SPACES);
//vls-end//

//vls-begin// bugfix
    if ( *filename == 0 ) {
        tintin_puts2(rs::rs(1238));
        return;
    }
//vls-end//

//vls-begin// base dir
//    if((myfile=fopen(filename, "r"))==NULL) {
//        sprintf(message,rs::rs(1031),filename);
    char fn[MAX_PATH+2];

    MakeAbsolutePath(fn, filename, szBASE_DIR);
    if((myfile=fopen(fn, "r"))==NULL) {
        sprintf(message,rs::rs(1031), fn);
//vls-end//
        tintin_puts2(message);
    } else {
        process_file(myfile);
        fclose(myfile);
    }

//vls-begin// base dir
//    if((myfile=fopen("global.set", "r"))==NULL) {
    MakeAbsolutePath(fn, "global.set", szBASE_DIR);
    if((myfile=fopen(fn, "r"))==NULL) {
//vls-end//
        sprintf(message,rs::rs(1032));
        tintin_puts2(message);
    } else {
        process_file(myfile);
        fclose(myfile);
    }

    
    if (!verbose) {
        tintin_puts2(rs::rs(1033));
        sprintf(message,rs::rs(1034), AliasList.size());
        tintin_puts2(message);
        sprintf(message,rs::rs(1035),ActionList.size());
        tintin_puts2(message);
        sprintf(message,rs::rs(1036),antisubnum);
        tintin_puts2(message);
        sprintf(message,rs::rs(1037),subnum);
        tintin_puts2(message);
        sprintf(message,rs::rs(1038),VarList.size ());
        tintin_puts2(message);
        sprintf(message,rs::rs(1039),HlightList.size());
        tintin_puts2(message);
        sprintf(message,rs::rs(1040),GroupList.size());
        tintin_puts2(message);
    }

    if ( GroupList.find(DEFAULT_GROUP_NAME) == GroupList.end() ) {
        GroupList[DEFAULT_GROUP_NAME] = new CGROUP(DEFAULT_GROUP_NAME);
    }
}

/************************/
/* write a command file */
/************************/
void write_command(char *arg)
{
    FILE *myfile, *globfile;
    char buffer[BUFFER_SIZE], filename[BUFFER_SIZE], group[BUFFER_SIZE];
    struct listnode *nodeptr;
    int i;
    CGROUP* grp ;

    arg=get_arg_in_braces(arg, filename, WITH_SPACES);
    arg=get_arg_in_braces(arg, group, STOP_SPACES);

    if (*filename=='\0') {
        tintin_puts2(rs::rs(1041));
        return;
    }
    
    if ( *group ) {
        GROUP_INDEX ind = GroupList.find(group);
        if ( ind == GroupList .end() ) {
            char result[BUFFER_SIZE];
            sprintf ( result, rs::rs(1042), group);
            tintin_puts2(result);
            return;
        }
        grp = ind->second;
    }


    if ( *group == 0 ) {
//vls-begin// base dir
//        if((myfile=fopen(filename, "w"))==NULL) {
//            char buff[128];
//            sprintf(buff,rs::rs(1043),filename);
        char fn[MAX_PATH+2];
        MakeAbsolutePath(fn, filename, szBASE_DIR);
        if((myfile=fopen(fn, "w"))==NULL) {
            char buff[BUFFER_SIZE];
            sprintf(buff,rs::rs(1043), fn);
//vls-end//
            tintin_puts2(buff);
            return; 
        }
//vls-begin// base dir
//        if((globfile=fopen("global.set", "w"))==NULL) {
        MakeAbsolutePath(fn, "global.set", szBASE_DIR);
        if((globfile=fopen(fn, "w"))==NULL) {
//vls-end//
            tintin_puts2(rs::rs(1044));
            fclose(myfile);
            return; 
        }

        // save messages state
        if ( !mesvar[MSG_ALIAS] ) {
            buffer[0] = cCommandChar ;
            strcpy(buffer+1, "message alias OFF\n");
            fputs(buffer, myfile);
        }
        if ( !mesvar[MSG_ACTION] ) {
            buffer[0] = cCommandChar ;
            strcpy(buffer+1, "message action OFF\n");
            fputs(buffer, myfile);
        }
        if ( !mesvar[MSG_SUB] ) {
            buffer[0] = cCommandChar ;
            strcpy(buffer+1, "message subst OFF\n");
            fputs(buffer, myfile);
        }
        if ( !mesvar[MSG_ANTISUB] ) {
            buffer[0] = cCommandChar ;
            strcpy(buffer+1, "message antisub OFF\n");
            fputs(buffer, myfile);
        }
        if ( !mesvar[MSG_HIGH] ) {
            buffer[0] = cCommandChar ;
            strcpy(buffer+1, "message high OFF\n");
            fputs(buffer, myfile);
        }
        if ( !mesvar[MSG_VAR] ) {
            buffer[0] = cCommandChar ;
            strcpy(buffer+1, "message variable OFF\n");
            fputs(buffer, myfile);
        }
        if ( !mesvar[MSG_GRP] ) {
            buffer[0] = cCommandChar ;
            strcpy(buffer+1, "message group OFF\n");
            fputs(buffer, myfile);
        }
        if ( !mesvar[MSG_HOT] ) {
            buffer[0] = cCommandChar ;
            strcpy(buffer+1, "message hotkey OFF\n");
            fputs(buffer, myfile);
        }
//vls-begin// script files
        if ( !mesvar[MSG_SF] ) {
            buffer[0] = cCommandChar ;
            strcpy(buffer+1, "message uses OFF\n");
            fputs(buffer, myfile);
        }
//vls-end//
//* en
        if ( !mesvar[MSG_LOG] ) {
            buffer[0] = cCommandChar ;
            strcpy(buffer+1, "message logs OFF\n");
            fputs(buffer, myfile);
        }
//*/en

        // save togglesub/echo/multiaction etc states 
        buffer[0] = cCommandChar ;

        strcpy(buffer+1, "multiaction ");
        strcat (buffer, bMultiAction ? "on\n" : "off\n" );
        fputs(buffer, myfile);

        strcpy(buffer+1, "multihighlight ");
        strcat (buffer, bMultiHighlight ? "on\n" : "off\n" );
        fputs(buffer, myfile);

        strcpy(buffer+1, "presub ");
        strcat (buffer, presub ? "on\n" : "off\n" );
        fputs(buffer, myfile);

        strcpy(buffer+1, "echo ");
        strcat (buffer, echo ? "on\n" : "off\n" );
        fputs(buffer, myfile);

        strcpy(buffer+1, "ignore ");
        strcat (buffer, ignore? "on\n" : "off\n" );
        fputs(buffer, myfile);

        strcpy(buffer+1, "speedwalk ");
        strcat (buffer, speedwalk ? "on\n" : "off\n" );
        fputs(buffer, myfile);

        strcpy(buffer+1, "togglesubs ");
        strcat (buffer, togglesubs ? "on\n" : "off\n" );
        fputs(buffer, myfile);

        strcpy(buffer+1, "verbat ");
        strcat (buffer, verbatim ? "on\n" : "off\n" );
        fputs(buffer, myfile);

//* en:colon
        strcpy(buffer+1, "colon ");
        strcat (buffer, bColon ? "replace\n" : "leave\n" );
        fputs(buffer, myfile);
//* en:comment
        sprintf(buffer, "%ccomment %c\n",cCommandChar,cCommentChar);
        fputs(buffer, myfile);
//* en:race
        sprintf(buffer, "%crace format %s\n",cCommandChar,race_format);
        fputs(buffer, myfile);
//*/en


        ALIAS_INDEX ind = AliasList.begin();
        while (ind  != AliasList.end() ) {
            ALIAS* pal = ind->second;
            prepare_for_write("alias", (char*)ind->first.data(), (char*)pal->m_strRight.data(), 
                "\0", (char*)pal->m_pGroup->m_strName.data () , buffer);
            fputs(buffer, pal->m_pGroup->m_bGlobal ? globfile : myfile);
            ind++;
        }

        ACTION_INDEX aind = ActionList.begin();
        while (aind  != ActionList.end() ) {
            // CActionPtr pac = *aind;
            ACTION* pac = *aind;
            if ( !pac->m_bDeleted ) {
                char buff[32];
                prepare_for_write("action", (char*)pac->m_strLeft.data(), (char*)pac->m_strRight.data(), 
                    itoa(pac->m_nPriority, buff, 10) , (char*)pac->m_pGroup->m_strName.data() , buffer);
                fputs(buffer, pac->m_pGroup->m_bGlobal ? globfile : myfile);
            }
            aind++;
        }


        VAR_INDEX vind = VarList.begin();
        while ( vind != VarList.end() ) {
            VAR* pvar = vind->second;
            prepare_for_write("variable", (char*)vind->first.data(), (char*)pvar->m_strVal.data(), pvar->m_bGlobal ? "global" : "\0",  "\0", buffer);
            fputs(buffer, pvar->m_bGlobal ? globfile : myfile);
            vind++;
        }

        HLIGHT_INDEX hind = HlightList.begin();
        while ( hind != HlightList.end() ) {
            HLIGHT* ph = hind->second;
            prepare_for_write("highlight", (char*)ph->m_strColor.data(), (char*)hind->first.data(), 
                "\0", (char*)ph->m_pGroup->m_strName.data () , buffer);

            fputs(buffer, ph->m_pGroup->m_bGlobal ? globfile : myfile);
            hind++;
        }
    


      nodeptr=common_antisubs;
      while((nodeptr=nodeptr->next)) {
        prepare_for_write("antisubstitute", nodeptr->left,
        nodeptr->right, "\0",  "\0",buffer);
        fputs(buffer, myfile);
      } 
  
      nodeptr=common_subs;
      while((nodeptr=nodeptr->next)) {
        prepare_for_write("substitute", nodeptr->left, nodeptr->right, "\0",  "\0", buffer);
        fputs(buffer, myfile);
      }

      nodeptr=common_pathdirs;
      // !!! TO prevent default patgs writting
      i = 0;
      do {
        nodeptr=nodeptr->next;
        i++;
      } while ( nodeptr && i < 7 );

      while(nodeptr) {
        prepare_for_write("pathdir", nodeptr->right, nodeptr->left, "\0",  "\0",buffer);
        fputs(buffer, myfile);
        nodeptr=nodeptr->next;
      }

//vls-begin// grouped hotkeys
        // write hotkeys here 
        HOTKEY_INDEX hotind = HotkeyList.begin();
        while ( hotind != HotkeyList.end() ) {
            CHotKey* pHotKey= hotind->second;
            prepare_for_write("hot", (char*)pHotKey->m_strKey.data() ,
                (char*)pHotKey->m_strAction.data() , "\0",
                (char*)pHotKey->m_pGroup->m_strName.data(),  buffer);
            fputs(buffer, myfile);
            hotind++;
        }
//vls-end//

//vls-begin// script files
        SCRIPTFILE_INDEX sfind = ScriptFileList.begin();
        while ( sfind != ScriptFileList.end() ) {
            PCScriptFile psf = *sfind;
            prepare_for_write("use", (char*)psf->m_strName.data(), "\0", "\0", "\0", buffer);
            fputs(buffer, myfile);
            sfind++;
        }
//vls-end//
    
        // write groups states
        GROUP_INDEX gind = GroupList.begin();
        while ( gind != GroupList.end() ) {
            CGROUP* pg = gind->second;
            if ( !pg->m_bEnabled ) {
                sprintf( buffer, "%cgroup disable %s\n", cCommandChar , (char*)pg->m_strName.data());
                fputs(buffer, pg->m_bGlobal ? globfile : myfile);
            } 
            if ( pg->m_bGlobal ) {
                sprintf( buffer, "%cgroup global %s\n", cCommandChar , (char*)pg->m_strName.data());
                fputs(buffer, globfile );
            } else {
                sprintf( buffer, "%cgroup local %s\n", cCommandChar , (char*)pg->m_strName.data());
                fputs(buffer, pg->m_bGlobal ? globfile : myfile);
            }

            gind++;
        }

//vls-begin// grouped hotkeys
//        // write hotkeys here 
//        HOTKEY_INDEX hotind = HotkeyList.begin();
//        while ( hotind != HotkeyList.end() ) {
//            CHotKey* pHotKey= hotind->second;
//            prepare_for_write("hot", (char*)pHotKey->m_strKey.data() ,
//                (char*)pHotKey->m_strAction.data() , "\0",  "\0",buffer);
//            fputs(buffer, myfile);
//            hotind++;
//        }
//vls-end//

        // write ticksize now 
        sprintf(buffer , "%cticksize %d\n" , cCommandChar , tick_size );
        fputs(buffer, myfile);


        fclose(myfile);
        fclose(globfile);
        
    } else {
        if((myfile=fopen(filename, "a"))==NULL) {
            char buff[128];
            sprintf(buff,rs::rs(1045),filename);
            tintin_puts2(buff);
            return; 
        }
        fseek(myfile, 0 , SEEK_END);
        ALIAS_INDEX ind = AliasList.begin();
        while (ind  != AliasList.end() ) {
            ALIAS* pal = ind->second;
            if ( pal->m_pGroup == grp ) {
                prepare_for_write("alias", (char*)ind->first.data(), (char*)pal->m_strRight.data(), 
                    "\0", (char*)pal->m_pGroup->m_strName.data () , buffer);
                fputs(buffer, myfile);
            }
            ind++;
        }

        ACTION_INDEX aind = ActionList.begin();
        while (aind  != ActionList.end() ) {
            // CActionPtr pac = *aind;
            ACTION* pac = *aind;
            char buff[32];
            if ( pac->m_pGroup == grp && !pac->m_bDeleted ) {
                prepare_for_write("action", (char*)pac->m_strLeft.data(), (char*)pac->m_strRight.data(), 
                    itoa(pac->m_nPriority, buff, 10) , (char*)pac->m_pGroup->m_strName.data() , buffer);
                fputs(buffer, myfile);
            }
            aind++;
        }


        HLIGHT_INDEX hind = HlightList.begin();
        while ( hind != HlightList.end() ) {
            HLIGHT* ph = hind->second;
            if ( ph->m_pGroup == grp ) {
                prepare_for_write("highlight", (char*)ph->m_strColor.data(), (char*)hind->first.data(), 
                    "\0", (char*)ph->m_pGroup->m_strName.data () , buffer);

                fputs(buffer, myfile);
            }
            hind++;
        }

//vls-begin// grouped hotkeys
        HOTKEY_INDEX hotind = HotkeyList.begin();
        while ( hotind != HotkeyList.end() ) {
            CHotKey* pHotKey= hotind->second;
            prepare_for_write("hot", (char*)pHotKey->m_strKey.data() ,
                (char*)pHotKey->m_strAction.data() , "\0",
                (char*)pHotKey->m_pGroup->m_strName.data(),  buffer);
            fputs(buffer, myfile);
            hotind++;
        }
//vls-end//

        fclose(myfile);
    }
    tintin_puts2(rs::rs(1046));
}

void prepare_for_write(char *command, char *left, char *right, char *pr, char* group, char *result)
{
  /* char tmpbuf[BUFFER_SIZE]; */
  *result=cCommandChar;
  *(result+1)='\0';
  strcat(result, command);
  strcat(result, " {");
  strcat(result, left);
  strcat(result, "}");
  if (strlen(right)!=0) {
    strcat(result, " {");
    strcat(result, right);
    strcat(result, "}");
  }
  if (strlen(pr)!=0) {
    strcat(result, " {");
    strcat(result, pr);
    strcat(result, "}");
  }
  if (strlen(group)!=0) {
    strcat(result, " {");
    strcat(result, group);
    strcat(result, "}");
  }
  strcat(result,"\n");
}

void prepare_quotes(char *string)
{
  char s[BUFFER_SIZE], *cpsource, *cpdest;
  int nest=FALSE;
  strcpy(s, string);

  cpsource=s;
  cpdest=string;

 while(*cpsource) {
    if(*cpsource=='\\') {
      *cpdest++=*cpsource++;
      if(*cpsource)
        *cpdest++=*cpsource++;
    }
    else if(*cpsource=='\"' && nest==FALSE) {
      *cpdest++='\\';
      *cpdest++=*cpsource++;
    }
    else if(*cpsource=='{') {
      nest=TRUE;
      *cpdest++=*cpsource++;
    }
    else if(*cpsource=='}') {
      nest=FALSE;
      *cpdest++=*cpsource++;
    }
    else
      *cpdest++=*cpsource++; 
  }
  *cpdest='\0';
}


// Writting to log file without ESC characters

//vls-begin// multiple output
//void WriteToLog(char* str, int StrSize )
void WriteToLog(int wnd, char* str, int StrSize )
//vls-end//
{
    static DWORD LastTicker = 0;
    char* buff, *src, *out;
    int count;

//vls-begin// multiple output
    HANDLE hLogFile = wnd < 0 ? ::hLogFile : hOutputLogFile[wnd];
    BOOL bCurLogHTML = wnd < 0 ? ::bCurLogHTML : bOutputCurLogHTML[wnd];
//vls-end//


    if ( bCurLogHTML ) {
        bool lookup = true;
        bool new_line = true;
        DWORD Written;
        char * ptr = str;


	    while( lookup )
	    {
		    switch( *ptr )
		    {
		    case '\r':
			    break;

		    case '\n':
                WriteFile(hLogFile , brk, strlen(brk), &Written, NULL);
			    new_line = true;
			    break;

		    case 0x1b:
			    if( *(ptr+1) == '[' )
			    {
				    char* p = ptr+2;
				    while( *p && *p != 'm' )
					    p++;

				    if( p == '\0' )
				    {
					    lookup = false;
					    break;
				    }

				    parse((char*)ptr+2, (char*)p, new_attrib, new_tcolor, new_bcolor);

				    ptr = p;

				    if( attrib != new_attrib || tcolor != new_tcolor || bcolor != new_bcolor )
				    {
					    // _write(dst, espan);
					    // _write(dst, span(tcolor, bcolor, attrib));

					    strcpy(cache, espan);
					    strcat(cache, span(new_tcolor, new_bcolor, new_attrib));

    //					attrib = new_attrib;
    //					tcolor = new_tcolor;
    //					bcolor = new_bcolor;
				    }
			    }
			    else if( *(ptr+1) == 'p' )
			    {
				    char* p = ptr+2;
				    while( *p && *p != 'm' )
					    p++;
				    ptr = p;
			    }

			    break;

		    case '\0':
			    lookup = false;
			    break;

		    default:
			    if( strlen(cache) > 0 )
			    {
				    if( attrib != new_attrib || tcolor != new_tcolor || bcolor != new_bcolor )
					    WriteFile(hLogFile , cache, strlen(cache), &Written, NULL);
				    strcpy(cache, "");

				    attrib = new_attrib;
				    tcolor = new_tcolor;
				    bcolor = new_bcolor;
			    }
			    WriteFile(hLogFile , ptr, 1, &Written, NULL);
			    new_line = false;
			    break;
		    }

		    ptr++;
	    }
        

    } else {
        if ( LastTicker == 0 ) 
            LastTicker = GetTickCount();

        if ( hLogFile == NULL || StrSize <= 0 ) 
            return;
          // fwrite(temp, count, 1, ses->logfile);
        /* removing ESC CHARS */
        buff = (char*)malloc(StrSize*2+18);
        buff[0] = 0;
    
        // Add RMA ANSI command
        if ( bRMASupport ) {
            DWORD Currticker = GetTickCount();
            if ( Currticker - LastTicker ) {
                buff[0] = 0x1B;
                sprintf(buff+1 , "p:%dm" , Currticker - LastTicker);
            }
            LastTicker = Currticker;
        }

        src = str;
        count = strlen(buff);
        out = buff + count;
        do {
            if ( !bANSILog && *src == 0x1B ) {
                // Skip ESC
                do {
                    src++;
                    StrSize--;
                }while ( StrSize && *src != 'm' ) ;
                src++;
                StrSize--;
                continue;
            }
            *out++ = *src++;
            count++;
            StrSize--;
        } while (StrSize >0);

        if ( count ) {
            DWORD Written;
            WriteFile(hLogFile , buff , count , &Written, NULL);
        }
        free(buff);
    }
}

// str must be ESC free
//vls-begin// multiple output
//void WriteToLog2(char* str, int StrSize )
//{
void WriteToLog2(int wnd, char* str, int StrSize )
{
    HANDLE hLogFile = wnd < 0 ? ::hLogFile : hOutputLogFile[wnd];
//vls-end//

    DWORD Written;
    WriteFile(hLogFile , str, StrSize, &Written, NULL);
}

//vls-begin// base dir
// Examples:
// base = "c:\mud\jmc";
// 1. abs = "C:\muD\dir\file.ext";
//    loc = "..\dir\file.ext";
// 2. abs = "D:\dir";
//    loc = "D:\dir";
// 3. abs = "C:\mud\jmc\dir1\dir2\file3";
//    loc = "dir1\dir2\file3";

int MakeLocalPath(char *loc, const char *abs, const char *base)
{
    std::string sLoc;
    std::string sAbs(abs);
    std::string sBase(base);
    int iBase = 0;

    int nAbs = sAbs.length();
    int iAbs = 0;
    while ((iAbs = sAbs.find('/', iAbs)) != std::string::npos) sAbs[iAbs] = '\\';

    if (sAbs.length() < 4 || sAbs[1] != ':' || sAbs[2] != '\\') {
        strcpy(loc, sAbs.data());
        return nAbs;
    }

    int nBaseDirs = 0;
    sBase += '\\';
    for (iBase = 0; iBase < sBase.length(); iBase++) {
        if (sBase[iBase] == '\\') nBaseDirs++;
    }

    int nMatchDirs = 0;
    int nMin = min(sAbs.length(), sBase.length());
    iAbs = 0;
    for (iBase = 0; iBase < nMin; iBase++) {
        if (tolower(sBase[iBase]) != tolower(sAbs[iBase]))
            break;

        if (sAbs[iBase] == '\\') {
            iAbs = iBase + 1;
            nMatchDirs++;
        }
    }

    if (nMatchDirs > 0) {
        for (; nMatchDirs < nBaseDirs; nMatchDirs++) {
            sLoc += "..\\";
        }
    }
    sLoc.append(sAbs, iAbs, sAbs.length() - iAbs);
    if (sLoc.length() >= MAX_PATH) {
        strncpy(loc, sLoc.data(), MAX_PATH-1);
        loc[MAX_PATH-1] = '\0';
        return MAX_PATH-1;
    }
    strcpy(loc, sLoc.data());
    return sLoc.length();
}

int MakeAbsolutePath(char *abs, const char *loc, const char *base)
{
    std::string sAbs(base);
    std::string sLoc(loc);

    int nLoc = sLoc.length();
    int iLoc = 0;
    while ((iLoc = sLoc.find('/', iLoc)) != std::string::npos) sLoc[iLoc] = '\\';

    if (sLoc.length() > 3 && sLoc[1] == ':' && sLoc[2] == '\\') {
        strcpy(abs, sLoc.data());
        return nLoc;
    }

    iLoc = 0;
    int nCurDir = 0;
    while (iLoc < nLoc) {
        if (sLoc[iLoc] == '.' && nCurDir == 0) {
            if (iLoc + 2 < nLoc && sLoc[iLoc+1] == '.' && sLoc[iLoc+2] == '\\') {
                int iAbs = sAbs.rfind('\\');
                if (iAbs != std::string::npos) {
                    sAbs.resize(iAbs);
                }
                iLoc += 2;
                continue;
            }
        }
        if (sLoc[iLoc] == '\\') {
            nCurDir = 0;
            iLoc++;
            continue;
        }

        if (nCurDir == 0) {
            sAbs += '\\';
        }
        sAbs += sLoc[iLoc];
        nCurDir++;
        iLoc++;
    }
    if (sAbs.length() >= MAX_PATH) {
        strncpy(abs, sAbs.data(), MAX_PATH-1);
        abs[MAX_PATH-1] = '\0';
        return MAX_PATH-1;
    }
    strcpy(abs, sAbs.data());
    return sAbs.length();
}
//vls-end//

//vls-begin// multiple output
void DLLEXPORT InitOutputNameFunc(GET_OUTPUTNAME_FUNC OutputNameFunc)
{
    GetOutputName = OutputNameFunc;
}

void wlog_command(char *arg)
{
    char left[BUFFER_SIZE], right[BUFFER_SIZE], number[BUFFER_SIZE];
    int wnd;

    arg=get_arg_in_braces(arg, number, STOP_SPACES);
    arg=get_arg_in_braces(arg, left, WITH_SPACES);
    arg=get_arg_in_braces(arg, right, WITH_SPACES);

    if (!sscanf(number, "%d", &wnd) || wnd < 0 || wnd >= MAX_OUTPUT || !*left) {
        tintin_puts(rs::rs(1241));
        return;
    }
    if ( StartLog(wnd, left, right) && mesvar[MSG_LOG]) {
        char msg[BUFFER_SIZE], name[BUFFER_SIZE];
        GetOutputName(wnd, name, BUFFER_SIZE);
        sprintf(msg, rs::rs(1243), left, name);
        tintin_puts2(msg);
    }
}

void StopLogging()
{
    DWORD Written;

    if (hLogFile) {
        if ( bCurLogHTML )
            WriteFile(hLogFile , epilog, strlen(epilog), &Written, NULL);
        CloseHandle(hLogFile);
        hLogFile = NULL;
    }

    for (int i = 0; i < MAX_OUTPUT; i++) {
        if (hOutputLogFile[i]) {
            if ( bOutputCurLogHTML[i] )
                WriteFile(hOutputLogFile[i] , epilog, strlen(epilog), &Written, NULL);
            CloseHandle(hOutputLogFile[i]);
            hOutputLogFile[i] = NULL;
			sOutputLogName[i][0] = '\0';
        }
    }
};
//vls-end//

//* en


void sos_command(char *arg)
{
	char command[BUFFER_SIZE],
		 paramet[BUFFER_SIZE];

	arg = get_arg_in_braces(arg, command, STOP_SPACES);
	arg = get_arg_in_braces(arg, paramet, WITH_SPACES);

    int i;
    char buffer[BUFFER_SIZE];

    if(command[0]=='m' && is_abrev(command,"mode"))
	{
		if(is_abrev(paramet,"exact"))
			bSosExact = TRUE;
		else
			bSosExact = FALSE;
		return;
	}

    if(command[0]=='l' && is_abrev(command,"list"))
	{
		for(i=0;soski[i].name[0];i++)
		{
			sprintf(buffer,"¹%2d group=%10s;entity=%40s",i,soski[i].group,soski[i].name);
			tintin_puts2(buffer);
		}
	}

    if(command[0]=='c' && is_abrev(command,"clear"))
	{
		for(i=0;soski[i].name[0];i++)
		{
			soski[i].group[0]=0;
			soski[i].name[0]=0;
		}
		bSosExact = FALSE;
		return;
	}
	
	if(command[0]=='s' && is_abrev(command,"save"))
	{//save soski

    FILE *myfile;
        char fn[MAX_PATH+2];
        MakeAbsolutePath(fn, paramet, szBASE_DIR);
        if((myfile=fopen(fn, "w"))==NULL) {
            char buff[BUFFER_SIZE];
            sprintf(buff,rs::rs(1043), fn);
            tintin_puts2(buff);
            return; 
        }

		sprintf(buffer,"%c%c%c JMC Save Our Vars dump\n",cCommandChar,cCommandChar,cCommandChar);
		fputs(buffer,myfile);

		ALIAS_INDEX ind = AliasList.begin();
        while (ind  != AliasList.end() ) 
		{
            ALIAS* pal = ind->second;
			for(i=0;soski[i].name[0];i++)
			{
              if((is_abrev(soski[i].group,"alias") && (
				  (soski[i].name[0] == '*')||
				 (!bSosExact && is_abrev(soski[i].name,(char*)ind->first.data()))||
				  (bSosExact && !strcmpi(soski[i].name,(char*)ind->first.data())))))
			  {
                prepare_for_write("alias", (char*)ind->first.data(), (char*)pal->m_strRight.data(), 
                  "\0", (char*)pal->m_pGroup->m_strName.data () , buffer);
                fputs(buffer, myfile);
			  }
			}
            ind++;
        }

        ACTION_INDEX aind = ActionList.begin();
        while (aind  != ActionList.end() ) 
		{

            ACTION* pac = *aind;
            if ( !pac->m_bDeleted ) 
			{
			  for(i=0;soski[i].name[0];i++)
			  {
              if(is_abrev(soski[i].group,"action") && (
				  (soski[i].name[0] == '*')||
				  (!bSosExact && is_abrev(soski[i].name,(char*)pac->m_strLeft.data()))||
				  (bSosExact && !strcmpi(soski[i].name,(char*)pac->m_strLeft.data()))))
			  {
				  char buff[32];
                  prepare_for_write("action", (char*)pac->m_strLeft.data(), (char*)pac->m_strRight.data(), 
                      itoa(pac->m_nPriority, buff, 10) , (char*)pac->m_pGroup->m_strName.data() , buffer);
                  fputs(buffer, myfile);
				}
			  }
			}
            aind++;
        }


        VAR_INDEX vind = VarList.begin();
        while ( vind != VarList.end() ) 
		{
            VAR* pvar = vind->second;
			for(i=0;soski[i].name[0];i++)
			{
              if(is_abrev(soski[i].group,"variable") && (
				  (soski[i].name[0] == '*')||
				  (!bSosExact && is_abrev(soski[i].name,(char*)vind->first.data()))||
				  (bSosExact && !strcmpi(soski[i].name,(char*)vind->first.data()))))
			  {
				prepare_for_write("variable", (char*)vind->first.data(), (char*)pvar->m_strVal.data(), 
					pvar->m_bGlobal ? "global" : "\0",  "\0", buffer);
				fputs(buffer, myfile);
			  }  
			} 
            vind++;
        }

        HOTKEY_INDEX hotind = HotkeyList.begin();
        while ( hotind != HotkeyList.end() ) {
            CHotKey* pHotKey= hotind->second;
			for(i=0;soski[i].name[0];i++)
			{
            if(is_abrev(soski[i].group,"hotkey") && (
				(soski[i].name[0] == '*')||
				(!bSosExact && is_abrev(soski[i].name,(char*)pHotKey->m_strKey.data()))||
				(bSosExact && !strcmpi(soski[i].name,(char*)pHotKey->m_strKey.data()))))
			{
            prepare_for_write("hot", (char*)pHotKey->m_strKey.data() ,
                (char*)pHotKey->m_strAction.data() , "\0",
                (char*)pHotKey->m_pGroup->m_strName.data(),  buffer);
            fputs(buffer, myfile);
			}
			}
            hotind++;
        }

        fclose(myfile);
		return;
	}
	
	    for(i=0;soski[i].name[0];i++)
			if(is_abrev(soski[i].name,paramet)&&is_abrev(soski[i].group,command))
				return;
		
        for(i=0;soski[i].name[0];i++);
		if(is_abrev(command,"hotkey") || 
			is_abrev(command,"alias") || 
			is_abrev(command,"action") || 
			is_abrev(command,"variable"))
			strcpy(buffer,command);
		if(buffer[0] && paramet[0])
            strcpy(buffer,command);
		else 
			return;
		strcpy(soski[i].group,buffer);
		strcpy(soski[i].name,paramet);


	;//new soska
	
}
//*/en
