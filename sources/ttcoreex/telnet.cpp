#include "stdafx.h"
#include "winsock.h"
#include "tintin.h"
#include "telnet.h"

BOOL DLLEXPORT bMCCPEnabled = FALSE;
BOOL DLLEXPORT bTelnetIACs = FALSE;
unsigned char DLLEXPORT strPromptEndSequence[BUFFER_SIZE];
unsigned char DLLEXPORT strPromptEndReplace[BUFFER_SIZE];
BOOL DLLEXPORT bPromptEndEnabled = FALSE;

unsigned char State;
unsigned char LastSB;
bool SentDoCompress2;
int WaitBuggyMccp1;
int SocketFlags;

int PromptEndIndex;

z_stream mccp_stream;

void TelnetMsg(char *type, int cmd, int opt)
{
	static char cmdstr[32], optstr[32], buf[128];
	
	if (!bTelnetIACs)
		return;

	switch (cmd) {
	case TN_SB:
		sprintf(cmdstr, "SB");
		break;
	case TN_WILL:
		sprintf(cmdstr, "WILL");
		break;
	case TN_WONT:
		sprintf(cmdstr, "WONT");
		break;
	case TN_DO:
		sprintf(cmdstr, "DO");
		break;
	case TN_DONT:
		sprintf(cmdstr, "DONT");
		break;

	case TN_EOR:
		sprintf(cmdstr, "EOR");
		break;
	case TN_SE:
		sprintf(cmdstr, "SE");
		break;
	case TN_NOP:
		sprintf(cmdstr, "NOP");
		break;
	case TN_DATA_MARK:
		sprintf(cmdstr, "DATA_MARK");
		break;
	case TN_BRK:
		sprintf(cmdstr, "BRK");
		break;
	case TN_IP:
		sprintf(cmdstr, "IP");
		break;
	case TN_AO:
		sprintf(cmdstr, "AO");
		break;
	case TN_AYT:
		sprintf(cmdstr, "AYT");
		break;
	case TN_EC:
		sprintf(cmdstr, "EC");
		break;
	case TN_EL:
		sprintf(cmdstr, "EL");
		break;
	case TN_GA:
		sprintf(cmdstr, "GA");
		break;

	default:
		sprintf(cmdstr, "%04o", cmd);
	}

	switch (opt) {
	case 0:
		sprintf(optstr, "");
		break;
	case TN_ECHO:
		sprintf(optstr, "ECHO");
		break;
	case TN_SGA:
		sprintf(optstr, "SGA");
		break;
	case TN_STATUS:
		sprintf(optstr, "STATUS");
		break;
	case TN_TIMING_MARK:
		sprintf(optstr, "TIMING_MARK");
		break;
	case TN_TTYPE:
		sprintf(optstr, "TTYPE");
		break;
	case TN_EOR_OPT:
		sprintf(optstr, "EOR_OPT");
		break;
	case TN_NAWS:
		sprintf(optstr, "NAWS");
		break;
	case TN_TSPEED:
		sprintf(optstr, "TSPEED");
		break;
	case TN_LINEMODE:
		sprintf(optstr, "LINEMODE");
		break;
	case TN_COMPRESS:
		sprintf(optstr, "COMPRESS");
		break;
	case TN_COMPRESS2:
		sprintf(optstr, "COMPRESS2");
		break;
	default:
		sprintf(optstr, "%04o", opt);
	}

	sprintf(buf, "#TELNET %s IAC:%s:%s", type, cmdstr, optstr);
	tintin_puts2(buf);
}

void SendCmd(int cmd, int opt)
{
	char szBuf[4];
	wsprintf(szBuf, "%c%c%c", (char)(unsigned char)TN_IAC, (char)cmd, (char)opt);
    send(MUDSocket,szBuf, 3, 0 );

	TelnetMsg("send", cmd, opt);
}

static void RecvCmd(int cmd, int opt)
{
	TelnetMsg("recv", cmd, opt);
}


#ifdef _DEBUG_LOG
extern DLLEXPORT HANDLE hExLog;
#endif

