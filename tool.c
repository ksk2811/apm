#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "php.h"
#include "php_ini.h"
#include "php_apm.h"
#include "tool.h"
#include <errno.h>

//ZEND_DECLARE_MODULE_GLOBALS(apm);
ZEND_EXTERN_MODULE_GLOBALS(apm);

void send_data(char *msg)
{
	int client_socket;
	struct sockaddr_in serverAddress;
	int server_addr_size;
	int msg_len = 0;
	int real_send_msg_len = 0;
	char log_msg[128];

	if (APM_G(sock_type) == 1) { //udp
		if ((client_socket = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
			//php_errors.log 시작할때 warning 로그는 생기는데 이건 안나온다...
			//php_log_err("can't create socket");
			php_syslog(LOG_NOTICE, "can't create socket");
			return;
		}

		memset(&serverAddress, 0, sizeof(serverAddress));
		serverAddress.sin_family = AF_INET;
		inet_aton(APM_G(server_host), (struct in_addr*) &serverAddress.sin_addr.s_addr);
		serverAddress.sin_port = htons(APM_G(server_port));
		server_addr_size = sizeof(serverAddress);

		msg_len = strlen(msg);
		real_send_msg_len = sendto(client_socket, msg, msg_len, 0, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
		if (real_send_msg_len < 0) {
			snprintf(log_msg, 128, "can't send data(%d/%d)", msg_len, real_send_msg_len);
			php_syslog(LOG_NOTICE, log_msg);
		}
		close(client_socket);
	} else { //tcp
		if ((client_socket = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
			//php_log_err("can't create socket");
			php_syslog(LOG_NOTICE, "can't create socket");
			return;
		}

		memset(&serverAddress, 0, sizeof(serverAddress));
		serverAddress.sin_family = AF_INET;
		serverAddress.sin_port = htons(APM_G(server_port));
		inet_aton(APM_G(server_host), (struct in_addr*) &serverAddress.sin_addr.s_addr);

		if (connect(client_socket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
			php_syslog(LOG_NOTICE, "can't conncet socket");
			close(client_socket);
			return;
		}

		write(client_socket, msg, strlen(msg));
		close(client_socket);
	}
}

time_t get_millisec()
{
    struct timeval te; 
    gettimeofday(&te, NULL);
    time_t milliseconds = (te.tv_sec * 1000) + (te.tv_usec / 1000);

    return milliseconds;
}

int get_heap_super_global(char **msg, const char* name)
{

	zval *super_global = NULL;
	zval *data = NULL;
	zend_string *str_data = NULL;
	int msg_size = 0;
	
	if ((super_global = zend_hash_str_find(&EG(symbol_table), ZEND_STRL("_SERVER"))) == NULL) {
		msg_size = snprintf(NULL, 0, "%s", "no_data") + 1;
		if ((*msg = malloc(msg_size)) != NULL) {
			snprintf(msg, msg_size, "%s", "no_data") + 1;
		} else {
			php_syslog(LOG_NOTICE,"malloc failed (_SERVER)");
		}
		//*msg = pemalloc(msg_size, 1);
		return FALSE;
	}

	if ((data = zend_hash_str_find(Z_ARRVAL_P(super_global), (name), strlen(name))) == NULL) {
		msg_size = snprintf(NULL, 0, "%s", "no_data") + 1;
		if ((*msg = malloc(msg_size)) != NULL) {
			snprintf(*msg, msg_size, "%s", "no_data");
		} else {
			php_syslog(LOG_NOTICE,"malloc failed %s", name);
		}
		return FALSE;
	}

	str_data = zend_string_init(Z_STRVAL_P(data), Z_STRLEN_P(data), 0);
	msg_size = snprintf(NULL, 0, "%s", ZSTR_VAL(str_data)) + 1;
	if ((*msg = malloc(msg_size)) != NULL) {
		snprintf(*msg, msg_size, "%s", ZSTR_VAL(str_data));
	} else {
		php_syslog(LOG_NOTICE, "malloc failed msg");
	}
	zend_string_release(str_data);

	return TRUE;
	/*
	다른 코드
	zval *super_global;
	zval *data;
	zend_string *str_data = NULL;

	//zend_is_auto_global_str(ZEND_STRL("_SERVER"));
	//super_global = &PG(http_globals)[TRACK_VARS_SERVER];
	data = zend_hash_str_find(Z_ARRVAL_P(super_global), (name), strlen(name));
	str_data = zend_string_init(Z_STRVAL_P(data), Z_STRLEN_P(data), 0); //free가 필요한 함수인지 조사
	snprintf(msg, len, "%s", ZSTR_VAL(str_data));

	return TRUE;
	*/
}

int get_super_global(char *msg, int len, const char* name)
{
        zval *super_global = NULL;
        zval *data = NULL;
        zend_string *str_data = NULL;
    
        if ((super_global = zend_hash_str_find(&EG(symbol_table), ZEND_STRL("_SERVER"))) == NULL) {
                snprintf(msg, len, "%s", "no_data");
                return FALSE;
        }   

        if ((data = zend_hash_str_find(Z_ARRVAL_P(super_global), (name), strlen(name))) == NULL) {
                snprintf(msg, len, "%s", "no_data");
                return FALSE;
        }   

        str_data = zend_string_init(Z_STRVAL_P(data), Z_STRLEN_P(data), 0); 
        snprintf(msg, len, "%s", ZSTR_VAL(str_data));
        zend_string_release(str_data);

        return TRUE;
}

char* snprintf_heap(char *fmt, ...)
{
	char *msg = NULL;
	int msg_size;
	va_list ap1, ap2;

	va_start(ap1, fmt);
	va_copy(ap2, ap1);

	msg_size = vsnprintf(NULL ,0, fmt, ap1) + 1;
	msg = malloc(msg_size);
	va_end(ap1);

	vsnprintf(msg, msg_size, fmt, ap2);
	va_end(ap2);

	return msg;
}
