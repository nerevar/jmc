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

UINT DLLEXPORT LogCodePage = 0;

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

static wstring current_css_class = L"";

DWORD lastTicker = 0;
DWORD firstTicker = 0;

GET_OUTPUTNAME_FUNC GetOutputName;

//vls-begin// multiple output
void DLLEXPORT InitOutputNameFunc(GET_OUTPUTNAME_FUNC OutputNameFunc)
{
    GetOutputName = OutputNameFunc;
}
//vls-end//

void add_line_to_scrollbuffer(const wchar_t *line)
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
	pScrollLinesBuffer[ScrollBufferEnd].line = wstring(line);
	ScrollBufferEnd = (ScrollBufferEnd + 1) % ScrollBufferCapacity;
	if(ScrollBufferEnd == ScrollBufferBegin)
		ScrollBufferBegin = (ScrollBufferBegin + 1) % ScrollBufferCapacity;
}

void debug(wchar_t *pszFormat, ...)
{
    va_list marker;
    va_start(marker, pszFormat);
    wstring str = StrPrintfV(pszFormat, marker);
    va_end(marker);

	wofstream myfile;
	myfile.open ("debug.txt", ios::out | ios::app);
	
	myfile << str << L"\n";
	
	myfile.close();
}

void debug(wstring str)
{
	wofstream myfile;
	myfile.open ("debug.txt", ios::out | ios::app);
	
	myfile << str << L"\n";
	
	myfile.close();
}

static char *pLogBuffer = NULL;
static int LogBufSize = 0;
void log(wstring st)
{
	if (!hLogFile.is_open())
		return;
		
	//hLogFile << st;
	int cp = LogCodePage;
	if (!cp)
		cp = MudCodePage;
	int len = WideCharToMultiByte(cp, 0, st.c_str(), st.length(), NULL, 0, NULL, NULL);
	if (!pLogBuffer || LogBufSize < len) {
		LogBufSize = len*2;
		pLogBuffer = (char*)realloc(pLogBuffer, LogBufSize);
	}
	WideCharToMultiByte(cp, 0, st.c_str(), st.length(), pLogBuffer, LogBufSize, NULL, NULL);
	hLogFile.write(pLogBuffer, len);
}

void log(int wnd, wstring st)
{
	if (!(wnd >= 0)) {
		log(st);
		return;
	}

	if (!hOutputLogFile[wnd].is_open())
		return;
		
	//hOutputLogFile[wnd] << st;
	int cp = LogCodePage;
	if (!cp)
		cp = MudCodePage;
	int len = WideCharToMultiByte(cp, 0, st.c_str(), st.length(), NULL, 0, NULL, NULL);
	if (!pLogBuffer || LogBufSize < len) {
		LogBufSize = len*2;
		pLogBuffer = (char*)realloc(pLogBuffer, LogBufSize);
	}
	WideCharToMultiByte(cp, 0, st.c_str(), st.length(), pLogBuffer, LogBufSize, NULL, NULL);
	hOutputLogFile[wnd].write(pLogBuffer, len);
}

wstring loadHTMLFromResource(int name)
{
	DWORD size;
	HRSRC rc = ::FindResource(rs::hInst, MAKEINTRESOURCE(name), RT_HTML);
    HGLOBAL rcData = ::LoadResource(rs::hInst, rc);

    size = ::SizeofResource(rs::hInst, rc);
	wstring html_content(static_cast<const wchar_t*>(::LockResource(rcData)), size);

	return html_content;
}

wstring loadHTMLFromFile(const wchar_t *filename)
{
	USES_CONVERSION;

	wchar_t fn[MAX_PATH+2];
	wstring ret;

	MakeAbsolutePath(fn, filename, szBASE_DIR);

	wifstream t(W2A(fn));
	if(t.fail()) {
		wchar_t message[BUFFER_SIZE];
		swprintf(message,rs::rs(1262), fn);
        tintin_puts2(message);
		ret = loadHTMLFromResource(IDR_HTML_HEAD);
	} else {
		wstringstream buffer;
		buffer << t.rdbuf();
		ret = buffer.str();
	}

	return ret;
}