static void start_decompressing()
{
	mccp_stream.zalloc = Z_NULL;
    mccp_stream.zfree = Z_NULL;
    mccp_stream.opaque = Z_NULL;
    mccp_stream.avail_in = 0;
    mccp_stream.next_in = Z_NULL;
    int ret = inflateInit(&mccp_stream);
    if (ret != Z_OK) {
        //what should we do?
	}
	SocketFlags |= SOCKCOMPRESSING;
}
static void stop_decompressing()
{
	if(SocketFlags & SOCKCOMPRESSING) {
		inflateEnd(&mccp_stream);
		SocketFlags &= ~SOCKCOMPRESSING;
	}
}
static void do_decompressing(const char *src, int size, int *used, char *dst, int capacity, int *decompressed)
{
	mccp_stream.avail_in = size;
	mccp_stream.next_in = (unsigned char*)src;
	mccp_stream.avail_out = capacity;
    mccp_stream.next_out = (unsigned char*)dst;

    int ret = inflate(&mccp_stream, Z_PARTIAL_FLUSH);
    switch (ret) {
		case Z_STREAM_ERROR:
        case Z_NEED_DICT:
        case Z_DATA_ERROR:
        case Z_MEM_ERROR:
			ret = Z_DATA_ERROR;
			break;
    }

	*used = size - mccp_stream.avail_in;
	*decompressed = capacity - mccp_stream.avail_out;

	if (ret != Z_OK)
		stop_decompressing();

}

void mccp_command(char *arg) 
{
	char flag[BUFFER_SIZE];
    arg = get_arg_in_braces(arg,flag,WITH_SPACES);

    if ( *flag == 0 ) {
        bMCCPEnabled =! bMCCPEnabled;
    } else {
        if ( !_strcmpi(flag, "on" ) )
            bMCCPEnabled = TRUE;
        else 
            bMCCPEnabled = FALSE;
    }
  
    if(bMCCPEnabled)
        tintin_puts2(rs::rs(1266));
    else
        tintin_puts2(rs::rs(1267));
}

static char *last_strstr(char *haystack, char *needle)
{
    if (*needle == '\0')
        return haystack;

    char *result = NULL;
    for (;;) {
        char *p = strstr(haystack, needle);
        if (p == NULL)
            break;
        result = p;
        haystack = p + 1;
    }

    return result;
}

void promptend_command(char *arg) 
{
	char left[BUFFER_SIZE], right[BUFFER_SIZE];
	char buff[BUFFER_SIZE];

    arg = get_arg_in_braces(arg, left, STOP_SPACES);

	if ( *left == 0 ) {
		if (!bPromptEndEnabled)
			sprintf(buff, rs::rs(1278));
		else
			sprintf(buff, rs::rs(1279), strPromptEndSequence, strPromptEndReplace);
	} else if (is_abrev(left, "disable")) {
		bPromptEndEnabled = FALSE;
		sprintf(buff, rs::rs(1278));
	} else {
		if (*left) {
			arg = get_arg_in_braces(arg, right, STOP_SPACES);
			strcpy((char*)strPromptEndSequence, left);
			strcpy((char*)strPromptEndReplace, right);
			bPromptEndEnabled = TRUE;
			sprintf(buff, rs::rs(1279), strPromptEndSequence, strPromptEndReplace);
		}
	}
	tintin_puts2(buff);
}

char *pInputData = NULL;
char *pDecompressedData = NULL;
char *pOutputData = NULL;
int InputCapacity = 0;
int DecompressedCapacity = 0;
int OutputCapacity = 0;
int InputSize = 0;
int DecompressedSize = 0;
int OutputSize = 0;

void increase_capacity(char **ppBuf, int *capacity, int needed) {
	if (needed > *capacity) {
		int newcap = needed + BUFFER_SIZE;
		char *newptr = (char*)realloc(*ppBuf, newcap);
		if (newptr) {
			*capacity = newcap;
			*ppBuf = newptr;
		}
	}
}

void reset_telnet_protocol()
{
	stop_decompressing();
	SocketFlags = 0; //SOCKECHO;
	SentDoCompress2 = false;
	WaitBuggyMccp1 = 0;
	LastSB = 0;
	InputSize = OutputSize = DecompressedSize = 0;
	State = '\0';
	PromptEndIndex = 0;
}

