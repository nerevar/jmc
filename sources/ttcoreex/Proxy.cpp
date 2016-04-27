#include "stdafx.h"
#include "winsock.h"
#include "tintin.h"
#include "Proxy.h"

static unsigned char socks5_method_implemented [] = { NO_AUTH_REQ, USER_PWD };

/* socks 5 functions */
static int socks5_create_methods (struct socks5_methods * method, unsigned char met);
static int socks5_create_request (struct socks5_req * req, unsigned char cmd, unsigned long dst_ip, unsigned short dst_port);
static int socks5_create_auth_request (struct socks5_auth_req * req, char * user_name, char * passwd);
static int socks5_auth_req_hs (struct socks5_auth_req * socks_req, int a, struct sockaddr *serv, char * username, char * passwd);
static int socks5_in_all (struct sockaddr * serv, int a, int command,unsigned long addr, unsigned short port);
/* end of socks5 functions */

/* socks 4 functions */
static int socks4_connect(struct socks4_req * socks4, int type, int dst_port, struct in_addr * dstip, char * userid);
static int socks4_create_packet(struct socks4_req *socks4, int vn_connect,int cd_connect,int dst_port,struct in_addr * dstip, char * userid);
static int socks4_in_all(int command, int port, struct in_addr * dst, int a, struct sockaddr * serv);
/* end of socks 4 functions */

static int socks_connect_socks_server(int sock, struct sockaddr * server, void * request, int version);
static int socks_new_connect_ss (int sock, struct sockaddr * server, int val);
static int socks_send(int sock, int lenght, void * sen);
static int socks_recv (int sock, int rrlen, void * response);

static SOCKET connected_socket = INVALID_SOCKET;

DWORD DLLEXPORT ulProxyAddress = 0;
WORD DLLEXPORT dwProxyPort = 0;
WORD DLLEXPORT dwProxyType = PROXY_SOCKS4;
char DLLEXPORT sProxyUserName[256];
char DLLEXPORT sProxyUserPassword[256];

static char prev_proxylist_fname[MAX_PATH+2];
static int prev_proxylist_line = 0;

/*
 * syntax:
 * #proxy
 * #proxy disable
 * #proxy list <filename> [<linenum>]
 * #proxy socks4|socks5 <ip>[:<port>] [<username>] [<password>]
 */
