#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "php.h"
#include "php_ini.h"
#include "php_apm.h"
#include "tool.h"

//ZEND_DECLARE_MODULE_GLOBALS(apm);
ZEND_EXTERN_MODULE_GLOBALS(apm);

void send_data(char *msg)
{
	int client_socket;
	struct sockaddr_in serverAddress;
	int server_addr_size;
	int msg_len = 0;
	int real_send_msg_len = 0;
	char log_msg[BUF_SIZE];

	if (APM_G(sock_type) == 1) { //udp
		php_printf("udp");
		if ((client_socket = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
			//php_errors.log 시작할때 warning 로그는 생기는데 이건 안나온다...
			php_log_err("can't create socket\n");
			return;
		}

		memset(&serverAddress, 0, sizeof(serverAddress));
		serverAddress.sin_family = AF_INET;
		inet_aton(APM_G(server_host), (struct in_addr*) &serverAddress.sin_addr.s_addr);
		serverAddress.sin_port = htons(APM_G(server_port));
		server_addr_size = sizeof(serverAddress);

		msg_len = strlen(msg);
		real_send_msg_len = sendto(client_socket, msg, msg_len, 0, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
		if (msg_len != real_send_msg_len) {
			snprintf(log_msg, BUF_SIZE, "can't send data(%d/%d)", msg_len, real_send_msg_len);
			php_log_err(log_msg);
		}
		close(client_socket);
	} else { //tcp
		if (client_socket = socket(PF_INET, SOCK_STREAM, 0) == -1) {
			php_log_err("can't create socket\n");
			return;
		}
		memset(&serverAddress, 0, sizeof(serverAddress));

		serverAddress.sin_family = AF_INET;
		serverAddress.sin_port = htons(APM_G(server_port));
		inet_aton(APM_G(server_host), (struct in_addr*) &serverAddress.sin_addr.s_addr);

		if (connect(client_socket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
			php_log_err("can't conncet server");
			return;
		}

		write(client_socket, msg, sizeof(msg));
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

	str_data = zend_string_init(Z_STRVAL_P(data), Z_STRLEN_P(data), 0); //free가 필요한 함수인지 조사
	snprintf(msg, len, "%s", ZSTR_VAL(str_data));

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

