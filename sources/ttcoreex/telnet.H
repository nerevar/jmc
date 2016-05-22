#ifndef _TELNET_H_
#define _TELNET_H_

#define SOCKDEAD     		00001   /* connection dead */
#define SOCKPENDING  		00002   /* connection not yet established */
#define SOCKLOGIN    		00004   /* autologin requested by user */
#define SOCKEOR      		00020   /* server will send EOR after prompts */
#define SOCKECHO     		00040   /* server will do input echo */
#define SOCKEDIT     		00100   /* do local editing (not used) */
#define SOCKTRAP     		00200   /* do local signal trapping (not used) */
#define SOCKLINEMODE 		00400   /* do telnet LINEMODE negotiation (not used) */
#define SOCKTELNET   		01000   /* server supports telnet protocol (not used) */
#define SOCKPROMPT   		02000   /* last prompt was definitely a prompt */
#define SOCKCOMPRESSING		04000	/* zlib compression (MCCP) is ON */
#define SOCKNAWS		   010000	/* negotiations about window size enabled */

#define TN_ECHO         0001    /* echo option */
#define TN_SGA          0003    /* suppress GOAHEAD option */
#define TN_NAWS         0037    /* Negotiate About Window Size */
#define TN_STATUS       0005    /* not used */
#define TN_TIMING_MARK  0006    /* not used */
#define TN_EOR_OPT      0031    /* EOR option */
#define TN_TSPEED       0040    /* not used */
#define TN_LINEMODE     0042    /* not used */

/* MUD-specific protocols */
#define TN_TTYPE        0030    /* MUD Terminal Type Standard (MTTS) */
#define TN_MSDP			0105	/* MUD Server Data Protocol (MSDP) */
#define TN_MSSP			0106	/* MUD Server Status Protocol (MSSP) */
#define TN_COMPRESS		0125	/* MUD Client Compression Protocol (MCCP) v1 */
#define TN_COMPRESS2	0126	/* MUD Client Compression Protocol (MCCP) v2 */
#define TN_MSP			0132	/* MUD Sound Protocol (MSP) */
#define TN_MXP			0133	/* MUD eXtension Protocol (MXP) */
#define	TN_ATCP			0310	/* Achaea Telnet Client Protocol (ATCP) */
#define	TN_GMCP			0311	/* Generic MUD Communication Protocol (GMCP) */

/* TTYPE (MTTS) definitions */
#define MTTS_IS			0
#define MTTS_SEND		1

#define MTTS_ANSI		1
#define MTTS_VT100		2
#define MTTS_UTF8		4
#define MTTS_256COLORS	8
#define MTTS_MOUSETRACK	16
#define MTTS_OSCPALETTE	32
#define MTTS_SCREENREADER	64
#define MTTS_PROXY		128

/* MSDP definitions */
#define MSDP_VAR			1
#define MSDP_VAL			2
#define MSDP_TABLE_OPEN		3
#define MSDP_TABLE_CLOSE	4
#define MSDP_ARRAY_OPEN		5
#define MSDP_ARRAY_CLOSE	6

/* MSSP definitions */
#define MSSP_VAR			1
#define MSSP_VAL			2

#define TN_EOR          0357    /* End-Of-Record */
#define TN_SE           0360    /* End subnegotioation */
#define TN_NOP          0361    /* not used */
#define TN_DATA_MARK    0362    /* not used */
#define TN_BRK          0363    /* not used */
#define TN_IP           0364    /* not used */
#define TN_AO           0365    /* not used */
#define TN_AYT          0366    /* not used */
#define TN_EC           0367    /* not used */
#define TN_EL           0370    /* not used */
#define TN_GA           0371    /* Go Ahead */
#define TN_SB           0372    /* Begin subnegotiation */
#define TN_WILL         0373    /* I offer to ~, or ack for DO */
#define TN_WONT         0374    /* I will stop ~ing, or nack for DO */
#define TN_DO           0375    /* Please do ~?, or ack for WILL */
#define TN_DONT         0376    /* Stop ~ing!, or nack for WILL */
#define TN_IAC          0377    /* telnet Is A Command character */

#define TELNET_OPTIONS_NUM 12
typedef struct
{
	unsigned char Code;
	const char *Name;
	const char *Descr;
} TelnetOption;

#define ZLIB_DLL
#include "zlib.h"
#pragma comment(lib,"zlib.lib")

#endif // _TELNET_H_