BOOL CloseMainLog(wchar_t *logName, BOOL logHTML)
{
    if (hLogFile.is_open()) { // Close log file now opened 
        if(mesvar[MSG_LOG]) {
			wchar_t message[BUFFER_SIZE];
			swprintf(message, rs::rs(1024), logName);
			tintin_puts2(message);
		}
        
		if ( logHTML ) {
			log(html_footer);
        }

		hLogFile.close();

		wcscpy(sLogName, L"");
		
		return TRUE;
    }

	return FALSE;
}

BOOL CloseWNDLog(int wnd, wchar_t *logName)
{
    if (hOutputLogFile[wnd].is_open()) { // Close log file now opened 
        if(mesvar[MSG_LOG]) {
			wchar_t message[BUFFER_SIZE];
			swprintf(message, rs::rs(1024), logName);
			tintin_puts2(message);
		}
        
		hOutputLogFile[wnd].close();

		wcscpy(sOutputLogName[wnd], L"");

		return TRUE;
    }

	return FALSE;
}

BOOL StartMainLog(wchar_t* logName, BOOL logMode, BOOL logHTML)
{
	USES_CONVERSION;

	lastTicker = firstTicker = 0;

	if (logMode && !logHTML)
		hLogFile.open(W2A(logName), ios::out | ios::binary | ios::app );
	else 
		hLogFile.open(W2A(logName), ios::out | ios::binary );

	if (!hLogFile.is_open())
		return FALSE;

    // Do HTML Log pereference
    if ( logHTML ) {
		wstring html_header = loadHTMLFromFile(L"html.log.template");

		wstring::size_type pos = 0;
		while ( ( pos = html_header.find(L"%title%", pos) ) != wstring::npos ) {
			html_header.replace(pos, 7, logName);
		}
		int cp = LogCodePage;
		if (!cp)
			cp = MudCodePage;
		if (CPNames.find(cp) != CPNames.end()) { //always should be true
			pos = 0;
			while ( ( pos = html_header.find(L"%charset%", pos) ) != wstring::npos ) {
				html_header.replace(pos, 9, CPNames[cp]);
			}
		}

		log(html_header);
		//log(TAG_OPEN);
		current_css_class = L"";
    }

	return TRUE;
}

BOOL StartWNDLog(int wnd, wchar_t* logName, BOOL logMode)
{
	USES_CONVERSION;

	if (logMode)
		hOutputLogFile[wnd].open(W2A(logName), ios::out | ios::binary | ios::app );
	else 
		hOutputLogFile[wnd].open(W2A(logName), ios::out | ios::binary );

	return hOutputLogFile[wnd] != NULL ? TRUE : FALSE;
}

