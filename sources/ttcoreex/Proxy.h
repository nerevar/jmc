#ifndef _PROXY_H_
#define _PROXY_H_

/*
 * This is highly reduced and rewritten version of CSOCKS code
 */

enum proxy_version {
	PROXY_SOCKS4 = 4,
	PROXY_SOCKS5 = 5
};


//RFC-1928, RFC-1929, RFC-1961
enum socks_request {
	SOCKS4_CONNECT,
	SOCKS5_CONNECT,
	SOCKS5_AUTH_REQ,
	SOCKS5_KEY_EXCHANGE_REQ,
	SOCKS5_HOST_KEY_EXCHANGE_REQ,
	SOCKS5_METHOD
};

enum socks4_command {
	VN_CONNECT  =   0x04,
	CD_CONNECT  =   0x01,
	CD_CREQ_GRANT = 0x5a,	
	CD_CREQ_REJECT = 0x5b,
	CD_CREQ_UNABLE = 0x5c,
	CD_CREQ_DIFF  = 0x5c
};

enum socks5_command {
	CMD_CONNECT = 0x01,
	CMD_AUTH = 0x01,
	RSV = 0x00,
	ATYP_V4 = 0x01,
	ATYP_DN = 0x03,
	ATYP_V6 = 0x04
};

enum socks5_method {
	NO_AUTH_REQ = 0x00,
	GSSAPI = 0x01,
	USER_PWD = 0x02,
	S_USER_PWD = 0x21,
	NAM = 0xFF
};

enum socks_socket_type {
	SOCKS_TCP_CONNECT = 0x0
};

#define SOCKS5_METHOD_IMPLEMENTED  0x02

enum returned_values {
	PROXY_ENOMEM = -2,
	PROXY_OK =	0,
	PROXY_KO = 	-1,
	PROXY_CONN_FAILED = -3,
	PROXY_BIND_FAILED = -4
};

#define SOCKS4_TIMEOUT	120	/* timeout from rfc 	 					    */
#define USERID_SIZE 255
#define SOCKS4_RESPONSE	0x02	/* RESPONSE from Server (it return VN(1 byte) and CD(1 byte)	    */
#define PROXY_DEFAULT_PORT 1080
#define SOCKS4_RESPONSE_LEN 0x08

#define SOCKS5_METHODS 255

#pragma pack(1)

// RFC-1928
#define LEN_SOCKS5_METHODS 2
struct socks5_methods{
	unsigned char ver;
	unsigned char nmethods;
	unsigned char methods[SOCKS5_METHOD_IMPLEMENTED];
};
#define LEN_SOCKS5_REQ 10
struct socks5_req{
	unsigned char ver;
	unsigned char cmd;
	unsigned char rsv;
	unsigned char atyp;
	unsigned long dst_ip;
	unsigned short dstport;
};

/* RFC-1929 (Socks5 Authentication) */
#define LEN_SOCKS5_AUTH_REQ 3
struct socks5_auth_req{
	unsigned char ver;
	unsigned char ulen;
	unsigned char uname_plen_password [ 2 * USERID_SIZE +  1];
};


#define SOCKS5_AUTH_REPLY 20
struct socks5_auth_reply{
	unsigned char ver;
	unsigned char status;
};

#define LEN_SOCKS4_REQ 9
struct socks4_req{
	unsigned char vn;
	unsigned char cd;
	unsigned short dstport;
	unsigned long  dst_ip;
	char userid [USERID_SIZE]; 	/* can be null , (byte of zero) */   
	unsigned char term;
};

struct socks4_resp{
	unsigned char vn;
	unsigned char cd;
	unsigned short dstport;
	unsigned long dst_ip;
};

#pragma pack()

#endif // _PROXY_H_

