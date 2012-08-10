#include "stdafx.h"
#include "tintin.h"
#include <time.h>
#include <io.h>
#include <string>
#include "Logs.h"
#include <fstream>
#include <iostream>

using namespace std;

//vls-begin// #logadd + #logpass
BOOL bLogPassedLine = FALSE;
//vls-end//

static BOOL bCurLogHTML = FALSE;
static BOOL bOutputCurLogHTML[MAX_OUTPUT];

static int attrib;
static int tcolor;
static int bcolor;

static int new_attrib = attrib;
static int new_tcolor = tcolor;
static int new_bcolor = bcolor;

string strCache = "";


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
		sprintf(buf, "<i class=\"%s\">", dtable[ tcolor-30 ]);
		break;

	case 1:
		sprintf(buf, "<i class=\"%s\">", ltable[ tcolor-30 ]);
		break;

	default:
		sprintf(buf, "<i class=\"%s\">", dtable[ tcolor-30 ]);
		break;
	}

	return buf;
}

GET_OUTPUTNAME_FUNC GetOutputName;

//vls-begin// multiple output
void DLLEXPORT InitOutputNameFunc(GET_OUTPUTNAME_FUNC OutputNameFunc)
{
    GetOutputName = OutputNameFunc;
}
//vls-end//


void debug(string st)
{
	ofstream myfile;
	myfile.open ("debug.txt", ios::out | ios::app);
	
	myfile << st << "\n";
	
	myfile.close();
}

void log(string st)
{
	if (logFile.is_open()) {
		logFile << st;
	}
}

//vls-begin// multiple output
BOOL StartLog(int wnd, char* left, char *right)
{

	// TODO: wnd
	if (wnd > 0)
		return false;

//vls-begin// multiple output
    //HANDLE *hLogFile = wnd < 0 ? &::hLogFile : &hOutputLogFile[wnd];
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

    if (logFile.is_open()) { // Close log file now opened 
        if(mesvar[MSG_LOG]) {
			char message[BUFFER_SIZE];
			sprintf(message, rs::rs(1024), logName);
			tintin_puts2(message);
		}
        
		if ( *bCurLogHTML ) {
			log(html_footer);
        }

		logFile.close();

    	strcpy(logName,left);

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
		logFile.open (left, ios::out | ios::app);
	} else {
		logFile.open (left);
	}

	if (!logFile) {
		char buff[128];
		sprintf(buff,rs::rs(1028), left);
		tintin_puts2(buff);
		return FALSE;		
	}

    // Do HTML Log pereference
    if ( *bCurLogHTML ) {
		log(html_header);

	    attrib = 0;
	    tcolor = 37;
	    bcolor = 40;

		log(span(tcolor, bcolor, attrib));

		strCache = "";
    }

	GetLocalTime(&stl);

    if (wnd < 0)
        sprintf(Timerecord, rs::rs(1029) , stl.wDay, stl.wMonth , stl.wYear , stl.wHour, stl.wMinute);
    else
        sprintf(Timerecord, rs::rs(1242) , wnd, stl.wDay, stl.wMonth , stl.wYear , stl.wHour, stl.wMinute);
    
	log(Timerecord);
	
	strcpy(logName,left);
    return TRUE;
}
//vls-end//

string processHTML(string strInput)
{
	return strInput;
}

string processANSI(string strInput, int isRMA)
{
	return strInput;
}

string processTEXT(string strInput)
{
	// delete everything from 0x1B to 'm'
	string strOutput = strInput;
	int escPos = strOutput.find(0x1B),
		endEscPos = strOutput.find('m', escPos);

	while (escPos != string::npos) {
		strOutput.erase(escPos, endEscPos - escPos + 1);
		
		escPos = strOutput.find(0x1B),
		endEscPos = strOutput.find('m', escPos);
	}

	return strOutput;
}


string processLine(char *charInput, int StrSize)
{
	string strInput(charInput), strOutput;
	
	if (bCurLogHTML) {
		// parse line to HTML
		strOutput = processHTML(strInput);
	} else if (bANSILog) {
		// keep ANSI codes
		if (bRMASupport)
			strOutput = processANSI(strInput, bRMASupport);
		else
			// add RMA tags
			strOutput = strInput;	
	} else {
		// strip all Esc-sequences
		strOutput = processTEXT(strInput);
	}

	return strOutput;
}

