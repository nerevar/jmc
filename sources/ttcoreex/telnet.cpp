#include "stdafx.h"
#include <winsock.h>
#include "tintin.h"
#include "telnet.h"

#include "ttcoreex.h"
#include "JmcObj.h"

#include <vector>

const TelnetOption TelnetOptions[TELNET_OPTIONS_NUM] = {
	{TN_EOR_OPT,   "EOR",   "End-of-record"},						/* should be controlled by JMC */
	{TN_ECHO,      "ECHO",  "Server echoes user's input"},			/* should be controlled by JMC */
	{TN_NAWS,      "NAWS",  "Negotiate about window size"},			/* should be controlled by JMC */
	{TN_TTYPE,     "MTTS",  "MUD Terminal Type Standard"},			/* should be controlled by JMC */
	{TN_MSDP,      "MSDP",  "MUD Server Data Protocol"},			
	{TN_MSSP,      "MSSP",  "MUD Server Status Protocol"},			
	{TN_COMPRESS2, "MCCP",  "MUD Client Compression Protocol v2"},	/* should be controlled by JMC */
	{TN_COMPRESS,  "MCCP1", "MUD Client Compression Protocol v1"},	/* should be controlled by JMC */
	{TN_COMPRESS2, "MCCP2", "MUD Client Compression Protocol v2"},	/* should be controlled by JMC */
	{TN_MSP,       "MSP",   "MUD Sound Protocol"},
	{TN_MXP,       "MXP",   "MUD eXtension Protocol"},
	{TN_ATCP,      "ATCP",  "Achaea Telnet Client Protocol"},
	{TN_GMCP,      "GMCP",  "Generic MUD Communication Protocol"}
	};

std::vector<unsigned char> vEnabledTelnetOptions;
std::vector<char> SubnegotiationBuffer;

extern GET_WNDSIZE_FUNC GetWindowSize;
extern CComObject<CJmcObj>* pJmcObj;

int get_telnet_option_num(const char *name)
{
	int i;
	if(is_all_digits(name) && strlen(name) > 0)
		return atoi(name);
	for(i = 0; i < TELNET_OPTIONS_NUM; i++)
		if(is_abrev(name, TelnetOptions[i].Name))
			return TelnetOptions[i].Code;
	return -1;
}
void get_telnet_option_name(unsigned char num, char *buf)
{
	int i;
	for(i = 0; i < TELNET_OPTIONS_NUM; i++)
		if(TelnetOptions[i].Code == num) {
			strcpy(buf, TelnetOptions[i].Name);
			return;
		}
	sprintf(buf, "%d", num);
}
static void get_telnet_option_descr(unsigned char num, char *buf)
{
	int i;
	for(i = 0; i < TELNET_OPTIONS_NUM; i++)
		if(TelnetOptions[i].Code == num) {
			sprintf(buf, "%d[%s] [%s]",
				TelnetOptions[i].Code,
				TelnetOptions[i].Name,
				TelnetOptions[i].Descr);
			return;
		}
	sprintf(buf, "%d [unknown telnet option]", num);
}

BOOL telnet_option_enabled(unsigned char Option)
{
	int i;
	for(i = 0; i < vEnabledTelnetOptions.size(); i++)
		if(vEnabledTelnetOptions[i] == Option)
			return TRUE;
	return FALSE;
}

BOOL DLLEXPORT bTelnetDebugEnabled = FALSE;

unsigned char DLLEXPORT strPromptEndSequence[BUFFER_SIZE];
unsigned char DLLEXPORT strPromptEndReplace[BUFFER_SIZE];
BOOL DLLEXPORT bPromptEndEnabled = FALSE;

unsigned char State;
unsigned char CurrentSubnegotiation;
bool SentDoCompress2;
int MttsCounter;
unsigned int SocketFlags;
int LastWidthReported, LastHeightReported;

int PromptEndIndex;

z_stream mccp_stream;

