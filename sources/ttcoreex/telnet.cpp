#include "stdafx.h"
#include <winsock.h>
#include "tintin.h"
#include "telnet.h"

#include "ttcoreex.h"
#include "JmcObj.h"

#include <vector>

const TelnetOption TelnetOptions[TELNET_OPTIONS_NUM] = {
	{TN_EOR_OPT,   L"EOR",   L"End-of-record"},						/* should be controlled by JMC */
	{TN_ECHO,      L"ECHO",  L"Server echoes user's input"},			/* should be controlled by JMC */
	{TN_NAWS,      L"NAWS",  L"Negotiate about window size"},			/* should be controlled by JMC */
	{TN_TTYPE,     L"MTTS",  L"MUD Terminal Type Standard"},			/* should be controlled by JMC */
	{TN_MSDP,      L"MSDP",  L"MUD Server Data Protocol"},			
	{TN_MSSP,      L"MSSP",  L"MUD Server Status Protocol"},			
	{TN_COMPRESS2, L"MCCP",  L"MUD Client Compression Protocol v2"},	/* should be controlled by JMC */
	{TN_COMPRESS,  L"MCCP1", L"MUD Client Compression Protocol v1"},	/* should be controlled by JMC */
	{TN_COMPRESS2, L"MCCP2", L"MUD Client Compression Protocol v2"},	/* should be controlled by JMC */
	{TN_MSP,       L"MSP",   L"MUD Sound Protocol"},
	{TN_MXP,       L"MXP",   L"MUD eXtension Protocol"},
	{TN_ATCP,      L"ATCP",  L"Achaea Telnet Client Protocol"},
	{TN_GMCP,      L"GMCP",  L"Generic MUD Communication Protocol"}
	};

std::vector<unsigned char> vEnabledTelnetOptions;
std::vector<char> SubnegotiationBuffer;

extern GET_WNDSIZE_FUNC GetWindowSize;
extern CComObject<CJmcObj>* pJmcObj;

