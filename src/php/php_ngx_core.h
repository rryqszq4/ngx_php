/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */

#ifndef _PHP_NGX_CORE_H_
#define _PHP_NGX_CORE_H_

#include <ngx_http.h>

#include <php.h>
#include <php_ini.h>
#include <ext/standard/info.h>

PHP_METHOD(ngx, _exit);
PHP_METHOD(ngx, query_args);
PHP_METHOD(ngx, post_args);
PHP_METHOD(ngx, sleep);

void php_ngx_core_init(int module_number TSRMLS_DC);

void php_co_ngx_init(int module_number TSRMLS_DC);

void (*ori_execute_ex)(zend_execute_data *execute_data TSRMLS_DC);
void ngx_execute_ex(zend_execute_data *execute_data TSRMLS_DC);

void ngx_execute_internal(zend_execute_data *execute_data_ptr, zend_fcall_info *fci, int return_value_used TSRMLS_DC);
#endif