/**
 *    Copyright(c) 2016-2017 rryqszq4
 *
 *
 */


#ifndef NGX_HTTP_PHP_THREAD_HANDLER_H
#define NGX_HTTP_PHP_THREAD_HANDLER_H

#include <nginx.h>
#include <ngx_http.h>

#include "ngx_http_php_module.h"

ngx_int_t ngx_http_php_content_inline_thread_handler(ngx_http_request_t *r);
ngx_int_t ngx_http_php_content_file_thread_handler(ngx_http_request_t *r);

#endif