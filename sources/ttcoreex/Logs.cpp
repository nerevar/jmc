#include "stdafx.h"
#include "resource.h"
#include "tintin.h"
#include <time.h>
#include <io.h>

#include "Logs.h"
#include "Utils.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <streambuf>

//vls-begin// #logadd + #logpass
BOOL bLogPassedLine = FALSE;
//vls-end//

int DLLEXPORT nScrollSize = 300;
ScrollLineRec *pScrollLinesBuffer = NULL;
int ScrollBufferCapacity = 0, ScrollBufferBegin = 0, ScrollBufferEnd = 0;

static BOOL bCurLogHTML = FALSE;

static int attrib;
static int tcolor;
static int bcolor;

static int new_attrib = attrib;
static int new_tcolor = tcolor;
static int new_bcolor = bcolor;

static string current_css_class = "";

DWORD lastTicker = 0;
DWORD firstTicker = 0;

GET_OUTPUTNAME_FUNC GetOutputName;

//vls-begin// multiple output
void DLLEXPORT InitOutputNameFunc(GET_OUTPUTNAME_FUNC OutputNameFunc)
{
    GetOutputName = OutputNameFunc;
}
//vls-end//

void add_line_to_scrollbuffer(const char *line)
{
	nScrollSize = max(MIN_SCROLL_SIZE, min(nScrollSize, MAX_SCROLL_SIZE));
	if(ScrollBufferCapacity < nScrollSize) {
		pScrollLinesBuffer = (ScrollLineRec*)realloc(pScrollLinesBuffer, nScrollSize * sizeof(ScrollLineRec));

		if(ScrollBufferEnd < ScrollBufferBegin) {
			int taillen = ScrollBufferCapacity - ScrollBufferBegin;
			memmove(&pScrollLinesBuffer[nScrollSize - taillen], &pScrollLinesBuffer[ScrollBufferBegin], taillen * sizeof(ScrollLineRec));
			memset(&pScrollLinesBuffer[ScrollBufferBegin], 0, (nScrollSize - ScrollBufferCapacity) * sizeof(ScrollLineRec));
			ScrollBufferBegin = nScrollSize - taillen;
		} else {
			memset(&pScrollLinesBuffer[ScrollBufferCapacity], 0, (nScrollSize - ScrollBufferCapacity) * sizeof(ScrollLineRec));
		}

		ScrollBufferCapacity = nScrollSize;
	}
	pScrollLinesBuffer[ScrollBufferEnd].timestamp = GetTickCount();
	pScrollLinesBuffer[ScrollBufferEnd].line = string(line);
	ScrollBufferEnd = (ScrollBufferEnd + 1) % ScrollBufferCapacity;
	if(ScrollBufferEnd == ScrollBufferBegin)
		ScrollBufferBegin = (ScrollBufferBegin + 1) % ScrollBufferCapacity;
}

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
	if (!hLogFile.is_open())
		return;
		
	hLogFile << st;
}

void log(int wnd, string st)
{

	if (!(wnd >= 0)) {
		log(st);
		return;
	}

	if (!hOutputLogFile[wnd].is_open())
		return;
		
	hOutputLogFile[wnd] << st;
}

string loadHTMLFromResource(int name)
{
	DWORD size;
	HRSRC rc = ::FindResource(rs::hInst, MAKEINTRESOURCE(name), RT_HTML);
    HGLOBAL rcData = ::LoadResource(rs::hInst, rc);

    size = ::SizeofResource(rs::hInst, rc);
	string html_content(static_cast<const char*>(::LockResource(rcData)), size);

	return html_content;
}

string loadHTMLFromFile(const char *filename)
{
	char fn[MAX_PATH+2];
	string ret;

	MakeAbsolutePath(fn, filename, szBASE_DIR);

	ifstream t(filename);
	if(t.fail()) {
		char message[BUFFER_SIZE];
		sprintf(message,rs::rs(1262), fn);
        tintin_puts2(message);
		ret = loadHTMLFromResource(IDR_HTML_HEAD);
	} else {
		stringstream buffer;
		buffer << t.rdbuf();
		ret = buffer.str();
	}

	return ret;
}

BOOL CloseMainLog(char *logName, BOOL logHTML)
{
    if (hLogFile.is_open()) { // Close log file now opened 
        if(mesvar[MSG_LOG]) {
			char message[BUFFER_SIZE];
			sprintf(message, rs::rs(1024), logName);
			tintin_puts2(message);
		}
        
		if ( logHTML ) {
			log(html_footer);
        }

		hLogFile.close();

		strcpy(sLogName, "");
		
		return TRUE;
    }

	return FALSE;
}

BOOL CloseWNDLog(int wnd, char *logName)
{
    if (hOutputLogFile[wnd].is_open()) { // Close log file now opened 
        if(mesvar[MSG_LOG]) {
			char message[BUFFER_SIZE];
			sprintf(message, rs::rs(1024), logName);
			tintin_puts2(message);
		}
        
		hOutputLogFile[wnd].close();

		strcpy(sOutputLogName[wnd], "");

		return TRUE;
    }

	return FALSE;
}

