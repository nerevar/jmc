#include "stdafx.h"
#include <winsock.h>
#include "tintin.h"

TLSType DLLEXPORT lTLSType;
wstring DLLEXPORT strCAFile;

static TLSType last_tls = TLS_DISABLED;
static bool ssl_loaded = false;

#pragma comment(lib, "wolfssl.lib")
#include <wolfssl/ssl.h>
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/wolfcrypt/asn_public.h>
#include <wolfssl/wolfcrypt/error-crypt.h>

static WOLFSSL *ssl = NULL;
static WOLFSSL_CTX *ctx = NULL;
static WOLFSSL_X509_STORE *store = NULL;

/*
 * syntax:
 * #secure
 * #secure [disable|enable|ssl3|tls1|tls1.1|tls1.2] {ca clear|<filename.pem>}
 */
void secure_command(wchar_t *arg) 
{
    wchar_t param[BUFFER_SIZE], param2[BUFFER_SIZE];

	while (arg && arg[0]) {
		arg = get_arg_in_braces(arg,param,STOP_SPACES,sizeof(param)/sizeof(wchar_t)-1);
		if (is_abrev(param, L"disable"))
			lTLSType = TLS_DISABLED;
		else if (is_abrev(param, L"enable"))
			lTLSType = TLS_TLS1;
		else if (is_abrev(param, L"ssl3"))
			lTLSType = TLS_SSL3;
		else if (is_abrev(param, L"tls1"))
			lTLSType = TLS_TLS1;
		else if (is_abrev(param, L"tls1.1"))
			lTLSType = TLS_TLS1_1;
		else if (is_abrev(param, L"tls1.2"))
			lTLSType = TLS_TLS1_2;
		else if (is_abrev(param, L"ca")) {
			arg = get_arg_in_braces(arg,param2,STOP_SPACES,sizeof(param2)/sizeof(wchar_t)-1);
			if (is_abrev(param2, L"clear"))
				strCAFile = L"";
			else
				strCAFile = param2;
		}
	}

	//#secure settings: protocol %s; CA %s
	swprintf(param, rs::rs(1282),
		(lTLSType == TLS_SSL3     ? L"ssl3"   :
	     lTLSType == TLS_TLS1     ? L"tls1"   :
	     lTLSType == TLS_TLS1_1   ? L"tls1.1" :
	     lTLSType == TLS_TLS1_2   ? L"tls1.2" : L"-"),
	    (strCAFile.size() > 0 ? strCAFile.c_str() : L"-"));
	tintin_puts2(param);
}

static int verify_cert(int preverify, WOLFSSL_X509_STORE_CTX* store)
{
	USES_CONVERSION;
	
	wchar_t buf[BUFFER_SIZE];
	wchar_t fname[128], fpath[BUFFER_SIZE];

	if (!preverify) {
		switch (store->error) {
		case ASN_BEFORE_DATE_E:
		case ASN_AFTER_DATE_E:
			tintin_puts2(rs::rs(1285));
			return 0;
		case ASN_NO_SIGNER_E:
			break;
		default:
			return 0;
		}
	}

	WOLFSSL_X509 *cert_remote = wolfSSL_get_peer_certificate(ssl);

	if (!cert_remote) {
		tintin_puts2(rs::rs(1286));
		return 0;
	}

	tintin_puts2(rs::rs(1287));

	if (strCAFile.size() > 0) {
		wcscpy(fname, strCAFile.c_str());
	} else {
		swprintf(fname, L"%ls.pem", A2W(store->domain));
		MakeAbsolutePath(fpath, fname, szSETTINGS_DIR);
	}

	WOLFSSL_X509 *cert_local = wolfSSL_X509_load_certificate_file(W2A(fpath), SSL_FILETYPE_PEM);

	swprintf(fname, L"%ls.pem", A2W(store->domain));
	MakeAbsolutePath(fpath, fname, szSETTINGS_DIR);

	if (cert_local) {
		int len_local, len_remote;

		const unsigned char *der_local = wolfSSL_X509_get_der(cert_local, &len_local);
		const unsigned char *der_remote = wolfSSL_X509_get_der(cert_remote, &len_remote);

		if (len_local != len_remote || memcmp(der_local, der_remote, len_local)) {
			swprintf(buf, rs::rs(1288), fpath);
			tintin_puts2(buf);
			wolfSSL_X509_free(cert_remote);
			wolfSSL_X509_free(cert_local);
			return 0;
		}
	} else {
		char pem_buf[10 * 1024]; // should be ebough
		int len_remote;
		const unsigned char *der_remote = wolfSSL_X509_get_der(cert_remote, &len_remote);

		FILE *fpem = _wfopen(fpath, L"w");
		int len_pem = wc_DerToPem(der_remote, len_remote, (unsigned char*)pem_buf, sizeof(pem_buf), CERT_TYPE);
		if (len_pem > 0 && len_pem < sizeof(pem_buf)) {
			pem_buf[len_pem] = '\0';
			fwrite(pem_buf, 1, len_pem, fpem);
		}
		fclose(fpem);
		swprintf(buf, rs::rs(1289), fpath);
		tintin_puts2(buf);
	}

	wolfSSL_X509_free(cert_local);
	wolfSSL_X509_free(cert_remote);

    return 1;
}