int get_telnet_option_num(const wchar_t *name)
{
	int i;
	if(is_all_digits(name) && wcslen(name) > 0)
		return _wtoi(name);
	for(i = 0; i < TELNET_OPTIONS_NUM; i++)
		if(is_abrev(name, TelnetOptions[i].Name))
			return TelnetOptions[i].Code;
	return -1;
}
void get_telnet_option_name(unsigned char num, wchar_t *buf)
{
	int i;
	for(i = 0; i < TELNET_OPTIONS_NUM; i++)
		if(TelnetOptions[i].Code == num) {
			wcscpy(buf, TelnetOptions[i].Name);
			return;
		}
	swprintf(buf, L"%d", num);
}
static void get_telnet_option_descr(unsigned char num, wchar_t *buf)
{
	int i;
	for(i = 0; i < TELNET_OPTIONS_NUM; i++)
		if(TelnetOptions[i].Code == num) {
			swprintf(buf, L"%d[%s] [%ls]",
				TelnetOptions[i].Code,
				TelnetOptions[i].Name,
				TelnetOptions[i].Descr);
			return;
		}
	swprintf(buf, L"%d [unknown telnet option]", num);
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

wchar_t DLLEXPORT strPromptEndSequence[BUFFER_SIZE];
wchar_t DLLEXPORT strPromptEndReplace[BUFFER_SIZE];

unsigned char strPromptEndSeqBytes[BUFFER_SIZE];
unsigned char strPromptEndReplBytes[BUFFER_SIZE];

BOOL DLLEXPORT bPromptEndEnabled = FALSE;

unsigned char State;
unsigned char CurrentSubnegotiation;
bool SentDoCompress2;
int MttsCounter;
unsigned int SocketFlags;
int LastWidthReported, LastHeightReported;

int PromptEndIndex;

z_stream mccp_stream;

void TelnetMsg(wchar_t *type, int cmd, int opt)
{
	static wchar_t cmdstr[32], optstr[256], buf[512];
	
	if (!mesvar[MSG_TELNET] || !bTelnetDebugEnabled)
		return;

	switch ((unsigned char)cmd) {
	case TN_WILL:
		swprintf(cmdstr, L"WILL");
		break;
	case TN_WONT:
		swprintf(cmdstr, L"WONT");
		break;
	case TN_DO:
		swprintf(cmdstr, L"DO");
		break;
	case TN_DONT:
		swprintf(cmdstr, L"DONT");
		break;

	case TN_SB:
		swprintf(cmdstr, L"SB");
		break;
	case TN_SE:
		swprintf(cmdstr, L"SE");
		break;
	

	case TN_EOR:
		swprintf(cmdstr, L"EOR");
		break;
	
	case TN_NOP:
		swprintf(cmdstr, L"NOP");
		break;
	case TN_DATA_MARK:
		swprintf(cmdstr, L"DATA_MARK");
		break;
	case TN_BRK:
		swprintf(cmdstr, L"BRK");
		break;
	case TN_IP:
		swprintf(cmdstr, L"IP");
		break;
	case TN_AO:
		swprintf(cmdstr, L"AO");
		break;
	case TN_AYT:
		swprintf(cmdstr, L"AYT");
		break;
	case TN_EC:
		swprintf(cmdstr, L"EC");
		break;
	case TN_EL:
		swprintf(cmdstr, L"EL");
		break;
	case TN_GA:
		swprintf(cmdstr, L"GA");
		break;

	default:
		swprintf(cmdstr, L"%d", cmd);
	}

	if(opt)
		get_telnet_option_descr(opt, optstr);
	else
		wcscpy(optstr, L"");
	
	swprintf(buf, L"#TELNET %ls IAC:%ls", type, cmdstr);
	if(wcslen(optstr) > 0) {
		wcscat(buf, L":");
		wcscat(buf, optstr);
	}
	tintin_puts2(buf);
}

void send_telnet_command(unsigned char cmd, unsigned char opt)
{
	char szBuf[4], len = 0;
	szBuf[len++] = (char)TN_IAC;
	szBuf[len++] = (char)cmd;
	if (opt)
		szBuf[len++] = (char)opt;

	tls_send(MUDSocket, szBuf, len);
    
	TelnetMsg(L"send", cmd, opt);

	if (cmd != TN_SE && cmd != TN_SB) {
		pJmcObj->m_pvarEventParams[0] = ((LONG)cmd);
		pJmcObj->m_pvarEventParams[1] = ((LONG)opt);
		pJmcObj->m_pvarEventParams[2].Clear();
		
		BOOL bRet = pJmcObj->Fire_Telnet();
		if ( bRet ) {
		}
	}
}

static void RecvCmd(int cmd, int opt)
{
	TelnetMsg(L"recv", cmd, opt);

	if (cmd != TN_SE && cmd != TN_SB) {
		pJmcObj->m_pvarEventParams[0] = ((LONG)cmd);
		pJmcObj->m_pvarEventParams[1] = ((LONG)opt);
		pJmcObj->m_pvarEventParams[2].Clear();
		
		BOOL bRet = pJmcObj->Fire_Telnet();
		if ( bRet ) {
		}
	}
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

void telnet_command(wchar_t *arg) 
{
	wchar_t option[BUFFER_SIZE], flag[BUFFER_SIZE], tmp[BUFFER_SIZE];

	arg = get_arg_in_braces(arg,option,STOP_SPACES,sizeof(option)/sizeof(wchar_t)-1);
    arg = get_arg_in_braces(arg,flag,STOP_SPACES,sizeof(flag)/sizeof(wchar_t)-1);

	if ( !wcslen(option) ) {
		wchar_t options[BUFFER_SIZE];
		if(vEnabledTelnetOptions.size() == 0) {
			wcscpy(options, L"-");
		} else {
			wcscpy(options, L"");
			for(int i = 0; i < vEnabledTelnetOptions.size(); i++) {
				if(i > 0)
					wcscat(options,L", ");
				get_telnet_option_name(vEnabledTelnetOptions[i], tmp);
				wcscat(options, tmp);
			}
		}
		swprintf(tmp, rs::rs(1266), options);
		tintin_puts2(tmp);
	} else if ( is_abrev(option, L"debug") ) {
		if ( wcslen(flag) ==0 )
			bTelnetDebugEnabled = !bTelnetDebugEnabled;
		else if ( !wcsicmp(flag, L"on") )
			bTelnetDebugEnabled = TRUE;
		else
			bTelnetDebugEnabled = FALSE;

		swprintf(tmp, rs::rs(1267), L"debug",
			bTelnetDebugEnabled ? L"ON" : L"OFF");
			tintin_puts2(tmp);
	} else {
		int opt = get_telnet_option_num(option);
		if(opt <= 0 || opt >= 255) {
			swprintf(tmp, rs::rs(1280), option);
			tintin_puts2(tmp);
		} else {
			if ( wcslen(flag) ) {
				if ( !wcsicmp(flag, L"on") ) {
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
			swprintf(tmp, rs::rs(1267), 
				option,
				telnet_option_enabled(opt) ? L"ON" : L"OFF");
			tintin_puts2(tmp);
		}

	}
}

void promptend_command(wchar_t *arg) 
{
	wchar_t left[BUFFER_SIZE], right[BUFFER_SIZE];
	wchar_t buff[BUFFER_SIZE];

    arg = get_arg_in_braces(arg,left,STOP_SPACES,sizeof(left)/sizeof(wchar_t)-1);

	if ( *left == 0 ) {
		if (!bPromptEndEnabled)
			swprintf(buff, rs::rs(1278));
		else
			swprintf(buff, rs::rs(1279), strPromptEndSequence, strPromptEndReplace);
	} else if (is_abrev(left, L"disable")) {
		bPromptEndEnabled = FALSE;
		swprintf(buff, rs::rs(1278));
	} else {
		if (*left) {
			arg = get_arg_in_braces(arg,right,STOP_SPACES,sizeof(right)/sizeof(wchar_t)-1);
			
			wcscpy(strPromptEndSequence, left);
			wcscpy(strPromptEndReplace, right);

			WideCharToMultiByte(MudCodePage, 0, strPromptEndSequence, wcslen(strPromptEndSequence), 
				(char*)strPromptEndSeqBytes, sizeof(strPromptEndSeqBytes), NULL, NULL);
			WideCharToMultiByte(MudCodePage, 0, strPromptEndReplace, wcslen(strPromptEndReplace), 
				(char*)strPromptEndReplBytes, sizeof(strPromptEndReplBytes), NULL, NULL);
			
			bPromptEndEnabled = TRUE;
			swprintf(buff, rs::rs(1279), strPromptEndSequence, strPromptEndReplace);
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

int telnet_pop_front(wchar_t *dst, int maxsize)
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
				do_telnet_protecol((const char*)pDecompressedData, DecompressedSize, &processed,
								   (char*)pOutputData + OutputSize, OutputCapacity - OutputSize, &output_generated);

				if (processed < DecompressedSize)
					memmove(pDecompressedData, pDecompressedData + processed, DecompressedSize - processed);
				DecompressedSize -= processed;
			} else {
				do_telnet_protecol((const char*)pInputData, InputSize, &input_processed,
								   (char*)pOutputData + OutputSize, OutputCapacity - OutputSize, &output_generated);
			}
		}
		
		if (input_processed < InputSize)
			memmove(pInputData, pInputData + input_processed, InputSize - input_processed);
		InputSize -= input_processed;
		
		OutputSize += output_generated;
		
		static DWORD mode = 0;
		/*
		  MultiByteToWideChar(CodePage, 0, pOutputData, OutputSize, dst, maxsize);
		    -- doesn't work correctly with incomplete multibyte sequences
		  memcpy(dst, pOutputData, copy_out); 
		    -- old ANSI line
		  */
		int len = maxsize;
		int processed = min(maxsize, OutputSize);
		int retConv;

		/* Codepages 1200 (UTF-16LE) and 1201 (UTF-16BE) doesn't supported for unmanager applications
		   though they can be very simply implemented for almost(!) all situations
		 */
		if (MudCodePage == 1200) {
			int count = min(len, processed) / sizeof(wchar_t);
			memcpy(dst, pOutputData, count * sizeof(wchar_t));
			len = processed = count * sizeof(wchar_t);
			retConv = S_OK;
		} else if (MudCodePage == 1201) {
			int count = min(len, processed) / sizeof(wchar_t);
			utf16le_to_utf16be(dst, (const wchar_t*)pOutputData, count);
			len = processed = count * sizeof(wchar_t);
			retConv = S_OK;
		} else {
			retConv = fConvertINetMultiByteToUnicode(&mode, MudCodePage, pOutputData, &processed, dst, &len);
		}
		if (retConv != S_OK) {
			wchar_t buf[BUFFER_SIZE];
			swprintf(buf, rs::rs(1292), processed, retConv);
			tintin_puts2(buf);
		}

		if (processed < OutputSize)
			memmove(pOutputData, pOutputData + processed, OutputSize - processed);
		OutputSize -= processed;
		ret += len;
	}
	return ret;
}

void send_telnet_subnegotiation(unsigned char option, const wchar_t *output, int length)
{
	int i;
	vector<char> buf;

	int coded_len = WideCharToMultiByte(MudCodePage, 0, output, length, NULL, 0, NULL, NULL);
	char *coded = new char[coded_len];
	WideCharToMultiByte(MudCodePage, 0, output, length, coded, coded_len, NULL, NULL);

	buf.push_back((char)TN_IAC);
	buf.push_back((char)TN_SB);
	buf.push_back((char)option);
	for (i = 0; i < coded_len; i++) {
		switch (coded[i]) {
		case TN_IAC:
			buf.push_back((char)TN_IAC);
			buf.push_back((char)TN_IAC);
			break;
		case TN_SB:
		case TN_SE:
			break;
		default:
			buf.push_back(coded[i]);
			break;
		}
	}
	buf.push_back((char)TN_IAC);
	buf.push_back((char)TN_SE);

	delete[] coded;
	
	tls_send(MUDSocket, buf.begin(), buf.size());

	if (mesvar[MSG_TELNET]) {
		wchar_t buf[BUFFER_SIZE], optname[64];
		get_telnet_option_name(option, optname);
		swprintf(buf, L"#TELNET SB-%ls: send %d byte(s)", optname, length);
		tintin_puts(buf);
	}
}

void recv_telnet_subnegotiation(unsigned char option, const char *input, int length)
{
	if (mesvar[MSG_TELNET]) {
		wchar_t buf[BUFFER_SIZE], optname[64];
		get_telnet_option_name(option, optname);
		swprintf(buf, L"#TELNET SB-%ls: recv %d byte(s)", optname, length);
		tintin_puts(buf);
	}

	int wide_len = MultiByteToWideChar(MudCodePage, 0, input, length, NULL, 0);
	wchar_t *data = new wchar_t[wide_len + 1];
	MultiByteToWideChar(MudCodePage, 0, input, length, data, wide_len);
	data[wide_len] = L'\0';

	pJmcObj->m_pvarEventParams[0] = ((LONG)TN_SE);
	pJmcObj->m_pvarEventParams[1] = ((LONG)option);
	pJmcObj->m_pvarEventParams[2] = (data);
	
    BOOL bRet = pJmcObj->Fire_Telnet();
	if ( bRet ) {
    }

	delete[] data;
}

void do_telnet_protecol(const char* input, int length, int *used, char* output, int capacity, int *generated)
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

	if (SocketFlags & SOCKNAWS) {
		unsigned char buf[9];
		int w, h;
		GetWindowSize(-1, w, h);
		if (w != LastWidthReported || h != LastHeightReported) {
			buf[0] = TN_IAC;
			buf[1] = TN_SB;
			buf[2] = TN_NAWS;
			buf[3] = (w >> 8) & 0xFF;
			buf[4] = (w >> 0) & 0xFF;
			buf[5] = (h >> 8) & 0xFF;
			buf[6] = (h >> 0) & 0xFF;
			buf[7] = TN_IAC;
			buf[8] = TN_SE;
			// can't use send_telnet_subnegotiation() routine
			// due to conversion from unicode used there
			// this data is binary, not text
			//send_telnet_subnegotiation(TN_NAWS, (const char*)buf, 4);
			tls_send(MUDSocket, (char*)buf, sizeof(buf));
			LastWidthReported = w;
			LastHeightReported = h;
		}
	}

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
								wchar_t mtts_is[128];
								wchar_t *value = &(mtts_is[1]);
								mtts_is[0] = MTTS_IS;

								switch (MttsCounter) {
								case 0:
									swprintf(value, L"%ls", L"JMC");
									MttsCounter++;
									break;
								case 1:
									swprintf(value, L"%ls", L"ANSI");
									MttsCounter++;
									break;
								case 2:
									swprintf(value, L"%ls %d", L"MTTS", 
										MTTS_ANSI | MTTS_UTF8);
									break;
								}
								send_telnet_subnegotiation(TN_TTYPE, mtts_is, wcslen(value) + 1);
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
					send_telnet_command(TN_DONT, opt);
				} else {
					switch(opt) {
					case TN_ECHO:
						SocketFlags |= SOCKECHO;
						send_telnet_command(TN_DO, TN_ECHO);
						break;
					case TN_TTYPE:
						MttsCounter = 0;
						send_telnet_command(TN_DO, TN_TTYPE);
						break;
					case TN_COMPRESS:
						if (SentDoCompress2)
							send_telnet_command(TN_DONT, TN_COMPRESS);
						else
							send_telnet_command(TN_DO, TN_COMPRESS);
						break;
					case TN_COMPRESS2:
						send_telnet_command(TN_DO, TN_COMPRESS2);
						SentDoCompress2 = true;
						break;
					default:
						send_telnet_command(TN_DO, opt);
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
					send_telnet_command(TN_DONT, TN_ECHO);
					break;
				case TN_EOR_OPT:
					SocketFlags &= ~SOCKEOR;
					send_telnet_command(TN_DONT, TN_EOR);
					break;
				default:
					send_telnet_command(TN_DONT, opt);
					break;
				}
				State = '\0';
			}
			break;
		case TN_DO: {
				unsigned char opt = input[*used];
				RecvCmd(TN_DO, opt);
				if (!telnet_option_enabled(opt)) {
					send_telnet_command(TN_WONT, opt);
				} else {
					switch (opt) {
					case TN_ECHO:
						SocketFlags &= ~SOCKECHO;
						send_telnet_command(TN_WILL, TN_ECHO);
						break;
					case TN_NAWS:
						SocketFlags |= SOCKNAWS;
						send_telnet_command(TN_WILL, TN_NAWS);
						break;
					case TN_TTYPE:
						MttsCounter = 0;
						send_telnet_command(TN_WILL, TN_TTYPE);
						break;
					default:
						send_telnet_command(TN_WONT, opt);
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
						send_telnet_command(TN_WONT, TN_ECHO);
						break;
					case TN_NAWS:
						SocketFlags &= ~SOCKNAWS;
						send_telnet_command(TN_WONT, TN_NAWS);
						break;
					default:
						send_telnet_command(TN_WONT, opt);
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
							if (input[*used] == strPromptEndSeqBytes[PromptEndIndex]) {
								PromptEndIndex++;
								if (!strPromptEndSeqBytes[PromptEndIndex]) { //match!
									for (char *copy = (char*)strPromptEndReplBytes; *copy; )
										output[(*generated)++] = *(copy++);
									output[(*generated)++] = END_OF_PROMPT_MARK;
									PromptEndIndex = 0;
								}
								continue;
							}
							for (char *copy = (char*)strPromptEndSeqBytes; PromptEndIndex; PromptEndIndex--)
								output[(*generated)++] = *(copy++);
						}

						if (bIACReciveSingle || ch != 255) {
							output[(*generated)++] = ch;
						} else {
							State = ch;
						}
					}
				}
			}
		}
    }
}
 