BOOL StartMainLog(char* logName, BOOL logMode, BOOL logHTML)
{
	lastTicker = firstTicker = 0;

	if (logMode && !logHTML)
		hLogFile.open(logName, ios::out | ios::binary | ios::app );
	else 
		hLogFile.open(logName, ios::out | ios::binary );

	if (!hLogFile.is_open())
		return FALSE;

    // Do HTML Log pereference
    if ( logHTML ) {
		string html_header = loadHTMLFromFile("html.log.template");

		string::size_type pos = 0;
		while ( ( pos = html_header.find("%title%", pos) ) != string::npos ) {
			html_header.replace(pos, 7, logName);
		}

		log(html_header);
		//log(TAG_OPEN);
		current_css_class = "";
    }

	return TRUE;
}

BOOL StartWNDLog(int wnd, char* logName, BOOL logMode)
{
	if (logMode)
		hOutputLogFile[wnd].open(logName, ios::out | ios::binary | ios::app );
	else 
		hOutputLogFile[wnd].open(logName, ios::out | ios::binary );

	return hOutputLogFile[wnd] != NULL ? TRUE : FALSE;
}

//vls-begin// multiple output
BOOL StartLog(int wnd, char* left, char *right, int dumplines = 0)
{
	BOOL status;
	char *logName = wnd >= 0 ? sOutputLogName[wnd] : sLogName;

	SYSTEMTIME stl;
    char Timerecord[BUFFER_SIZE], logTitle[BUFFER_SIZE];
    BOOL bLogMode = bDefaultLogMode;

//* en	
	// do nothing on second command "#log <logname>", but overwrite or html
	if(!strcmpi(left,logName) && strcmpi(right, "overwrite") && strcmpi(right, "html") )
		return FALSE;
//*/en	

	// try to close previous log with previous logName
	if (wnd >= 0) {
		status = CloseWNDLog(wnd, logName);
	} else {
		status = CloseMainLog(logName, bCurLogHTML);
	}

	// successfully closed previous log by #log command
	if (status)
		return FALSE;

    if ( !*left) {
        tintin_puts2(rs::rs(1025));
        return FALSE;
    }

	// set new log name from params
	strcpy(logName, left);

	// disable HTML mode for output windows
	if (!(wnd >= 0))
		bCurLogHTML = bHTML;
    
	if ( *right ) {
        if ( !strcmpi(right, "append") ) { 
			// set append mode 
            bLogMode = TRUE;
            if ( !(wnd >= 0) && bCurLogHTML ) 
                tintin_puts2(rs::rs(1026));
        } else if ( !strcmpi(right, "overwrite") ) { 
			// set overwrite mode 
            bLogMode = FALSE;
        } else if ( !strcmpi(right, "html") )  {
			// set HTML mode for Main log
			if (!(wnd >= 0))
				bCurLogHTML = TRUE;
		} else {
			// error in params
            tintin_puts2(rs::rs(1027));
            return FALSE;
        }
    }

	if (wnd >= 0) {
		status = StartWNDLog(wnd, logName, bLogMode);
	} else {
		status = StartMainLog(logName, bLogMode, bCurLogHTML);
	}

	if (!status) {
		char buff[128];
		sprintf(buff,rs::rs(1028), logName);
		tintin_puts2(buff);
		return FALSE;		
	}	

	if (bAppendLogTitle) {
		GetLocalTime(&stl);

		sprintf(logTitle, rs::rs(1258) , logName);
		log(wnd, logTitle);

		if (wnd >= 0)
			sprintf(Timerecord, rs::rs(1242) , wnd, stl.wDay, stl.wMonth , stl.wYear , stl.wHour, stl.wMinute);
		else
			sprintf(Timerecord, rs::rs(1029) , stl.wDay, stl.wMonth , stl.wYear , stl.wHour, stl.wMinute);
	    
		log(wnd, Timerecord);
	}

	if (dumplines > 0) {
		int i;
		for (i = ScrollBufferBegin; 
		     i != ScrollBufferEnd && dumplines > 0; 
			 i = (i + 1) % ScrollBufferCapacity, dumplines--) {
			log(processLine(pScrollLinesBuffer[i].line.c_str(), pScrollLinesBuffer[i].timestamp));
			log("\n");
		}
	}
	
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

	if (param >= 40 && param <= 47)
		return css_bg_colors[ param - 40 ];

	return "";
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

string processHTML(string strInput, DWORD TimeStamp)
{
	string strOutput = "";

	BOOL close_timestamp_tag = FALSE;

	if ( bHTMLTimestamps ) {
		DWORD currTicker = 0;

		if ( !TimeStamp )
			TimeStamp = GetTickCount();

		if ( firstTicker == 0 ) {
			firstTicker = TimeStamp;
			lastTicker = 0;
		}

		currTicker = TimeStamp - firstTicker;

		if ( currTicker - lastTicker >= MIN_HTML_FRAMES_DELAY_MS ) {
			strOutput += strprintf("<div class=\"t %d\">", currTicker);
			lastTicker = currTicker;
			close_timestamp_tag = TRUE;
		}
	}
	
	string::size_type pos = 0, processed = 0;
	string::size_type posEscEnd, posSpace;
	string next_css_class = current_css_class, outputted_css_class = "";

	BOOL tag_opened = FALSE;
	string text = "";
	while (processed < strInput.length()) {
		pos = strInput.find("\x1B[", processed);

		if (pos != string::npos) {
			posEscEnd = strInput.find("m", pos);

			if (posEscEnd != string::npos) {
				posSpace = 0;

				string params = strInput.substr(pos + 2, posEscEnd - pos - 2);

				// replace spaces to ";"
				while ( ( posSpace = params.find(" ", posSpace) ) != string::npos ) {
					params.replace(posSpace, 1, ";");
				}

				// parse params string to vector
				vector<int> paramsList = processParams(params);

				next_css_class = "";
				// for each parameter add css class
				vector<int>::iterator itr;
				for(itr = paramsList.begin(); itr != paramsList.end(); itr++) {
					string newCSS = getCSSClass(*itr);

					if (newCSS.length() > 0) {
						if (next_css_class.length() > 0) (next_css_class += " ");
						next_css_class += newCSS;
					}
				}
			} else { //unknown escape sequense: skip to the end of line
				next_css_class = current_css_class;
				processed = pos = posEscEnd = strInput.length();
			}
		} else {
			next_css_class = current_css_class;
			pos = posEscEnd = strInput.length();
		}

		text = strInput.substr(processed, pos - processed);
		stripRMA(text);
		if (text.length() > 0) {
			if (outputted_css_class != current_css_class || !tag_opened) {
				if (tag_opened)
					strOutput += TAG_CLOSE;
				if (current_css_class.length() > 0)
					strOutput += string("<") + HTML_TAG + string(" class=\"") + current_css_class + string("\"") + string(">");
				else 
					strOutput += TAG_OPEN;
				outputted_css_class = current_css_class;
				tag_opened = TRUE;
			}
			strOutput += text;
		}
		processed = posEscEnd + 1;
		
		current_css_class = next_css_class;
    }
	if (tag_opened)
		strOutput += TAG_CLOSE;

	if (close_timestamp_tag == TRUE) {
		strOutput += "</div>";
	}

	return strOutput;
}

string processRMA(string strInput, DWORD TimeStamp)
{
	string strOutput;
	DWORD currTicker = 0;

	if ( !TimeStamp )
		TimeStamp = GetTickCount();

    if ( lastTicker == 0 ) 
        lastTicker = TimeStamp;

	currTicker = TimeStamp;

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
		if ( (endEscPos = strOutput.find('m', escPos)) == string::npos ) {
			//incorrect escape sequence, erase entire string
			strOutput.erase(escPos);
		} else {
			strOutput.erase(escPos, endEscPos - escPos + 1);
		}
	}

	return strOutput;
}


