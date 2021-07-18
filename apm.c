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
	//PHP_INI 등록을 안하면 불러올수가 없다
	//두번째 파라미터는 설정파일에 없는경우 적용되는 값
	//PHP_INI_ENTRY("apm.enabled", "1", PHP_INI_ALL, NULL)
	//PHP_INI_ENTRY("apm.server_host", "localhost", PHP_INI_ALL, NULL)
	//PHP_INI_ENTRY("apm.server_port", "9999", PHP_INI_ALL, NULL)

	//APM_G 매크로 대신 설정을 바로 전역변수에 연결
	STD_PHP_INI_ENTRY("apm.enabled", "0", PHP_INI_ALL, OnUpdateLong, enabled, zend_apm_globals, apm_globals)
	STD_PHP_INI_ENTRY("apm.server_port", "9999", PHP_INI_ALL, OnUpdateLong, server_port, zend_apm_globals, apm_globals)
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
	//파일에 입력하는 부분을 별도의 파일로 분리해야한다
	//설정이 안읽힌다. 기존에 설정은 읽히지만 새로추가한 건 읽히지 않는다.
	//APM_G(enabled) = INI_INT("apm.enabled");
	//APM_G(server_host) = INI_STR("apm.server_host");
	//APM_G(server_port) = INI_INT("apm.server_port");
	APM_G(start_time) = time(NULL);
	APM_G(fh) = fopen(TMP_FILE_PATH, "a+");

	//ini_set 으러 변경한 값을 가져온다.
	//php_printf("ini val server host is %s\n", INI_STR("apm.server_host"));
	//php_printf("ini val server port is %d\n", INI_INT("apm.server_port"));
	//php_printf("ini val enabled is %d\n", INI_INT("apm.enabled"));

	//ini_set과 상관없이 설정에 입력된 값을 가져온다.
	//php_printf("ini org val server host is %s\n", INI_ORIG_STR("apm.server_host"));
	//php_printf("ini org val server port is %d\n", INI_ORIG_INT("apm.server_port"));
	//php_printf("ini org val enabled is %d\n", INI_ORIG_INT("apm.enabled"));

	php_printf("start: global val start time(%11d), enabled(%d), server host(%s), server port(%d)\n", APM_G(start_time), APM_G(enabled), APM_G(server_host), APM_G(server_port));
	fprintf(APM_G(fh), "start %d\n", APM_G(start_time)); //안먹힌다.
}

PHP_RSHUTDOWN_FUNCTION(apm)
{
	//send_data(1);
	APM_G(end_time) = time(NULL);
	php_printf("finish: global val finsh time(%11d), enabled(%d), server host(%s), server port(%d)\n", APM_G(end_time), APM_G(enabled), APM_G(server_host), APM_G(server_port));

	fprintf(APM_G(fh), "end %d\n", APM_G(end_time)); //안먹힌다
	fclose(APM_G(fh));
}
