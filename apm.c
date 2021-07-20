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
	APM_G(start_time) = time(NULL);

	struct timeval s_time; 
	gettimeofday(&s_time, NULL);
	APM_G(start_time_ms) = s_time.tv_sec*1000LL + s_time.tv_usec/1000;
	//APM_G(start_time) = current_timestamp();
}

PHP_RSHUTDOWN_FUNCTION(apm)
{
	//check_time
	struct timeval e_time; 
	gettimeofday(&e_time, NULL);
	APM_G(end_time_ms) = e_time.tv_sec*1000LL + e_time.tv_usec/1000;
	//APM_G(end_time) = current_timestamp();

	//send_data
	char msg[BUF_SIZE];
	snprintf(msg, BUF_SIZE, "%d, %lld\n", APM_G(start_time), APM_G(end_time_ms) - APM_G(start_time_ms)); //format: start_time, end_time, ....
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
	
	//format: start_time, end_time, ....
	sendto(client_socket, msg, strlen(msg), 0, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
	close(client_socket);
}

/* make error
long long current_timestamp() {
    struct timeval te; 
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // calculate milliseconds

    return milliseconds;
}
*/