void proxy_command(char *arg)
{
    char cmd[BUFFER_SIZE];
    char param[BUFFER_SIZE], param2[BUFFER_SIZE], temp[BUFFER_SIZE];

    arg = get_arg_in_braces(arg, cmd,    STOP_SPACES);

    if ( !cmd[0] ) {
		char buf[BUFFER_SIZE];
		if (!ulProxyAddress)
			sprintf(buf, rs::rs(1268));
		else
			sprintf(buf, rs::rs(1269),
			(dwProxyType == PROXY_SOCKS4 ? "socks4" : "socks5"),
			(ulProxyAddress >> 24) & 0xff, (ulProxyAddress >> 16) & 0xff, (ulProxyAddress >> 8) & 0xff, (ulProxyAddress >> 0) & 0xff,
			(dwProxyPort ? dwProxyPort : PROXY_DEFAULT_PORT),
			(sProxyUserName[0] ? sProxyUserName : "-"),
			(sProxyUserPassword[0] ? sProxyUserPassword : "-"));
		
		tintin_puts2(buf);
        return;
    }

	if (is_abrev(cmd, "disable")) {
		ulProxyAddress = dwProxyPort = 0;
		sProxyUserName[0] = '\0';
		sProxyUserPassword[0] = '\0';

		tintin_puts2(rs::rs(1270));
		return;
	}

	if (is_abrev(cmd, "list")) {
		arg = get_arg_in_braces(arg, param, STOP_SPACES);
		arg = get_arg_in_braces(arg, param2, STOP_SPACES);

		substitute_vars(param, temp, sizeof(temp));
		substitute_myvars(temp, param, sizeof(param));

		int line;
		if (*param2)
			line = atoi(param2);
		else if (!strcmp(param, prev_proxylist_fname))
			line = prev_proxylist_line + 1;
		else
			line = 1;

		FILE *flist = fopen(param, "r");
		if (!flist) {
			tintin_puts2(rs::rs(1271));
			return;
		}

		for(;;) {
			int count = 0;
			while(fgets(temp, sizeof(temp), flist))  {
				if (!*temp)
					continue;
				if (++count == line) {
					break;
				}
			}
			if (count < line) {
				if (count == 0) {
					tintin_puts2(rs::rs(1272));
					fclose(flist);
					return;
				}
				line = ((line - 1) % count) + 1;
				fseek(flist, 0, SEEK_SET);
			} else {
				break;
			}
		}
		fclose(flist);

		strcpy(prev_proxylist_fname, param);
		prev_proxylist_line = line;

		int n = strlen(temp) - 1;
		while (n > 0 && (temp[n] == '\r' || temp[n] == '\n'))
			temp[n--] = '\0';
		arg = temp;

		arg = get_arg_in_braces(arg, cmd, STOP_SPACES);
	}

	int type;
	if (is_abrev(cmd, "socks4"))
		type = PROXY_SOCKS4;
	else if (is_abrev(cmd, "socks5"))
		type = PROXY_SOCKS5;
	else {
		tintin_puts2(rs::rs(1273));
		return;
	}

	arg = get_arg_in_braces(arg, param, STOP_SPACES);
	unsigned int ip1 = 0, ip2 = 0, ip3 = 0, ip4 = 0, port = 0;
	if (!sscanf(param, "%d.%d.%d.%d:%d", &ip1, &ip2, &ip3, &ip4, &port) &&
		!sscanf(param, "%d.%d.%d.%d", &ip1, &ip2, &ip3, &ip4) ) {
		tintin_puts2(rs::rs(1274));
		return;
	}

	dwProxyType = type;
	ulProxyAddress = (ip1 << 24) | (ip2 << 16) | (ip3 << 8) | (ip4 << 0);
	dwProxyPort = port;

	arg = get_arg_in_braces(arg, param, STOP_SPACES);
	strcpy(sProxyUserName, param);
	arg = get_arg_in_braces(arg, param, STOP_SPACES);
	strcpy(sProxyUserPassword, param);

	char buf[BUFFER_SIZE];
	if (!ulProxyAddress)
		sprintf(buf, rs::rs(1270));
	else
		sprintf(buf, rs::rs(1275),
		(dwProxyType == PROXY_SOCKS4 ? "socks4" : "socks5"),
		(ulProxyAddress >> 24) & 0xff, (ulProxyAddress >> 16) & 0xff, (ulProxyAddress >> 8) & 0xff, (ulProxyAddress >> 0) & 0xff,
		(dwProxyPort ? dwProxyPort : PROXY_DEFAULT_PORT),
		(sProxyUserName[0] ? sProxyUserName : "-"),
		(sProxyUserPassword[0] ? sProxyUserPassword : "-"));
	tintin_puts2(buf);
}

