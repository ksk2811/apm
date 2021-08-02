#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "php.h"
#include "php_ini.h"
#include "php_apm.h"
#include "SAPI.h"
#include "tool.h" 

ZEND_DECLARE_MODULE_GLOBALS(apm);
//ZEND_EXTERN_MODULE_GLOBALS(apm);

struct rusage start_usage;
struct rusage end_usage;
time_t start_time_ms;
time_t end_time_ms;
char *uri;
char *host;
char ip[IP_LEN];
char method[METHOD_LEN];

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
	STD_PHP_INI_ENTRY("apm.sock_type", "1", PHP_INI_ALL, OnUpdateLong, sock_type, zend_apm_globals, apm_globals)
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
	if (APM_G(enabled) != 1) {
		return SUCCESS;
	}

	//check cpu
	memset(&start_usage, 0, sizeof(struct rusage));
	if (getrusage(RUSAGE_SELF, &start_usage) != 0) {
		//todo error	
	}
	
	//php_code에서 $_SERVER를 사용안해도 접근할 수 있게 해줌
	zend_is_auto_global_str(ZEND_STRL("_SERVER")); 

	//check_time
	start_time_ms = get_millisec();
	
	//get data
	get_heap_super_global(&host, "HTTP_HOST");
	get_heap_super_global(&uri, "REQUEST_URI");
	get_super_global(ip, IP_LEN, "REMOTE_ADDR");
	get_super_global(method, METHOD_LEN, "REQUEST_METHOD");

	//헤더 데이터 임시 코드
	/*
	zend_llist_position pos;
	sapi_header_struct* h;
	for (h = (sapi_header_struct*)zend_llist_get_first_ex(&SG(sapi_headers).headers, &pos); 
		h; 
		h = (sapi_header_struct*)zend_llist_get_next_ex(&SG(sapi_headers).headers, &pos)) 
	{
		php_printf("SAPI! %.*s <br/>", h->header_len, h->header);
	}
	*/
}

PHP_RSHUTDOWN_FUNCTION(apm)
{
	if (APM_G(enabled) != 1) {
		 return SUCCESS;
	}

	memset(&end_usage, 0, sizeof(struct rusage));
	if (getrusage(RUSAGE_SELF, &end_usage) != 0) {
		//todo error	
	}

	//check cpu
	time_t usr_cpu = ((end_usage.ru_utime.tv_sec - start_usage.ru_utime.tv_sec) * 1000000) + end_usage.ru_utime.tv_usec - start_usage.ru_utime.tv_usec;
	time_t sys_cpu = ((end_usage.ru_stime.tv_sec - start_usage.ru_stime.tv_sec) * 1000000) + end_usage.ru_stime.tv_usec;// - start_usage.ru_stime.tv_usec;
	
	//check_time
	end_time_ms = get_millisec();
	double peak_mem_usage = zend_memory_peak_usage(1);

	char *msg = NULL;
	int msg_size = snprintf(NULL, 0, "%ld, %ld, %ld, %s%s, %s, %s, %.0f, %ld, %ld",
		start_time_ms, //millisecond
		end_time_ms, //millisecond
		end_time_ms - start_time_ms, //millisecond
		host,
		uri,
		ip,
		method,
		peak_mem_usage, //byte
		usr_cpu, //micro second
		sys_cpu //micro second
	) + 1;
	msg = malloc(msg_size);

	snprintf(msg, msg_size, "%ld, %ld, %ld, %s%s, %s, %s, %.0f, %ld, %ld",
		start_time_ms, //millisecond
		end_time_ms, //millisecond
		end_time_ms - start_time_ms, //millisecond
		host,
		uri,
		ip,
		method,
		peak_mem_usage, //byte
		usr_cpu, //micro second
		sys_cpu //micro second
	);
	
//	char *test = NULL;
//	php_printf("before emalloc");
//	php_printf("emalloc result is %d\n", emalloc(999999999999999999999)); 
//	php_printf("after emalloc");

//	php_printf("before malloc");
//	php_printf("malloc result is %d\n", malloc(999999999999999999999)); 
//	php_printf("after malloc");

	send_data(msg);
	free(host); host = NULL;
	free(uri); uri = NULL;
	free(msg); msg = NULL;
	memset(&ip, 0, IP_LEN);
	memset(&method, 0, METHOD_LEN);
}
