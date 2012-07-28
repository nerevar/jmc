
#ifndef _TELNET_H_
#define _TELNET_H_

#define SOCKDEAD     		00001   /* connection dead */
#define SOCKPENDING  		00002   /* connection not yet established */
#define SOCKLOGIN    		00004   /* autologin requested by user */
#define SOCKEOR      		00020   /* server will send EOR after prompts */
#define SOCKECHO     		00040   /* do local keyboard echo */
#define SOCKEDIT     		00100   /* do local editing (not used) */
#define SOCKTRAP     		00200   /* do local signal trapping (not used) */
#define SOCKLINEMODE 		00400   /* do telnet LINEMODE negotiation (not used) */
#define SOCKTELNET   		01000   /* server supports telnet protocol (not used) */
#define SOCKPROMPT   		02000   /* last prompt was definitely a prompt */

#define TN_ECHO         0001    /* echo option */
#define TN_SGA          0003    /* suppress GOAHEAD option */
#define TN_STATUS       0005    /* not used */
#define TN_TIMING_MARK  0006    /* not used */
#define TN_TTYPE        0030    /* not used */
#define TN_EOR_OPT      0031    /* EOR option */
#define TN_NAWS         0037    /* not used */
#define TN_TSPEED       0040    /* not used */
#define TN_LINEMODE     0042    /* not used */

#define TN_EOR          0357    /* End-Of-Record */
#define TN_SE           0360    /* not used */
#define TN_NOP          0361    /* not used */
#define TN_DATA_MARK    0362    /* not used */
#define TN_BRK          0363    /* not used */
#define TN_IP           0364    /* not used */
#define TN_AO           0365    /* not used */
#define TN_AYT          0366    /* not used */
#define TN_EC           0367    /* not used */
#define TN_EL           0370    /* not used */
#define TN_GA           0371    /* Go Ahead */
#define TN_SB           0372    /* not used */
#define TN_WILL         0373    /* I offer to ~, or ack for DO */
#define TN_WONT         0374    /* I will stop ~ing, or nack for DO */
#define TN_DO           0375    /* Please do ~?, or ack for WILL */
#define TN_DONT         0376    /* Stop ~ing!, or nack for WILL */
#define TN_IAC          0377    /* telnet Is A Command character */


#endif // _TELNET_H_

