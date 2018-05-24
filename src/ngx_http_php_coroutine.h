/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */

#ifndef _NGX_HTTP_PHP_COROUTINE_H_
#define _NGX_HTTP_PHP_COROUTINE_H_

#include "ngx_php_coroutine.h"
#include "ngx_http_php_module.h"

ngx_php_coroutine_t *ngx_http_php_coroutine_alloc(ngx_http_request_t *r);

ngx_int_t ngx_http_php_coroutine_run(ngx_http_request_t *r);

ngx_int_t ngx_http_php_coroutine_yield(ngx_http_request_t *r);

ngx_int_t ngx_http_php_coroutine_resume(ngx_http_request_t *r);

#endif