int proxy_connect(int sock, const struct sockaddr * sockaddr, int socketlen)
{
	if (!ulProxyAddress)
		return connect(sock, sockaddr, socketlen);

	struct sockaddr_in * bi = (struct sockaddr_in *)sockaddr;
	int sock_type = -1;
	
	unsigned int val = sizeof(sock_type);
    struct sockaddr_in * serv_4 = NULL;
    struct sockaddr * serv = NULL;
	int ret = 0;
	
	serv = (struct sockaddr*)malloc(sizeof(struct sockaddr));
	memset (serv, 0, sizeof(struct sockaddr));
	serv_4 = (struct sockaddr_in *) (serv);
    serv_4->sin_addr.s_addr = htonl(ulProxyAddress);
	serv_4->sin_family = AF_INET;
	serv_4->sin_port = htons(dwProxyPort ? dwProxyPort : PROXY_DEFAULT_PORT);
	memset(&(serv_4->sin_zero), 0, 8);
	
	switch (dwProxyType) {
		
		case PROXY_SOCKS4:
			if ( socks4_in_all(SOCKS4_CONNECT, bi->sin_port, &(bi->sin_addr), sock, (struct sockaddr *) serv)){
				free(serv);
				return PROXY_KO;
			}
			break;
		
		case PROXY_SOCKS5:
			/* creating request for methods */
			/* check for UDP Socket */
			ret = socks5_in_all ((struct sockaddr *)serv, sock, CMD_CONNECT, bi->sin_addr.s_addr, bi->sin_port);
			if (ret != 0){
				free (serv);
				return PROXY_KO;
			}	
			break;
		
		default:
			return PROXY_KO;
			break;
	}/* switch socks_version */

	free(serv);
	return PROXY_OK;
}

int proxy_close(int sock)
{
	if (sock == connected_socket)
		connected_socket = INVALID_SOCKET;
	return closesocket(sock);
}


/******************************************************/
static int socks5_auth_req_hs (struct socks5_auth_req * socks_req, int a, struct sockaddr *serv,
							   char * username, char * passwd){

	if (socks5_create_auth_request (socks_req, username, passwd) < 0)
		return PROXY_KO;
	if (socks_connect_socks_server(a, (struct sockaddr *)serv, (void *) socks_req, SOCKS5_AUTH_REQ ) != 0){
		memset(socks_req,0,sizeof(struct socks5_auth_req));
		return PROXY_KO;
	}
	memset(socks_req,0,sizeof(struct socks5_auth_req));
	return PROXY_OK;
}

static int socks5_in_all (struct sockaddr * serv, int a, int command,unsigned long addr, unsigned short port){
	int i = 0;
	int value = 0;
	int ret_ser = 0;
	void * socks_req2 = NULL;

	socks_req2 =  malloc(sizeof(struct socks5_methods));
	if (!socks_req2)
		return PROXY_ENOMEM;

	memset(socks_req2, 0, sizeof(struct socks5_methods));
	((struct socks5_methods *)socks_req2)->nmethods = 0;

	/* create and send methods implemented yet.  */
	for (i=0; i < SOCKS5_METHOD_IMPLEMENTED; i++){
		if (socks5_create_methods((struct socks5_methods *)socks_req2, socks5_method_implemented[i]) < 0) {
			free(socks_req2);
			return PROXY_KO;
		}
	}
	/* send methods to socks server */
	ret_ser = socks_connect_socks_server(a, (struct sockaddr *)serv, (void *) socks_req2, SOCKS5_METHOD);
	if ( ret_ser < 0 ){
		free (socks_req2);
		return PROXY_KO;
	}

	free(socks_req2);
	/* check response from socks server for accepted methods */
	switch (ret_ser){
	
		case NO_AUTH_REQ:
			/* ok, now send directly the request */
			break;

		case USER_PWD:
			/* creating request for authentication */
			socks_req2 = malloc(sizeof(struct socks5_auth_req));
			if (!socks_req2)
				return PROXY_ENOMEM;
			memset(socks_req2, 0, sizeof(struct socks5_auth_req));
			if (socks5_auth_req_hs ((struct socks5_auth_req *)socks_req2, a, serv, sProxyUserName, sProxyUserPassword) != 0) {
				free (socks_req2);
				return PROXY_KO;
			}
			break;

		default:
			return PROXY_KO;
	} /* switch */
	/* auth step is fine, now can send request */ 

	switch (command){

		case CMD_CONNECT:
			value = SOCKS5_CONNECT;
			break;
		
		default:
			return PROXY_KO;

	} /* switch */

	/* creating request for CONNECTION */
	socks_req2 = malloc(sizeof(struct socks5_req));
	if (!socks_req2)
		return PROXY_ENOMEM;
	
	memset(socks_req2, 0, sizeof(struct socks5_req));
	if (socks5_create_request ((struct socks5_req *)socks_req2, command, addr, port) < 0){
		free(socks_req2);
		return PROXY_KO;
	}
	
	/* send connection request */	
	if (socks_connect_socks_server(a, (struct sockaddr *)serv, (void *)socks_req2, value) != 0){
		free(socks_req2);
		return PROXY_CONN_FAILED;
	}
	
	free(socks_req2);
	return PROXY_OK;
}

