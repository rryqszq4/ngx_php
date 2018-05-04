/**
 *    Copyright(c) 2016-2017 rryqszq4
 *
 *
 */

#ifndef _PHP_NGX_H_
#define _PHP_NGX_H_

#include <php.h>
#include <SAPI.h>
#include <php_main.h>
#include <php_variables.h>
#include <php_ini.h>
#include <zend_ini.h>
#include <zend_exceptions.h>
#include <ext/standard/php_standard.h>
#include <ext/standard/info.h>

#include "../ngx_http_php_request.h"

int php_ngx_module_init();
void php_ngx_module_shutdown(TSRMLS_D);

int php_ngx_request_init(TSRMLS_D);
void php_ngx_request_shutdown(TSRMLS_D);

extern sapi_module_struct php_ngx_module;

extern zend_module_entry php_ngx_module_entry;
#define phpext_php_ngx_ptr &php_ngx_module_entry

#ifdef PHP_WIN32
#	define PHP_PHP_NGX_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_PHP_NGX_API __attribute__ ((visibility("default")))
#else
#	define PHP_PHP_NGX_API
#endif

#ifdef ZTS
#include <TSRM.h>
#endif

PHP_MINIT_FUNCTION(php_ngx);
PHP_MSHUTDOWN_FUNCTION(php_ngx);
PHP_RINIT_FUNCTION(php_ngx);
PHP_RSHUTDOWN_FUNCTION(php_ngx);
PHP_MINFO_FUNCTION(php_ngx);

PHP_FUNCTION(confirm_php_ngx_compiled);	/* For testing, remove later. */

 
/* 	Declare any global variables you may need between the BEGIN
	and END macros here:     
*/
ZEND_BEGIN_MODULE_GLOBALS(php_ngx)
	long  global_value;
	char *global_string;
   ngx_http_request_t *global_r;
ZEND_END_MODULE_GLOBALS(php_ngx)

extern ZEND_DECLARE_MODULE_GLOBALS(php_ngx);


/* In every utility function you add that needs to use variables 
   in php_php_ngx_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as PHP_NGX_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define PHP_NGX_G(v) TSRMG(php_ngx_globals_id, zend_php_ngx_globals *, v)
#else
#define PHP_NGX_G(v) (php_ngx_globals.v)
#endif

#endif