#define PHP_APM_VERSION "0.1"
#define PHP_APM_EXTNAME "apm"

extern zend_module_entry apm_module_entry;
#define phpext_apm_ptr &apm_module_entry

#ifdef PHP_WIN32
#	define PHP_EXTNAME_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_EXTNAME_API __attribute__ ((visibility("default")))
#else
#	define PHP_EXTNAME_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

ZEND_BEGIN_MODULE_GLOBALS(apm)
	unsigned int enabled;
	char *server_host;
	unsigned int server_port;
	time_t start_time;
	time_t end_time;
	FILE *fh; //tobe udp
ZEND_END_MODULE_GLOBALS(apm)
	
//ZEND_DECLARE_MODULE_GLOBALS(apm)
ZEND_EXTERN_MODULE_GLOBALS(apm)

#ifdef ZTS
#define APM_G(v) TSRMG(apm_globals_id, zend_apm_globals *, v)
#else
#define APM_G(v) (apm_globals.v)
#endif
	

PHP_MINIT_FUNCTION(apm);
PHP_MSHUTDOWN_FUNCTION(apm);
PHP_RINIT_FUNCTION(apm);
PHP_RSHUTDOWN_FUNCTION(apm);
PHP_MINFO_FUNCTION(apm);

//PHP_FUNCTION(apm_func);
