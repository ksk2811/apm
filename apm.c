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
	//check_time
	APM_G(start_time_ms) = current_timestamp();
}

PHP_RSHUTDOWN_FUNCTION(apm)
{
	//check_time
	APM_G(end_time_ms) = current_timestamp();
	
	//make data
	char msg[BUF_SIZE];
	char uri[BUF_SIZE];
	char host[BUF_SIZE];
	char ip[BUF_SIZE];
	char method[BUF_SIZE];

	get_super_global(host, BUF_SIZE, "HTTP_HOST");
	get_super_global(uri, BUF_SIZE, "REQUEST_URI");
	get_super_global(ip, BUF_SIZE, "REMOTE_ADDR");
	get_super_global(method, BUF_SIZE, "REQUEST_METHOD");
	//timestamp(milliseconds), tps(milliseconds), host, ip, method)
	snprintf(msg, BUF_SIZE, "%lld, %lld, %s%s, %s, %s\n",
			APM_G(start_time_ms),
			APM_G(end_time_ms) - APM_G(start_time_ms),
			host,
			uri,
			ip,
			method);

	//send data
	send_data(msg);
}

void send_data(char *msg)
{
	int client_socket;
	struct sockaddr_in serverAddress;
	int server_addr_size;

	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	inet_aton(APM_G(server_host), (struct in_addr*) &serverAddress.sin_addr.s_addr);
	serverAddress.sin_port = htons(APM_G(server_port));
	client_socket = socket(PF_INET, SOCK_DGRAM, 0);
	server_addr_size = sizeof(serverAddress);
	
	sendto(client_socket, msg, strlen(msg), 0, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
	close(client_socket);
}

time_t current_timestamp()
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
		return FALSE;
	}

	if ((data = zend_hash_str_find(Z_ARRVAL_P(super_global), (name), strlen(name))) == NULL) {
		return FALSE;
	}

	str_data = zend_string_init(Z_STRVAL_P(data), Z_STRLEN_P(data), 0); //free가 필요한 함수인지 조사
	snprintf(msg, len, "%s", ZSTR_VAL(str_data));

	return TRUE;

}
