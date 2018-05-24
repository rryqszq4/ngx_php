/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */

#ifndef _PHP_NGX_EXECUTE_H_
#define _PHP_NGX_EXECUTE_H_

#include <ngx_http.h>

#include <php.h>
#include <php_ini.h>
#include <ext/standard/info.h>

void (*ori_execute_ex)(zend_execute_data *execute_data TSRMLS_DC);
void ngx_coexecute_ex(zend_execute_data *execute_data TSRMLS_DC);

void ngx_execute_internal(zend_execute_data *execute_data_ptr, zend_fcall_info *fci, int return_value_used TSRMLS_DC);

#endif