/**
 *    Copyright(c) 2016-2017 rryqszq4
 *
 *
 */

#ifndef _PHP_NGX_TCP_H_
#define _PHP_NGX_TCP_H_

#include <ngx_http.h>
 
#include <php.h>
#include <php_ini.h>
#include <ext/standard/info.h>

PHP_METHOD(ngx_tcp, __construct);
PHP_METHOD(ngx_tcp, connect);
PHP_METHOD(ngx_tcp, send);
PHP_METHOD(ngx_tcp, receive);
PHP_METHOD(ngx_tcp, close);
PHP_METHOD(ngx_tcp, settimeout);
PHP_METHOD(ngx_tcp, setkeepalive);

void ngx_tcp_init(int module_number TSRMLS_DC);

#endif