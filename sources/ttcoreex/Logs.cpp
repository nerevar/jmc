#include "stdafx.h"
#include "tintin.h"
#include <time.h>
#include <io.h>

#include "Logs.h"
#include "Utils.h"

#include <string>
#include <fstream>
#include <iostream>
#include <vector>

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

DWORD lastTicker = 0;

GET_OUTPUTNAME_FUNC GetOutputName;

//vls-begin// multiple output
void DLLEXPORT InitOutputNameFunc(GET_OUTPUTNAME_FUNC OutputNameFunc)
{
    GetOutputName = OutputNameFunc;
}
//vls-end//

void debug(char *pszFormat, ...)
{
    va_list marker;
    va_start(marker, pszFormat);
    string str = StrPrintfV(pszFormat, marker);
    va_end(marker);

	ofstream myfile;
	myfile.open ("debug.txt", ios::out | ios::app);
	
	myfile << str << "\n";
	
	myfile.close();
}

void debug(string str)
{
	ofstream myfile;
	myfile.open ("debug.txt", ios::out | ios::app);
	
	myfile << str << "\n";
	
	myfile.close();
}

void log(string st)
{
	if (!logFile.is_open())
		return;
		
	logFile << st;
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
		//log(html_header);
		log(TAG_OPEN);
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

std::vector<int> processParams(string params)
{
	vector<int> paramsList;

	if (params == "" || params == "0") {
		paramsList.push_back(0);
	} else {
		paramsList = split(params, ';');

		// remove first 0 param - it's useless
		if (paramsList.at(0) == 0 && paramsList.size() > 1) {
			paramsList.erase(paramsList.begin());
		}
	}

	return paramsList;
}

string getCSSClass(int param)
{
	// add css light color
	if (param == 1) 
		return CSS_LIGHT_COLOR;

	// set text color
	if (param >= 30 && param <= 37)
		return css_colors[ param - 30 ];

	// TODO: set background color
	if (param >= 40 && param <= 47)
		return css_bg_colors[ param - 40 ];

	return "";
}

// remove \r symbols
void stripCR(string &strInput) {
	string::size_type pos = 0;
    while ( ( pos = strInput.find("\r", pos) ) != string::npos ) {
        strInput.erase ( pos, 1 );
    }
}

// remove RMA tags
void stripRMA(string &strInput) {
	string::size_type pos = 0;
    while ( ( pos = strInput.find("\x1Bp", pos) ) != string::npos ) {
		strInput.erase(pos, strInput.find('m', pos) - pos + 1);
    }
}

// remove [0m duplicates
void stripDefaultColorDuplicates(string &strInput) {
	string::size_type pos = 0;
    while ( ( pos = strInput.find("\x1B[0m\x1B[0m", pos) ) != string::npos ) {
		strInput.erase(pos, 4);
    }
}


string processHTML(string strInput)
{
	string strOutput = strInput;

	stripCR(strOutput);

	stripRMA(strOutput);

	stripDefaultColorDuplicates(strOutput);

	string::size_type pos = 0;
    while ( ( pos = strOutput.find("\x1B[", pos) ) != string::npos ) {
		string::size_type 
			posEscEnd = strOutput.find("m", pos),
			posSpace = 0;

		string params = strOutput.substr(pos + 2, posEscEnd - pos - 2);

		// replace spaces to ";"
		while ( ( posSpace = params.find(" ", posSpace) ) != string::npos ) {
			params.replace(posSpace, 1, ";");
		}

		// parse params string to vector
		vector<int> paramsList = processParams(params);

		string 
			css_class = "",
			html_tags = "";

		// for each parameter add css class
		vector<int>::iterator itr;
		for(itr = paramsList.begin(); itr != paramsList.end(); itr++) {
			string newCSS = getCSSClass(*itr);

			if (newCSS.length() > 0) {
				if (css_class.length() > 0) (css_class += " ");
				css_class += newCSS;
			}
		}
		
		html_tags += TAG_CLOSE;

		// create html entity
		if (css_class.length() > 0)
			html_tags += string("<") + HTML_TAG + string(" class=\"") + css_class + string("\"") + string(">");
		else 
			html_tags += TAG_OPEN;

		// replace source escape sequence to html entity
		strOutput.replace(pos, posEscEnd - pos + 1, html_tags);

		pos = strOutput.find("\x1B[", pos);
    }

	return strOutput;
}

string processRMA(string strInput)
{
	string strOutput;
	DWORD currTicker = 0;

    if ( lastTicker == 0 ) 
        lastTicker = GetTickCount();

	currTicker = GetTickCount();

    if ( currTicker - lastTicker ) {
		strOutput = strprintf("%cp:%dm", 0x1B, currTicker - lastTicker);
		strOutput += strInput;
    } else {
		strOutput = strInput;
	}

	lastTicker = currTicker;

	return strOutput;
}

string processTEXT(string strInput)
{
	// delete everything from 0x1B to 'm'
	string strOutput = strInput;
	int escPos = 0,
		endEscPos = 0;

	while ( (escPos = strOutput.find(0x1B, escPos)) != string::npos) {
		endEscPos = strOutput.find('m', escPos);

		strOutput.erase(escPos, endEscPos - escPos + 1);
	}

	return strOutput;
}


string processLine(char *charInput)
{
	string strInput(charInput), strOutput;
	
	if (bCurLogHTML) {
		// parse line to HTML
		strOutput = processHTML(strInput);
	} else if (bANSILog) {
		// keep ANSI codes
		strOutput = bRMASupport
			? processRMA(strInput)
			: strInput;
	} else {
		// strip all Esc-sequences
		strOutput = processTEXT(strInput);
	}

	return strOutput;
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