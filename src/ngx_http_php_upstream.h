/**
 *    Copyright(c) 2016-2017 rryqszq4
 *
 *
 */

#ifndef NGX_HTTP_PHP_UPSTREAM_H
#define NGX_HTTP_PHP_UPSTREAM_H

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include <ngx_event_connect.h>
#include <ngx_event_pipe.h>
#include <ngx_http.h>

#include "ngx_http_php_socket_tcp.h"

ngx_int_t ngx_http_php_upstream_create(ngx_http_request_t *r);
void ngx_http_php_upstream_init(ngx_http_request_t *r);
void ngx_http_php_upstream_connect(ngx_http_request_t *r,
    ngx_http_upstream_t *u);
void ngx_http_php_upstream_send_request(ngx_http_request_t *r,
    ngx_http_upstream_t *u);
void ngx_http_php_upstream_process_header(ngx_http_request_t *r,
    ngx_http_upstream_t *u);
void ngx_http_php_upstream_finalize_request(ngx_http_request_t *r,
    ngx_http_upstream_t *u, ngx_int_t rc);

#endif