static int socks5_create_methods (struct socks5_methods * method, unsigned char met){
	if (! method)
		return PROXY_KO;
	method->ver = PROXY_SOCKS5;
	method->methods[method->nmethods] = met;
	method->nmethods ++;
	return PROXY_OK;
}

static int socks5_create_auth_request (struct socks5_auth_req * req, char * user_name, char * passwd){

	unsigned int lenu = 0;

	req->ver = CMD_AUTH;

	req->ulen = strlen(user_name);

	memset(req->uname_plen_password, 0, 2 * USERID_SIZE + 1);
	memcpy(req->uname_plen_password, user_name, req->ulen);

	req->uname_plen_password[req->ulen] = strlen(passwd);

	memcpy(&(req->uname_plen_password[req->ulen + 1]), passwd, req->uname_plen_password[req->ulen] );

	return PROXY_OK;
}


static int socks5_create_request (struct socks5_req * req, unsigned char cmd, unsigned long dst_ip, unsigned short dst_port){
	req->ver = PROXY_SOCKS5;
	req->cmd = cmd;
	req->rsv = RSV;
	
	req->atyp = ATYP_V4;
    req->dst_ip = dst_ip;
	req->dstport = dst_port;
    
	return PROXY_OK;	
}

static int socks4_create_packet(struct socks4_req *socks4, int vn_connect,int cd_connect,int dst_port,struct in_addr * dstip, char * userid){
	socks4->vn = vn_connect;
	socks4->cd = cd_connect;
	socks4->dstport = dst_port;
	socks4->dst_ip = dstip->s_addr;
	memset(&socks4->userid,0,USERID_SIZE);	
	if (userid)
		memcpy(socks4->userid,userid,strlen(userid));
	return PROXY_OK;
}

static int socks4_in_all(int command, int port, struct in_addr * dst, int a, struct sockaddr * serv){
	void * socks_req = NULL;
	
	socks_req = malloc(sizeof (struct socks4_req));
    if (!socks_req)
		return PROXY_ENOMEM;
        
	memset(socks_req, 0, sizeof(struct socks4_req));
	
    if (socks4_connect((struct socks4_req*)socks_req, command, port, dst, sProxyUserName)){
		free(socks_req);
        return PROXY_KO;
	} 
	
	if (socks_connect_socks_server(a, serv, socks_req, command)){
		free(socks_req);
	    return PROXY_KO;
	}
	free(socks_req);
	return PROXY_OK;	
}		


static int socks4_connect(struct socks4_req * socks4, int type, int dst_port, struct in_addr * dstip, char * userid){
	if (!socks4)
		return PROXY_KO;
	switch (type){
		case SOCKS4_CONNECT:
			/* preparing socks4_connect message */
			if (socks4_create_packet(socks4,VN_CONNECT,CD_CONNECT,dst_port,dstip,userid) < 0  )
				return PROXY_KO;
			break;
		default:
			return PROXY_KO;
			break;
	};
	return PROXY_OK;
}

static int socks_send(int sock, int lenght, void * sen){
	int rlen = 0;
	fd_set set;

	do{
		FD_ZERO(&set);
		FD_SET(sock, &set);
		if (select(sock + 1, NULL, &set, NULL, NULL) <= 0)
			return PROXY_KO;
		if (FD_ISSET(sock, &set))
			break;
		continue;
	}while(1);

	if ((rlen = send (sock, (const char *) sen, lenght, 0)) <= 0)
		return PROXY_KO;
	return rlen;
}

