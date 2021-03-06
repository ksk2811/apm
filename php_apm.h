#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/timeb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
# include <sys/resource.h>

#define PHP_APM_VERSION "0.1"
#define PHP_APM_EXTNAME "apm"
#define IP_LEN 16
#define METHOD_LEN 8
#define FALSE 0
#define TRUE 1
#define SEC_TO_USEC(sec) ((sec) * 1000000.00)

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
	unsigned int sock_type;


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

