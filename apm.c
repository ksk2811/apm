#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "php.h"
#include "php_ini.h"
#include "php_apm.h"
#include "send.h"

//#include <send.h> //별도의 파일로 빼야한다
#include <stdio.h>
#include <string.h>
#include <time.h>
#define TMP_FILE_PATH "/tmp/cvs_data"
#define TMP_BUF_SIZE 10

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
	//안먹힌다;
	PHP_INI_ENTRY("apm.enabled", 0, PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("apm.server_host", "127.0.0.1", PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("apm.server_port", "8888", PHP_INI_ALL, NULL)
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
	//apache 시작 할 때 한번 실행되는걸로 보인다.
}

PHP_MSHUTDOWN_FUNCTION(apm)
{
	//apache 종료 할 때 한번 실행되는걸로 보인다.
}

PHP_RINIT_FUNCTION(apm)
{
	//파일에 입력하는 부분을 별도의 파일로 분리해야한다
	php_printf("start\n");

	//설정이 안읽힌다. 기존에 설정은 읽히지만 새로추가한 건 읽히지 않는다.
	APM_G(enabled) = INI_INT("apm.enabled");
	APM_G(server_host) = INI_STR("apm.server_host");
	APM_G(server_port) = INI_INT("apm.server_port");

	APM_G(start_time) = time(NULL);
	APM_G(fh) = fopen(TMP_FILE_PATH, "a+");

	php_printf("start time is %11d\n", APM_G(start_time));
	php_printf("server host is %s\n", APM_G(server_host));
	php_printf("server port is %d\n", APM_G(server_port));
	php_printf("enabled is %d\n", APM_G(enabled));
	fprintf(APM_G(fh), "start %11d\n", APM_G(start_time)); //안먹힌다.
}

PHP_RSHUTDOWN_FUNCTION(apm)
{
	//send_data(1);
	APM_G(end_time) = time(NULL);
	php_printf("end time is %11d\n", APM_G(end_time));
	fprintf(APM_G(fh), "end %11d\n", APM_G(end_time)); //안먹힌다

	fclose(APM_G(fh));
	php_printf("fisnsh");
}