int tls_open(SOCKET sock) 
{
	USES_CONVERSION;

	last_tls = TLS_DISABLED;

	if (!ssl_loaded) {
		wolfSSL_Init();
		ssl_loaded = true;
	}

	if (ctx) {
		wolfSSL_CTX_free(ctx);
		ctx = NULL;
	}

	WOLFSSL_METHOD*  method = NULL;
	switch (lTLSType) {
	default:
	case TLS_DISABLED:
		return 0;
		break;
	case TLS_SSL3:
		method = wolfSSLv3_client_method();
		break;
	case TLS_TLS1:
		method = wolfTLSv1_client_method();
		break;
	case TLS_TLS1_1:
		method = wolfTLSv1_1_client_method();
		break;
	case TLS_TLS1_2:
		method = wolfTLSv1_2_client_method();
		break;
	}
	if (method == NULL) {
		tintin_puts2(L"#ssl: can't create method");
		return -1;
	}
	
	ctx = wolfSSL_CTX_new(method);

	if (ctx == NULL) {
		tintin_puts2(L"#ssl error: can't create context");
		return -1;
	}

	wolfSSL_CTX_set_cipher_list(ctx, "DHE-PSK-AES128-GCM-SHA256");
	wolfSSL_CTX_SetMinDhKey_Sz(ctx, 1024);

	if (strCAFile.size() > 0) {
		wolfSSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, 0);
		if (!wolfSSL_CTX_load_verify_locations(ctx, W2A(strCAFile.c_str()), NULL)) {
			tintin_puts2(rs::rs(1290));
			wolfSSL_CTX_free(ctx);
			ctx = NULL;
			return -1;
		}
	} else {
		//wolfSSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, 0);
		wolfSSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, verify_cert);
	}

	if (ssl) {
		wolfSSL_free(ssl);
	}
	ssl = wolfSSL_new(ctx);

	if (!ssl) {
		tintin_puts2(L"#ssl error: can't create SSL object");
		wolfSSL_CTX_free(ctx);
		ctx = NULL;
		return -1;
	}

	if (!wolfSSL_set_fd(ssl, sock)) {
		tintin_puts2(L"#ssl error: initializing SSL failure");
		wolfSSL_free(ssl);
		ssl = NULL;
		wolfSSL_CTX_free(ctx);
		ctx = NULL;
		return -1;
	}

	int ret, err;

	unsigned long cmd = 1;
	if (ioctlsocket(sock, FIONBIO, &cmd) != 0) {
		tintin_puts2(L"#ssl: can't set socket non-blocking mode");
		return -1;
	}

	int dt = 20;
	for (int timeout = 2000; timeout > 0; timeout -= dt) {
		ret = wolfSSL_connect(ssl);

		if (ret == SSL_SUCCESS)
			break;

		err = wolfSSL_get_error(ssl, 0);

		if (err != SSL_ERROR_WANT_READ && err != SSL_ERROR_WANT_READ) {
			wchar_t buf[2048];
			swprintf(buf, rs::rs(1291), 
				err, A2W(wolfSSL_ERR_reason_error_string(err)));
			tintin_puts2(buf);
			break;
		}

		Sleep(dt);
	}

	if (ret != SSL_SUCCESS)
		return -1;

	last_tls = lTLSType;
	return 0;
}

int tls_send(SOCKET sock, const char *buffer, int length) 
{
	switch (last_tls) {
	default:
	case TLS_DISABLED:
		return send(sock, buffer, length, 0);
	case TLS_SSL3:
	case TLS_TLS1:
	case TLS_TLS1_1:
	case TLS_TLS1_2:
		return wolfSSL_write(ssl, buffer, length);
	}
}

int tls_recv(SOCKET sock, char *buffer, int maxlength) 
{
	switch (last_tls) {
	default:
	case TLS_DISABLED:
		return recv(sock, buffer, maxlength, 0);
	case TLS_SSL3:
	case TLS_TLS1:
	case TLS_TLS1_1:
	case TLS_TLS1_2:
		{
			int ret = wolfSSL_read(ssl, buffer, maxlength);
			if (ret <= 0)
				ret = (wolfSSL_get_error(ssl, 0) ==  SSL_ERROR_WANT_READ) ? 0 : -1;
			return ret;
		}
	}
}

int tls_close(SOCKET sock) 
{
	switch (last_tls) {
	default:
	case TLS_DISABLED:
		return 0;
	case TLS_SSL3:
	case TLS_TLS1:
	case TLS_TLS1_1:
	case TLS_TLS1_2:
		wolfSSL_shutdown(ssl);
		wolfSSL_free(ssl);
		ssl = NULL;
		wolfSSL_CTX_free(ctx);
		ctx = NULL;
		last_tls = TLS_DISABLED;
		return 0;
	}
}