void telnet_push_back(const char *src, int size)
{
	increase_capacity(&pInputData, &InputCapacity, InputSize + size);
	if (InputSize + size > InputCapacity) {
		//not enough memory!
		size = InputCapacity - InputSize;
	}
	if (size > 0) {
		memcpy(pInputData + InputSize, src, size);
		InputSize += size;
	}
}

int telnet_more_coming() {
	if (InputSize > 0 || ((SocketFlags & SOCKCOMPRESSING) && DecompressedSize > 0) || OutputSize > 0)
		return 1;
	return 0;
}

int telnet_pop_front(char *dst, int maxsize)
{
	int ret = 0;
	increase_capacity(&pOutputData, &OutputCapacity, maxsize);
	if (OutputSize < OutputCapacity && (InputSize > 0 || OutputSize >0)) {
		int input_processed = 0, output_generated = 0;

		if (InputSize > 0) {
			if (SocketFlags & SOCKCOMPRESSING) {
				increase_capacity(&pDecompressedData, &DecompressedCapacity, DecompressedSize + maxsize);

				int decompressed = 0;
				do_decompressing((const char*)pInputData, InputSize, &input_processed, 
								 (char*)pDecompressedData + DecompressedSize, DecompressedCapacity - DecompressedSize, &decompressed);
				DecompressedSize += decompressed;
				
				int processed = 0;
				do_telnet_protecol((const unsigned char*)pDecompressedData, DecompressedSize, &processed,
								   (unsigned char*)pOutputData + OutputSize, OutputCapacity - OutputSize, &output_generated);

				if (processed < DecompressedSize)
					memmove(pDecompressedData, pDecompressedData + processed, DecompressedSize - processed);
				DecompressedSize -= processed;
			} else {
				do_telnet_protecol((const unsigned char*)pInputData, InputSize, &input_processed,
								   (unsigned char*)pOutputData + OutputSize, OutputCapacity - OutputSize, &output_generated);
			}
		}
		
		if (input_processed < InputSize)
			memmove(pInputData, pInputData + input_processed, InputSize - input_processed);
		InputSize -= input_processed;
		
		OutputSize += output_generated;
		int copy_out = (OutputSize > maxsize) ? maxsize : OutputSize;
		memcpy(dst, pOutputData, copy_out);
		if (copy_out < OutputSize)
			memmove(pOutputData, pOutputData + copy_out, OutputSize - copy_out);
		OutputSize -= copy_out;
		ret += copy_out;
	}
	return ret;
}

