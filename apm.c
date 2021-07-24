#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "php.h"
#include "php_ini.h"
#include "php_apm.h"

ZEND_DECLARE_MODULE_GLOBALS(apm);
//ZEND_EXTERN_MODULE_GLOBALS(apm);

/*
 현재는 필요없다.
static zend_function_entry apm_functions[] = {
	PHP_FE(apm_func, NULL)
	{NULL, NULL, NULL}
};
*/

zend_module_entry apm_module_entry = {
	STANDARD_MODULE_HEADER,
	PHP_APM_EXTNAME,
	NULL, //apm_functions,
	PHP_MINIT(apm),
	PHP_MSHUTDOWN(apm),
	PHP_RINIT(apm),
	PHP_RSHUTDOWN(apm),
	PHP_MINFO(apm),
	PHP_APM_VERSION,
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_APM
ZEND_GET_MODULE(apm)
#endif

PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("apm.enabled", "0", PHP_INI_ALL, OnUpdateLong, enabled, zend_apm_globals, apm_globals)
	STD_PHP_INI_ENTRY("apm.server_port", "8888", PHP_INI_ALL, OnUpdateLong, server_port, zend_apm_globals, apm_globals)
	STD_PHP_INI_ENTRY("apm.server_host", "localhost", PHP_INI_ALL, OnUpdateStringUnempty, server_host, zend_apm_globals, apm_globals)
PHP_INI_END()

/*
PHP_FUNCTION(apm_func)
{
	char *name;
	size_t len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &len) == FAILURE) {
		php_printf("no parameter given");
		RETURN_NULL();
	}

	php_printf("Hello %s", name);

	RETURN_TRUE;
}
*/

PHP_MINFO_FUNCTION(apm)
{
//	php_info_print_table_start();
//	php_info_print_table_header(2, "apm support", "enabled");
//	php_info_print_table_end();
	DISPLAY_INI_ENTRIES();
}

PHP_MINIT_FUNCTION(apm)
{
	//apache 시작 할 때 한번 실행된다.
	REGISTER_INI_ENTRIES();
}

PHP_MSHUTDOWN_FUNCTION(apm)
{
	//apache 종료 할 때 한번 실행된다.
	UNREGISTER_INI_ENTRIES();
}

PHP_RINIT_FUNCTION(apm)
{
	//php_code에서 $_SERVER를 사용안해도 접근할 수 있게 해줌
	zend_is_auto_global_str(ZEND_STRL("_SERVER")); 

	//check_time
	APM_G(start_time_ms) = get_millisec();
	
	//get data
	get_super_global(APM_G(host), BUF_SIZE, "HTTP_HOST");
	get_super_global(APM_G(uri), BUF_SIZE, "REQUEST_URI");
	get_super_global(APM_G(ip), BUF_SIZE, "REMOTE_ADDR");
	get_super_global(APM_G(method), BUF_SIZE, "REQUEST_METHOD");
}

PHP_RSHUTDOWN_FUNCTION(apm)
{
	//check_time
	APM_G(end_time_ms) = get_millisec();

	char msg[BUF_SIZE];
	snprintf(msg, BUF_SIZE, "%ld, %ld, %s%s, %s, %s\n",
		APM_G(start_time_ms),
		APM_G(end_time_ms) - APM_G(start_time_ms),
		APM_G(host),
		APM_G(uri),
		APM_G(ip),
		APM_G(method));

	//send data
	send_data(msg);
}

void send_data(char *msg)
{
	int client_socket;
	struct sockaddr_in serverAddress;
	int server_addr_size;
	int msg_len = 0;
	int real_send_msg_len = 0;
	char log_msg[BUF_SIZE];

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