//vls-begin// multiple output
BOOL StartLog(int wnd, wchar_t* left, wchar_t *right, int dumplines = 0)
{
	BOOL status;
	wchar_t *logName = wnd >= 0 ? sOutputLogName[wnd] : sLogName;

	SYSTEMTIME stl;
    wchar_t Timerecord[BUFFER_SIZE], logTitle[BUFFER_SIZE];
    BOOL bLogMode = bDefaultLogMode;

//* en	
	// do nothing on second command "#log <logname>", but overwrite or html
	if(!wcsicmp(left,logName) && wcsicmp(right, L"overwrite") && wcsicmp(right, L"html") )
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
	wcscpy(logName, left);

	// disable HTML mode for output windows
	if (!(wnd >= 0))
		bCurLogHTML = bHTML;
    
	if ( *right ) {
        if ( !wcsicmp(right, L"append") ) { 
			// set append mode 
            bLogMode = TRUE;
            if ( !(wnd >= 0) && bCurLogHTML ) 
                tintin_puts2(rs::rs(1026));
        } else if ( !wcsicmp(right, L"overwrite") ) { 
			// set overwrite mode 
            bLogMode = FALSE;
        } else if ( !wcsicmp(right, L"html") )  {
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
		wchar_t buff[128];
		swprintf(buff,rs::rs(1028), logName);
		tintin_puts2(buff);
		return FALSE;		
	}	

	if (bAppendLogTitle) {
		GetLocalTime(&stl);

		swprintf(logTitle, rs::rs(1258) , logName);
		log(wnd, logTitle);

		if (wnd >= 0)
			swprintf(Timerecord, rs::rs(1242) , wnd, stl.wDay, stl.wMonth , stl.wYear , stl.wHour, stl.wMinute);
		else
			swprintf(Timerecord, rs::rs(1029) , stl.wDay, stl.wMonth , stl.wYear , stl.wHour, stl.wMinute);
	    
		log(wnd, Timerecord);
	}

	if (dumplines > 0) {
		int i;
		for (i = ScrollBufferBegin; 
		     i != ScrollBufferEnd && dumplines > 0; 
			 i = (i + 1) % ScrollBufferCapacity, dumplines--) {
			log(processLine(pScrollLinesBuffer[i].line.c_str(), pScrollLinesBuffer[i].timestamp));
			log(L"\n");
		}
	}
	
    return TRUE;
}
//vls-end//

std::vector<int> processParams(wstring params)
{
	vector<int> paramsList;

	if (params == L"" || params == L"0") {
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

wstring getCSSClass(int param)
{
	// add css light color
	if (param == 1) 
		return CSS_LIGHT_COLOR;

	// set text color
	if (param >= 30 && param <= 37)
		return css_colors[ param - 30 ];

	if (param >= 40 && param <= 47)
		return css_bg_colors[ param - 40 ];

	return L"";
}

// remove RMA tags
void stripRMA(wstring &strInput) {
	wstring::size_type pos = 0;
    while ( ( pos = strInput.find(L"\x1Bp", pos) ) != wstring::npos ) {
		strInput.erase(pos, strInput.find('m', pos) - pos + 1);
    }
}

// remove [0m duplicates
void stripDefaultColorDuplicates(wstring &strInput) {
	wstring::size_type pos = 0;
    while ( ( pos = strInput.find(L"\x1B[0m\x1B[0m", pos) ) != wstring::npos ) {
		strInput.erase(pos, 4);
    }
}

wstring processHTML(wstring strInput, DWORD TimeStamp)
{
	wstring strOutput = L"";

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
			strOutput += strprintf(L"<div class=\"t %d\">", currTicker);
			lastTicker = currTicker;
			close_timestamp_tag = TRUE;
		}
	}
	
	wstring::size_type pos = 0, processed = 0;
	wstring::size_type posEscEnd, posSpace;
	wstring next_css_class = current_css_class, outputted_css_class = L"";

	BOOL tag_opened = FALSE;
	wstring text = L"";
	while (processed < strInput.length()) {
		pos = strInput.find(L"\x1B[", processed);

		if (pos != wstring::npos) {
			posEscEnd = strInput.find(L"m", pos);

			if (posEscEnd != wstring::npos) {
				posSpace = 0;

				wstring params = strInput.substr(pos + 2, posEscEnd - pos - 2);

				// replace spaces to ";"
				while ( ( posSpace = params.find(L" ", posSpace) ) != wstring::npos ) {
					params.replace(posSpace, 1, L";");
				}

				// parse params wstring to vector
				vector<int> paramsList = processParams(params);

				next_css_class = L"";
				// for each parameter add css class
				vector<int>::iterator itr;
				for(itr = paramsList.begin(); itr != paramsList.end(); itr++) {
					wstring newCSS = getCSSClass(*itr);

					if (newCSS.length() > 0) {
						if (next_css_class.length() > 0) (next_css_class += L" ");
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
					strOutput += wstring(L"<") + HTML_TAG + wstring(L" class=\"") + current_css_class + wstring(L"\"") + wstring(L">");
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
		strOutput += L"</div>";
	}

	return strOutput;
}

wstring processRMA(wstring strInput, DWORD TimeStamp)
{
	wstring strOutput;
	DWORD currTicker = 0;

	if ( !TimeStamp )
		TimeStamp = GetTickCount();

    if ( lastTicker == 0 ) 
        lastTicker = TimeStamp;

	currTicker = TimeStamp;

    if ( currTicker - lastTicker ) {
		strOutput = strprintf(L"%lcp:%dm", ANSI_COMMAND_CHAR, currTicker - lastTicker);
		strOutput += strInput;
    } else {
		strOutput = strInput;
	}

	lastTicker = currTicker;

	return strOutput;
}

wstring processTEXT(wstring strInput)
{
	// delete everything from 0x1B to 'm'
	wstring strOutput = strInput;
	int escPos = 0,
		endEscPos = 0;

	while ( (escPos = strOutput.find(L'\x1B', escPos)) != wstring::npos) {
		if ( (endEscPos = strOutput.find(L'm', escPos)) == wstring::npos ) {
			//incorrect escape sequence, erase entire wstring
			strOutput.erase(escPos);
		} else {
			strOutput.erase(escPos, endEscPos - escPos + 1);
		}
	}

	return strOutput;
}


wstring processLine(const wchar_t *charInput, DWORD TimeStamp)
{
	wstring strInput(charInput), strOutput;
	
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
void log_command(wchar_t *arg)
{
    wchar_t filename[BUFFER_SIZE], params[BUFFER_SIZE];
	int dumplines = 0;

    arg=get_arg_in_braces(arg,filename,STOP_SPACES,sizeof(filename)/sizeof(wchar_t)-1);
    arg=get_arg_in_braces(arg,params,STOP_SPACES,sizeof(params)/sizeof(wchar_t)-1);

	if (!wcsicmp(params, L"all")) {
		dumplines = nScrollSize;
		arg=get_arg_in_braces(arg,params,STOP_SPACES,sizeof(params)/sizeof(wchar_t)-1);
	} else if (is_all_digits(params)) {
		dumplines = _wtoi(params);
		arg=get_arg_in_braces(arg,params,STOP_SPACES,sizeof(params)/sizeof(wchar_t)-1);
	}

    if ( StartLog(-1, filename, params, dumplines) && mesvar[MSG_LOG]) {
        wchar_t msg[BUFFER_SIZE];
        swprintf(msg, rs::rs(1030), filename);
        tintin_puts2(msg);
    }
}


// Declare JMC commands


/***********************/
/* the #logadd command */
/***********************/
void logadd_command(wchar_t *arg)
{
    wchar_t msg[BUFFER_SIZE];
	wchar_t tmp[BUFFER_SIZE];

	get_arg_in_braces(arg,msg,WITH_SPACES,sizeof(msg)/sizeof(wchar_t)-1);

    substitute_vars(msg,tmp, sizeof(tmp)/sizeof(wchar_t));
    substitute_myvars(tmp,msg, sizeof(msg)/sizeof(wchar_t));

	log(msg);
	log(L"\n");
	add_line_to_scrollbuffer(msg);
}

/************************/
/* the #logpass command */
/************************/
void logpass_command(wchar_t *arg)
{
    bLogPassedLine = TRUE;
}
//vls-end//


/********************/
/* the #wlog command */
/********************/
void wlog_command(wchar_t *arg)
{
    wchar_t left[BUFFER_SIZE], right[BUFFER_SIZE], number[BUFFER_SIZE];
    int wnd;

    arg=get_arg_in_braces(arg,number,STOP_SPACES,sizeof(number)/sizeof(wchar_t)-1);
    arg=get_arg_in_braces(arg,left,STOP_SPACES,sizeof(left)/sizeof(wchar_t)-1);
    arg=get_arg_in_braces(arg,right,WITH_SPACES,sizeof(right)/sizeof(wchar_t)-1);

    if (!swscanf(number, L"%d", &wnd) || wnd < 0 || wnd >= MAX_OUTPUT /*|| !*left*/) {
        tintin_puts2(rs::rs(1241));
        return;
    }
    if ( StartLog(wnd, left, right) && mesvar[MSG_LOG]) {
        wchar_t msg[BUFFER_SIZE], name[BUFFER_SIZE];
        GetOutputName(wnd, name, sizeof(name) / sizeof(wchar_t));
        swprintf(msg, rs::rs(1243), left, name);
        tintin_puts2(msg);
    }
}