void do_telnet_protecol(const unsigned char* input, int length, int *used, unsigned char* output, int capacity, int *generated)
{
    *used = *generated = 0;
#ifdef _DEBUG_LOG
	if (hExLog ) {
		char exLogText[128];
        DWORD Written;
        sprintf(exLogText , "\r\n#telnet state:%d#\r\n", State);
        WriteFile(hExLog , exLogText , strlen(exLogText) , &Written, NULL);
        //WriteFile(hExLog , buffer , didget , &Written, NULL);
    }
#endif

    for (*used = 0; *used < length && (*generated) < capacity; (*used)++) {
		switch( State ) {
		case TN_IAC: {
 				switch(State = input[*used]) {
					case TN_GA:
					case TN_EOR:
						TelnetMsg("recv", State, 0);
						//output[(*generated)++] = '\n';
						output[(*generated)++] = 0x1;
						State = '\0';
						break;

					case TN_SB:
					case TN_WILL:
					case TN_WONT:
					case TN_DO:
					case TN_DONT:
						break;

					case TN_SE:
						TelnetMsg("recv", State, 0);
						State = '\0';
						if(LastSB == TN_COMPRESS2) {
							(*used)++;
							start_decompressing();
							return;
						}
						break;

					case TN_IAC:
						output[(*generated)++] = input[*used];
						State = '\0';
						break;
					default:
						State = '\0';
						break;
				}
		    }
			break;
		case TN_WILL: {
				RecvCmd(TN_WILL, input[*used]);
				switch(input[*used]) {
				case TN_ECHO:
					if( SocketFlags & SOCKECHO ){
						SocketFlags &= ~SOCKECHO;
						SendCmd(TN_DONT, TN_ECHO); // !!! DID DO 
					}
					break;
				case TN_EOR_OPT:
					if( ! (SocketFlags & SOCKEOR) ){
						SocketFlags |= SOCKEOR;
						SendCmd(TN_DO, TN_EOR_OPT);
					}
					break;
				case TN_TSPEED:
					SendCmd(TN_DONT, TN_TSPEED);
					break;
				case TN_COMPRESS:
					if (!bMCCPEnabled || SentDoCompress2)
						SendCmd(TN_DONT, TN_COMPRESS);
					else
						SendCmd(TN_DO, TN_COMPRESS);
					break;
				case TN_COMPRESS2:
					if (!bMCCPEnabled) {
						SendCmd(TN_DONT, TN_COMPRESS2);
					} else {
						SendCmd(TN_DO, TN_COMPRESS2);
						SentDoCompress2 = true;
					}
					break;
				default:
					SendCmd(TN_DONT, input[*used]);
					break;
				}
				State = '\0';
			}
			break;
		case TN_WONT: {
				RecvCmd(TN_WONT, input[*used]);
				switch(input[*used]) {
				case TN_ECHO:
					if( ! (SocketFlags & SOCKECHO) ){
						SocketFlags |= SOCKECHO;
						SendCmd(TN_DONT, TN_ECHO);
					}
					break;
				case TN_EOR_OPT:
					if( SocketFlags & SOCKEOR ){
						SocketFlags &= ~SOCKEOR;
						SendCmd(TN_DONT, TN_EOR);
					}
					break;
				default:
					break;
				}
				State = '\0';
			}
			break;
		case TN_DO: {
				RecvCmd(TN_DO, input[*used]);
				SendCmd(TN_WONT, input[*used]);
				State = '\0';
			}
			break;
		case TN_DONT: {
				RecvCmd(TN_DONT, input[*used]);
				State = '\0';
			}
			break;
		case TN_SB: {
				RecvCmd(TN_SB, input[*used]);
				if( (LastSB = input[*used]) == TN_COMPRESS ) {
					WaitBuggyMccp1 = 1;
				}
				State = '\0';
			}
			break;
		default:
			if (WaitBuggyMccp1 == 1 && input[*used] == TN_WILL) {
				WaitBuggyMccp1++;
				continue;
			}
			if (WaitBuggyMccp1 == 2 && input[*used] == TN_SE) {
				start_decompressing();
				return;
			}
			WaitBuggyMccp1 = 0;
			switch (input[*used]) {
			case TN_IAC:
				if ( !(SocketFlags & SOCKTELNET) )
					SocketFlags |= SOCKTELNET;
				State = input[*used];
				break;
			case 0x9:
				if(capacity - (*generated) >= 4) {
					output[(*generated)++] = ' ';
					output[(*generated)++] = ' ';
					output[(*generated)++] = ' ';
					output[(*generated)++] = ' ';
				} else {
					return;
				}
				break;
			case 0x7:
				MessageBeep(MB_OK);
				break;
			default:
				if (bPromptEndEnabled) {
					if (input[*used] == strPromptEndSequence[PromptEndIndex]) {
						PromptEndIndex++;
						if (!strPromptEndSequence[PromptEndIndex]) { //match!
							for (char *copy = (char*)strPromptEndReplace; *copy; )
								output[(*generated)++] = *(copy++);
							output[(*generated)++] = 0x1;
							PromptEndIndex = 0;
						}
						continue;
					}
					for (char *copy = (char*)strPromptEndSequence; PromptEndIndex; PromptEndIndex--)
						output[(*generated)++] = *(copy++);
				}

				if ((bIACReciveSingle || input[*used] != 255) && 
				   (input[*used] >= 0x80 || 
					input[*used] == 0x1B || 
					input[*used] == '\r' || 
					input[*used] == '\n' ||
					isprint(input[*used]))) {
					output[(*generated)++] = input[*used];
				} else {
					State = input[*used];
				}
			}
		}
    }
}
 