void TelnetMsg(char *type, int cmd, int opt)
{
	static char cmdstr[32], optstr[256], buf[512];
	
	if (!mesvar[MSG_TELNET] || !bTelnetDebugEnabled)
		return;

	switch ((unsigned char)cmd) {
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

	case TN_SB:
		sprintf(cmdstr, "SB");
		break;
	case TN_SE:
		sprintf(cmdstr, "SE");
		break;
	

	case TN_EOR:
		sprintf(cmdstr, "EOR");
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
		sprintf(cmdstr, "%d", cmd);
	}

	if(opt)
		get_telnet_option_descr(opt, optstr);
	else
		strcpy(optstr, "");
	
	sprintf(buf, "#TELNET %s IAC:%s", type, cmdstr);
	if(strlen(optstr) > 0) {
		strcat(buf, ":");
		strcat(buf, optstr);
	}
	tintin_puts2(buf);
}

void SendCmd(int cmd, int opt)
{
	char szBuf[4], len = 0;
	szBuf[len++] = (char)TN_IAC;
	szBuf[len++] = (char)cmd;
	if (opt)
		szBuf[len++] = (char)opt;

	tls_send(MUDSocket, szBuf, len);
    
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

void telnet_command(char *arg) 
{
	char option[BUFFER_SIZE], flag[BUFFER_SIZE], tmp[BUFFER_SIZE];

	arg = get_arg_in_braces(arg, option, STOP_SPACES);
    arg = get_arg_in_braces(arg, flag, STOP_SPACES);

	if ( !strlen(option) ) {
		char options[BUFFER_SIZE];
		if(vEnabledTelnetOptions.size() == 0) {
			strcpy(options, "-");
		} else {
			strcpy(options, "");
			for(int i = 0; i < vEnabledTelnetOptions.size(); i++) {
				if(i > 0)
					strcat(options,", ");
				get_telnet_option_name(vEnabledTelnetOptions[i], tmp);
				strcat(options, tmp);
			}
		}
		sprintf(tmp, rs::rs(1266), options);
		tintin_puts2(tmp);
	} else if ( is_abrev(option, "debug") ) {
		if ( strlen(flag) ==0 )
			bTelnetDebugEnabled = !bTelnetDebugEnabled;
		else if ( !_strcmpi(flag, "on") )
			bTelnetDebugEnabled = TRUE;
		else
			bTelnetDebugEnabled = FALSE;

		sprintf(tmp, rs::rs(1267), "debug",
			bTelnetDebugEnabled ? "ON" : "OFF");
			tintin_puts2(tmp);
	} else {
		int opt = get_telnet_option_num(option);
		if(opt <= 0 || opt >= 255) {
			sprintf(tmp, rs::rs(1280), option);
			tintin_puts2(tmp);
		} else {
			if ( strlen(flag) ) {
				if ( !_strcmpi(flag, "on") ) {
					if ( !telnet_option_enabled(opt) )
						vEnabledTelnetOptions.push_back(opt);
				} else {
					for ( int i = 0; i < vEnabledTelnetOptions.size(); i++ )
						if ( vEnabledTelnetOptions[i] == opt ) {
							vEnabledTelnetOptions.erase(vEnabledTelnetOptions.begin() + i);
							break;
						}
				}
			}
			get_telnet_option_name(opt, option);
			sprintf(tmp, rs::rs(1267), 
				option,
				telnet_option_enabled(opt) ? "ON" : "OFF");
			tintin_puts2(tmp);
		}

	}
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

void free_telnet_buffer() {
	if (pInputData)
		free(pInputData);
	if (pDecompressedData)
		free(pDecompressedData);
	if (pOutputData)
		free(pOutputData);
	pInputData = pDecompressedData = pOutputData = NULL;
	InputCapacity = DecompressedCapacity = OutputCapacity = 0;
	InputSize = DecompressedSize = OutputSize = 0;
}

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
	SocketFlags = 0;
	SentDoCompress2 = false;
	MttsCounter = 0;
	LastWidthReported = LastHeightReported = 0;
	CurrentSubnegotiation = 0;
	InputSize = OutputSize = DecompressedSize = 0;
	State = '\0';
	SubnegotiationBuffer.clear();
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

void send_telnet_subnegotiation(unsigned char option, const char *output, int length)
{
	int i;
	vector<char> buf;

	buf.push_back((char)TN_IAC);
	buf.push_back((char)TN_SB);
	buf.push_back((char)option);
	for (i = 0; i < length; i++) {
		switch (output[i]) {
		case TN_IAC:
			buf.push_back((char)TN_IAC);
			buf.push_back((char)TN_IAC);
			break;
		case TN_SB:
		case TN_SE:
			break;
		default:
			buf.push_back((char)output[i]);
			break;
		}
	}
	buf.push_back((char)TN_IAC);
	buf.push_back((char)TN_SE);
	
	tls_send(MUDSocket, buf.begin(), buf.size());

	if (mesvar[MSG_TELNET]) {
		char buf[BUFFER_SIZE], optname[64];
		get_telnet_option_name(option, optname);
		sprintf(buf, "#TELNET SB-%s: send %d byte(s)", optname, length);
		tintin_puts(buf);
	}
}

void recv_telnet_subnegotiation(unsigned char option, const char *input, int length)
{
	USES_CONVERSION;

	if (mesvar[MSG_TELNET]) {
		char buf[BUFFER_SIZE], optname[64];
		get_telnet_option_name(option, optname);
		sprintf(buf, "#TELNET SB-%s: recv %d byte(s)", optname, length);
		tintin_puts(buf);
	}

	pJmcObj->m_pvarEventParams[0] = (input);
	pJmcObj->m_pvarEventParams[1] = ((LONG)option);
    BOOL bRet = pJmcObj->Fire_TelnetSE();
	if ( bRet ) {
    }
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
				State = input[*used];
 				switch(State) {
					case TN_GA:
						if (CurrentSubnegotiation > 0) {
							SubnegotiationBuffer.push_back(State);
						} else {
							//output[(*generated)++] = '\n';
							output[(*generated)++] = END_OF_PROMPT_MARK;
						}
						RecvCmd(State, 0);
						State = '\0';
						break;
					case TN_EOR:
						if(SocketFlags & SOCKEOR) {
							if (CurrentSubnegotiation > 0) {
								SubnegotiationBuffer.push_back(State);
							} else {
								output[(*generated)++] = END_OF_PROMPT_MARK;
							}
						}
						RecvCmd(State, 0);
						State = '\0';
						break;

					case TN_SB:
					case TN_WILL:
					case TN_WONT:
					case TN_DO:
					case TN_DONT:
						break;

					case TN_SE:
						RecvCmd(State, 0);
						State = '\0';
						SubnegotiationBuffer.push_back(0);
						recv_telnet_subnegotiation(CurrentSubnegotiation, SubnegotiationBuffer.begin(), SubnegotiationBuffer.size() - 1);
						SubnegotiationBuffer.pop_back();
						switch (CurrentSubnegotiation) {
						case TN_COMPRESS:
							(*used)++;
							CurrentSubnegotiation = 0;
							if (SubnegotiationBuffer.size() == 0) {
								start_decompressing();
								return;
							}
							break;
						case TN_COMPRESS2:
							(*used)++;
							CurrentSubnegotiation = 0;
							if (SubnegotiationBuffer.size() == 0) {
								start_decompressing();
								return;
							}
							break;
						case TN_TTYPE:
							if (SubnegotiationBuffer.size() == 1 && SubnegotiationBuffer[0] == MTTS_SEND) {
								char mtts_is[128];
								char *value = &(mtts_is[1]);
								mtts_is[0] = MTTS_IS;

								switch (MttsCounter) {
								case 0:
									sprintf(value, "%s", "JMC");
									MttsCounter++;
									break;
								case 1:
									sprintf(value, "%s", "ANSI");
									MttsCounter++;
									break;
								case 2:
									sprintf(value, "%s %d", "MTTS", 
										MTTS_ANSI);
									break;
								}
								send_telnet_subnegotiation(TN_TTYPE, mtts_is, strlen(value) + 1);
							}
							break;
						case TN_NAWS:
							break;
						}
						CurrentSubnegotiation = 0;
						break;

					case TN_IAC:
						if (CurrentSubnegotiation > 0) {
							SubnegotiationBuffer.push_back(State);
						} else {
							output[(*generated)++] = input[*used];
						}
						State = '\0';
						break;
					default:
						RecvCmd(State, 0);
						State = '\0';
						break;
				}
		    }
			break;
		case TN_WILL: {
				unsigned char opt = input[*used];
				RecvCmd(TN_WILL, opt);
				if (!telnet_option_enabled(opt)) {
					SendCmd(TN_DONT, opt);
				} else {
					switch(opt) {
					case TN_ECHO:
						SocketFlags |= SOCKECHO;
						SendCmd(TN_DO, TN_ECHO);
						break;
					case TN_TTYPE:
						MttsCounter = 0;
						SendCmd(TN_DO, TN_TTYPE);
						break;
					case TN_COMPRESS:
						if (SentDoCompress2)
							SendCmd(TN_DONT, TN_COMPRESS);
						else
							SendCmd(TN_DO, TN_COMPRESS);
						break;
					case TN_COMPRESS2:
						SendCmd(TN_DO, TN_COMPRESS2);
						SentDoCompress2 = true;
						break;
					default:
						SendCmd(TN_DO, opt);
						break;
					}
				}
				State = '\0';
			}
			break;
		case TN_WONT: {
				unsigned char opt = input[*used];
				RecvCmd(TN_WONT, opt);
				switch(input[*used]) {
				case TN_ECHO:
					SocketFlags &= ~SOCKECHO;
					SendCmd(TN_DONT, TN_ECHO);
					break;
				case TN_EOR_OPT:
					SocketFlags &= ~SOCKEOR;
					SendCmd(TN_DONT, TN_EOR);
					break;
				default:
					SendCmd(TN_DONT, opt);
					break;
				}
				State = '\0';
			}
			break;
		case TN_DO: {
				unsigned char opt = input[*used];
				RecvCmd(TN_DO, opt);
				if (!telnet_option_enabled(opt)) {
					SendCmd(TN_WONT, opt);
				} else {
					switch (opt) {
					case TN_ECHO:
						SocketFlags &= ~SOCKECHO;
						SendCmd(TN_WILL, TN_ECHO);
						break;
					case TN_NAWS:
						SocketFlags |= SOCKNAWS;
						SendCmd(TN_WILL, TN_NAWS);
						break;
					case TN_TTYPE:
						MttsCounter = 0;
						SendCmd(TN_WILL, TN_TTYPE);
						break;
					default:
						SendCmd(TN_WONT, opt);
						break;
					}
				}
				State = '\0';
			}
			break;
		case TN_DONT: {
				unsigned char opt = input[*used];
				RecvCmd(TN_DONT, opt);
				if (telnet_option_enabled(opt)) {
					switch (opt) {
					case TN_ECHO:
						SocketFlags |= SOCKECHO;
						SendCmd(TN_WONT, TN_ECHO);
						break;
					case TN_NAWS:
						SocketFlags &= ~SOCKNAWS;
						SendCmd(TN_WONT, TN_NAWS);
						break;
					default:
						SendCmd(TN_WONT, opt);
						break;
					}
				}
				State = '\0';
			}
			break;
		case TN_SB: {
				CurrentSubnegotiation = input[*used];
				SubnegotiationBuffer.clear();
				RecvCmd(TN_SB, CurrentSubnegotiation);
				State = '\0';
			}
			break;
		default:
			{
				unsigned char ch = input[*used];
				if (ch == TN_IAC) {
					if ( !(SocketFlags & SOCKTELNET) )
						SocketFlags |= SOCKTELNET;
					State = ch;
				} else if (CurrentSubnegotiation > 0) {
					if (CurrentSubnegotiation == TN_COMPRESS && ch == TN_WILL)
						State = TN_IAC;
					else
						SubnegotiationBuffer.push_back(ch);
				} else {
					switch (ch) {
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
									output[(*generated)++] = END_OF_PROMPT_MARK;
									PromptEndIndex = 0;
								}
								continue;
							}
							for (char *copy = (char*)strPromptEndSequence; PromptEndIndex; PromptEndIndex--)
								output[(*generated)++] = *(copy++);
						}

						if ((bIACReciveSingle || ch != 255) && 
						   (ch >= 0x80 || 
							ch == 0x1B || 
							ch == '\r' || 
							ch == '\n' ||
							isprint(ch))) {
							output[(*generated)++] = ch;
							if ((SocketFlags & SOCKNAWS) && (ch == '\n')) {
								unsigned char buf[4];
								int w, h;
								GetWindowSize(-1, w, h);
								if (w != LastWidthReported || h != LastHeightReported) {
									buf[0] = (w >> 8) & 0xFF;
									buf[1] = (w >> 0) & 0xFF;
									buf[2] = (h >> 8) & 0xFF;
									buf[3] = (h >> 0) & 0xFF;
									send_telnet_subnegotiation(TN_NAWS, (const char*)buf, 4);
									LastWidthReported = w;
									LastHeightReported = h;
								}
							}
						} else {
							State = ch;
						}
					}
				}
			}
		}
    }
}
 