/**
 * Writing to log file without ESC characters
 */
void rez_WriteToLog(int wnd, char* str, int StrSize )
{
    static DWORD LastTicker = 0;
    char *buff, *src, *out;
    int count;

//vls-begin// multiple output
    HANDLE hLogFile = wnd < 0 ? ::hLogFile : hOutputLogFile[wnd];
    BOOL bCurLogHTML = wnd < 0 ? ::bCurLogHTML : bOutputCurLogHTML[wnd];
//vls-end//


    if ( bCurLogHTML ) {
        bool lookup = true;
        bool new_line = true;
        char *ptr = str;


	    while( lookup )
	    {
		    switch( *ptr )
		    {
		    case '\r':
			    break;

		    case '\n':
				log(BR);
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

						strCache = TAG_CLOSE;
						strCache += span(new_tcolor, new_bcolor, new_attrib);

    //					attrib = new_attrib;
    //					tcolor = new_tcolor;
    //					bcolor = new_bcolor;
				    }
				} else if( *(ptr+1) == 'p' ) {
				    char *p = ptr+2;
				    while( *p && *p != 'm' )
					    p++;
				    ptr = p;
			    }

			    break;

		    case '\0':
			    lookup = false;
			    break;

		    default:
				// flush cache to file
			    if( strCache.length() > 0 )
			    {
					if( attrib != new_attrib || tcolor != new_tcolor || bcolor != new_bcolor ) {
						log(strCache);
					}
					//    WriteFile(hLogFile , strCache.c_str(), strCache.length(), &Written, NULL);
				    
					//strcpy(strCache, "");
					strCache = "";

				    attrib = new_attrib;
				    tcolor = new_tcolor;
				    bcolor = new_bcolor;
			    }

			    //WriteFile(hLogFile , ptr, 1, &Written, NULL);
				log(ptr);
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

        //if ( count ) {
        //    DWORD Written;
        //    WriteFile(hLogFile , buff , count , &Written, NULL);
        //}
		log(buff);
        free(buff);
    }
}

/**
 * Writes line to log file without any parsing
 */
void WriteLineToLog(int wnd, char* str, int StrSize )
{
	// TODO: wnd
	if (wnd > 0)
		return;

	log(str);
}


/**
 * Stops writing to log file and close file handle
 */
void StopLogging()
{
    if (logFile.is_open()) {
        if ( bCurLogHTML ) {
			log(html_footer);
		}
		logFile.close();
    }

	// TODO: wnd
	return;
    for (int i = 0; i < MAX_OUTPUT; i++) {
        if (hOutputLogFile[i]) {

            if ( bOutputCurLogHTML[i] ) {}
                //WriteFile(hOutputLogFile[i] , html_footer.c_str(), html_footer.length(), &Written, NULL);

            CloseHandle(hOutputLogFile[i]);
            hOutputLogFile[i] = NULL;
			sOutputLogName[i][0] = '\0';
        }
    }
};




/********************/
/* the #log command */
/********************/
void log_command(char *arg)
{
    char filename[BUFFER_SIZE], params[BUFFER_SIZE];

    arg=get_arg_in_braces(arg, filename, STOP_SPACES);
    arg=get_arg_in_braces(arg, params,   STOP_SPACES);

    if ( StartLog(-1, filename, params) && mesvar[MSG_LOG]) {
        char msg[BUFFER_SIZE];
        sprintf(msg, rs::rs(1030), filename);
        tintin_puts2(msg);
    }
}


// Declare JMC commands


/***********************/
/* the #logadd command */
/***********************/
void logadd_command(char *arg)
{
    char msg[BUFFER_SIZE];
	char tmp[BUFFER_SIZE];

	get_arg_in_braces(arg, arg, WITH_SPACES);

    substitute_vars(arg,tmp);
    substitute_myvars(tmp,msg);
    strcat(msg, "\n");

	log(msg);
}

/************************/
/* the #logpass command */
/************************/
void logpass_command(char *arg)
{
    bLogPassedLine = TRUE;
}
//vls-end//


/********************/
/* the #wlog command */
/********************/
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