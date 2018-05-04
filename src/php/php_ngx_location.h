/**
 *    Copyright(c) 2016-2017 rryqszq4
 *
 *
 */

#ifndef _PHP_NGX_LOCATION_H_
#define _PHP_NGX_LOCATION_H_

#include <ngx_http.h>
 
#include <php.h>
#include <php_ini.h>
#include <ext/standard/info.h>
#include <zend_closures.h>

#define COUNT_RECURSIVE     1

//PHP_METHOD(ngx_location, __construct);
//PHP_METHOD(ngx_location, capture_async);
//PHP_METHOD(ngx_location, capture_multi_async);

PHP_METHOD(ngx_location, capture);
PHP_METHOD(ngx_location, capture_multi);

void ngx_location_init(int module_number TSRMLS_DC);

#endif