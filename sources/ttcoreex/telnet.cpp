#include "stdafx.h"
#include "winsock.h"
#include "tintin.h"
#include "telnet.h"

unsigned char State;
int SocketFlags;

void SendCmd(int cmd, int opt)
{
	char szBuf[4];
	wsprintf(szBuf, "%c%c%c", (char)(unsigned char)TN_IAC, (char)cmd, (char)opt);
    send(MUDSocket,szBuf, 3, 0 );
}

static void RecvCmd(int cmd, int opt)
{
}


#ifdef _DEBUG_LOG
extern DLLEXPORT HANDLE hExLog;
#endif

int do_telnet_protecol(unsigned char* cpsource, char* cpdest, int MaxSize)
{
    int i = 0;
    int count = 0;
    
    
#ifdef _DEBUG_LOG
        if (hExLog ) {
            char exLogText[128];
            DWORD Written;
            sprintf(exLogText , "\r\n#telnet state:%d#\r\n", State);
            WriteFile(hExLog , exLogText , strlen(exLogText) , &Written, NULL);
            //WriteFile(hExLog , buffer , didget , &Written, NULL);
        }
#endif    
    
    for ( ; i < MaxSize ; i++ , cpsource++) {
	    if( State == TN_IAC ){
 		    switch( State = *cpsource){
			    case TN_GA:
                    // cpdest[count++] = '\n';
                    cpdest[count++] = 0x1;
			    case TN_EOR:
				    break;

			    case TN_SB: // Impossible now
			    case TN_WILL:
			    case TN_WONT:
			    case TN_DO:
			    case TN_DONT:
				    break;

                case TN_IAC:
                    cpdest[count++] = *cpsource;

                default:
				    State = '\0';
				    break;
		    }
	    }else if(State == TN_WILL){
		    RecvCmd(TN_WILL, *cpsource);
		    if( *cpsource == TN_ECHO ){
			    if( SocketFlags & SOCKECHO ){
				    SocketFlags &= ~SOCKECHO;
				    SendCmd(TN_DONT, TN_ECHO); // !!! DID DO 
			    }
		    }else if(*cpsource == TN_EOR_OPT) {
			    if( ! (SocketFlags & SOCKEOR) ){
				    SocketFlags |= SOCKEOR;
				    SendCmd(TN_DO, TN_EOR_OPT);
			    }
		    }else{
			    SendCmd(TN_DONT, *cpsource);
		    }
		    State = '\0';
	    }else if(State == TN_WONT){
		    RecvCmd(TN_WONT, *cpsource);
		    if( *cpsource == TN_ECHO){
			    if( ! (SocketFlags & SOCKECHO) ){
				    SocketFlags |= SOCKECHO;
				    SendCmd(TN_DONT, TN_ECHO);
			    }
		    }else if(*cpsource == TN_EOR_OPT){
			    if( SocketFlags & SOCKEOR ){
				    SocketFlags &= ~SOCKEOR;
				    SendCmd(TN_DONT, TN_EOR);
			    }
		    }
		    State = '\0';
	    }else if(State == TN_DO){
		    RecvCmd(TN_DO, *cpsource);
		    SendCmd(TN_WONT, *cpsource);
		    State = '\0';
	    }else if(State == TN_DONT){
		    RecvCmd(TN_DONT, *cpsource);
		    State = '\0';
	    }else if(*cpsource == TN_IAC){
		    if( ! (SocketFlags & SOCKTELNET) )
			    SocketFlags |= SOCKTELNET;
		    State = *cpsource;
        }else {
            while ( i < MaxSize ) {
                if ( *cpsource == 0x9 ) {
                    cpdest[count++] = ' ';
                    cpdest[count++] = ' ';
                    cpdest[count++] = ' ';
                    cpdest[count++] = ' ';
                    cpsource++;
                    i++;
                } else 
                    if ( (bIACReciveSingle || *cpsource != 255) && (*cpsource >= 128 || isprint(*cpsource) || * cpsource == 0x1B || 
                            *cpsource == '\r' || *cpsource == '\n') ) {
                        cpdest[count++] = *cpsource++;
                        i++;
                    } else 
                        if ( *cpsource == 0x7 ) {
                            MessageBeep(MB_OK);
                            cpsource++;
                            i++;
                        } else
                            break;

            }
            State = *cpsource;
	    }
    }
    cpdest[count] = 0;
    return count;
}
 
