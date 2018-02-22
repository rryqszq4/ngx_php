/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */

#ifndef NGX_HTTP_PHP_MODULE_H
#define NGX_HTTP_PHP_MODULE_H

#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_config.h>
#include <ngx_event.h>
#include <nginx.h>

#if defined(NDK) && NDK
#include <ndk.h>
#endif

#include "ngx_php_thread.h"
#include "ngx_php_thread_pool.h"
#include "ngx_php_uthread.h"

#include "ngx_http_php_core.h"

#define NGX_HTTP_PHP_MODULE_NAME "ngx_php"
#define NGX_HTTP_PHP_MODULE_VERSION	"0.0.10"

extern ngx_module_t	ngx_http_php_module;
ngx_http_request_t *ngx_php_request;

extern ngx_php_thread_pool_queue_t  ngx_php_thread_pool_done;
extern ngx_php_thread_pool_queue_t  ngx_php_thread_pool_running;
extern ngx_uint_t                   ngx_php_thread_pool_task_id;
extern ngx_atomic_t                 ngx_php_thread_pool_done_lock;
extern ngx_uint_t 					ngx_php_uthread_max;

typedef struct ngx_http_php_main_conf_s {

	ngx_str_t ini_path;
	ngx_http_php_code_t *init_code;
	ngx_http_php_code_t *init_inline_code;

	unsigned enabled_rewrite_handler:1;
	unsigned enabled_access_handler:1;
	unsigned enabled_content_handler:1;
	unsigned enabled_content_async_handler:1;
	unsigned enabled_log_handler:1;

	ngx_http_php_state_t *state;

	ngx_array_t thread_pools;

} ngx_http_php_main_conf_t;

typedef struct ngx_http_php_loc_conf_s {
	ngx_str_t document_root;

	ngx_http_upstream_conf_t upstream;

	ngx_http_php_code_t *rewrite_code;
	ngx_http_php_code_t *rewrite_inline_code;
	ngx_http_php_code_t *access_code;
	ngx_http_php_code_t *access_inline_code;
	ngx_http_php_code_t *content_code;
	ngx_http_php_code_t *content_inline_code;
	ngx_http_php_code_t *content_async_inline_code;
	ngx_http_php_code_t *log_code;
	ngx_http_php_code_t *log_inline_code;

	ngx_int_t (*rewrite_handler)(ngx_http_request_t *r);
	ngx_int_t (*access_handler)(ngx_http_request_t *r);
	ngx_int_t (*content_handler)(ngx_http_request_t *r);
	ngx_int_t (*content_async_handler)(ngx_http_request_t *r);
	ngx_int_t (*log_handler)(ngx_http_request_t *r);

} ngx_http_php_loc_conf_t;


#endif