string processLine(const char *charInput, DWORD TimeStamp)
{
	string strInput(charInput), strOutput;
	
	if (bCurLogHTML) {
		// parse line to HTML
		strOutput = processHTML(strInput, TimeStamp);
	} else if (bANSILog) {
		// keep ANSI codes
		strOutput = bRMASupport
			? processRMA(strInput, TimeStamp)
			: strInput;
	} else {
		// strip all Esc-sequences
		strOutput = processTEXT(strInput);
	}

	return strOutput;
}

/**
 * Stops writing to log file and close file handle
 */
void StopLogging()
{
    if (hLogFile.is_open()) {
        if ( bCurLogHTML ) {
			log(html_footer);
		}
		hLogFile.close();
    }

    for (int i = 0; i < MAX_OUTPUT; i++) {
        if (hOutputLogFile[i].is_open()) {	
			hOutputLogFile[i].close();
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
	int dumplines = 0;

    arg=get_arg_in_braces(arg, filename, STOP_SPACES);
    arg=get_arg_in_braces(arg, params,   STOP_SPACES);

	if (!strcmpi(params, "all")) {
		dumplines = nScrollSize;
		arg=get_arg_in_braces(arg, params, STOP_SPACES);
	} else if (is_all_digits(params)) {
		dumplines = atoi(params);
		arg=get_arg_in_braces(arg, params, STOP_SPACES);
	}

    if ( StartLog(-1, filename, params, dumplines) && mesvar[MSG_LOG]) {
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

    substitute_vars(arg,tmp, sizeof(tmp));
    substitute_myvars(tmp,msg, sizeof(msg));

	log(msg);
	log("\n");
	add_line_to_scrollbuffer(msg);
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
    arg=get_arg_in_braces(arg, left, STOP_SPACES);
    arg=get_arg_in_braces(arg, right, WITH_SPACES);

    if (!sscanf(number, "%d", &wnd) || wnd < 0 || wnd >= MAX_OUTPUT /*|| !*left*/) {
        tintin_puts2(rs::rs(1241));
        return;
    }
    if ( StartLog(wnd, left, right) && mesvar[MSG_LOG]) {
        char msg[BUFFER_SIZE], name[BUFFER_SIZE];
        GetOutputName(wnd, name, BUFFER_SIZE);
        sprintf(msg, rs::rs(1243), left, name);
        tintin_puts2(msg);
    }
}
