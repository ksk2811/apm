#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "php.h"
#include "php_ini.h"
#include "php_apm.h"

//#include <send.h> //별도의 파일로 빼야한다

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
	//php_printf("start: global val start time(%11d), enabled(%d), server host(%s), server port(%d)\n", APM_G(start_time), APM_G(enabled), APM_G(server_host), APM_G(server_port));
	APM_G(start_time) = time(NULL);
}

PHP_RSHUTDOWN_FUNCTION(apm)
{
	//php_printf("finish: global val finsh time(%11d), enabled(%d), server host(%s), server port(%d)\n", APM_G(end_time), APM_G(enabled), APM_G(server_host), APM_G(server_port));
	APM_G(end_time) = time(NULL);

	//udp start
	int client_socket;
	struct sockaddr_in serverAddress;
	int server_addr_size;
	char msg[BUF_SIZE];

	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	inet_aton(APM_G(server_host), (struct in_addr*) &serverAddress.sin_addr.s_addr);
	serverAddress.sin_port = htons(APM_G(server_port));
	client_socket = socket(PF_INET, SOCK_DGRAM, 0);
	server_addr_size = sizeof(serverAddress);
	
	//start_time, end_time, ....
	snprintf(msg, BUF_SIZE, "%d, %d\n", APM_G(start_time), APM_G(end_time));
	sendto(client_socket, msg, strlen(msg), 0, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
	close(client_socket);
	//udp finish
}