static int socks_recv (int sock, int rrlen, void * response){

	int slen = 0;
	fd_set set;

	do{
		FD_ZERO(&set);
		FD_SET(sock, &set);
		if (select(sock + 1, &set, NULL, NULL, NULL) <= 0)
			return PROXY_KO;
		if (FD_ISSET(sock, &set))
			break;
		continue;
	}while(1);
	slen = recv(sock, (char *) response, rrlen, 0);
	if ( slen <= 0)
		return PROXY_KO;
	return slen;
}

static int socks_new_connect_ss (int sock, struct sockaddr * server, int val){
	int retu= 0;
	fd_set set;

	if (sock == connected_socket)
		return PROXY_OK;
	retu = connect(sock, server, val);
	do{
		if (retu < 0)
			return PROXY_KO;
		
		FD_ZERO(&set);
		FD_SET(sock, &set);
		if (select(sock + 1, NULL, &set, NULL, NULL) <= 0)
			break;
		if (FD_ISSET(sock, &set))
			break;
	}while(1);
	connected_socket = sock;
	return PROXY_OK;
}

static int socks_connect_socks_server(int sock, struct sockaddr * server, void * request, int version){
	
	void * response = NULL;
	unsigned char rest = 0;
    int val = sizeof(struct sockaddr_in);
	void * sen = NULL;
	int len = 0;
	int rlen = 0; 
	int rrlen = 0; 
	int slen = 0;
    int sock_type = -1;
    unsigned int val2 = sizeof(sock_type);

	if (socks_new_connect_ss (sock, server, val) != 0) 
		return PROXY_KO;
	
	switch (version){

		case SOCKS4_CONNECT:
			len = strlen(((struct socks4_req *)request)->userid);
			if (socks_send(sock, LEN_SOCKS4_REQ + len, request) <= 0)
				return PROXY_KO;
			response = malloc (SOCKS4_RESPONSE_LEN);
			if (!response)
				return PROXY_ENOMEM;
			memset(response,0,SOCKS4_RESPONSE_LEN);
			if (socks_recv(sock, SOCKS4_RESPONSE_LEN, response) <= 0){
				free(response);
				return PROXY_KO;
			}
			rest = ((struct socks4_resp *)response)->cd;

			free(response);
			if (rest == CD_CREQ_GRANT)
				return PROXY_OK;
			return PROXY_KO;
			break;

		case SOCKS5_METHOD:
			/* send method request */
			sen = (struct socks5_methods *) request;
			len = LEN_SOCKS5_METHODS;
			slen = SOCKS5_METHOD_IMPLEMENTED;
			rrlen = SOCKS5_AUTH_REPLY;
			break;

		case SOCKS5_AUTH_REQ: 
			
			sen = (struct socks5_auth_req *) request;
			len = LEN_SOCKS5_AUTH_REQ;
			slen = ((struct socks5_auth_req *) request)->ulen + 
				((struct socks5_auth_req *) request)->uname_plen_password[((struct socks5_auth_req *) request)->ulen];
			rrlen = SOCKS5_AUTH_REPLY;
			break;

		case SOCKS5_CONNECT: 
			sen = (struct socks5_req *) request;
			len = LEN_SOCKS5_REQ;
			slen = 0;
			rrlen = SOCKS5_AUTH_REPLY;
			break;

		default:
			return PROXY_KO;
			break;
	} /* switch */
	
	/* Now send the socks5 request */
	if (socks_send(sock, (len + slen), sen) <= 0)
		return PROXY_KO;

	response = malloc (rrlen);
	if (!response)
		return PROXY_ENOMEM;
	memset(response, 0, rrlen);
	slen = 0;
	
	if (socks_recv (sock, rrlen, response) <= 0){
		free(response);	
		return PROXY_KO;
	}

	rlen = ((struct socks5_auth_reply *)response)->status;

	free(response);
	return rlen;	
}
