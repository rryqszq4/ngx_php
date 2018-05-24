/**
 *    Copyright(c) 2016-2017 rryqszq4
 *
 *
 */

#ifndef NGX_HTTP_PHP_SOCKET_TCP_H
#define NGX_HTTP_PHP_SOCKET_TCP_H

#include "ngx_http_php_module.h"
#include "ngx_http_php_upstream.h"

ngx_int_t ngx_http_php_socket_tcp_run(ngx_http_request_t *r);

ngx_int_t ngx_http_php_socket_tcp_create_request(ngx_http_request_t *r);

ngx_int_t ngx_http_php_socket_tcp_reinit_request(ngx_http_request_t *r);

ngx_int_t ngx_http_php_socket_tcp_process_header(ngx_http_request_t *r);

ngx_int_t ngx_http_php_socket_tcp_receive_parse(ngx_http_request_t *r);

void ngx_http_php_socket_tcp_abort_request(ngx_http_request_t *r);

void ngx_http_php_socket_tcp_finalize_request(ngx_http_request_t *r, ngx_int_t rc);

ngx_int_t ngx_http_php_socket_tcp_filter_init(void *data);

ngx_int_t ngx_http_php_socket_tcp_filter(void *data, ssize_t bytes);

ngx_int_t ngx_http_php_socket_tcp_rediscovery(ngx_http_request_t *r);

ngx_int_t ngx_http_php_socket_tcp_handler(ngx_http_request_t *r);


// socket tcp in thread pool 
ngx_int_t ngx_http_php_socket_tcp_thread_run(ngx_http_request_t *r);
ngx_int_t ngx_http_php_socket_tcp_thread_create_request(ngx_http_request_t *r);
ngx_int_t ngx_http_php_socket_tcp_thread_reinit_request(ngx_http_request_t *r);
ngx_int_t ngx_http_php_socket_tcp_thread_process_header(ngx_http_request_t *r);
ngx_int_t ngx_http_php_socket_tcp_thread_receive_parse(ngx_http_request_t *r);
void ngx_http_php_socket_tcp_thread_abort_request(ngx_http_request_t *r);
void ngx_http_php_socket_tcp_thread_finalize_request(ngx_http_request_t *r, ngx_int_t rc);
ngx_int_t ngx_http_php_socket_tcp_thread_filter_init(void *data);
ngx_int_t ngx_http_php_socket_tcp_thread_filter(void *data, ssize_t bytes);
ngx_int_t ngx_http_php_socket_tcp_thread_rediscovery(ngx_http_request_t *r);

#endif