/**
 *    Copyright(c) 2016-2017 rryqszq4
 *
 *
 */

#ifndef NGX_HTTP_PHP_CORE_H
#define NGX_HTTP_PHP_CORE_H

#include <ngx_http.h>
#include <php_embed.h>
#include "php/php_ngx.h"

#include "ngx_php_thread.h"
#include "ngx_php_thread_pool.h"
#include "ngx_php_uthread.h"

extern ngx_http_request_t *ngx_php_request;

typedef struct ngx_http_php_state_s {
	unsigned php_init;
	unsigned php_shutdown;
} ngx_http_php_state_t;

typedef enum code_type_s {
	NGX_HTTP_PHP_CODE_TYPE_FILE,
	NGX_HTTP_PHP_CODE_TYPE_STRING
} code_type_t;

typedef struct ngx_http_php_code_s {
	union code {
		char *file;
		char *string;
	} code;
	code_type_t code_type;
} ngx_http_php_code_t;

#if defined(NDK) && NDK
typedef struct ngx_http_php_set_var_data_s {
	size_t size;
	ngx_str_t var_name;
	ngx_str_t script;
	ngx_http_php_code_t *code;
	ngx_str_t result;
} ngx_http_php_set_var_data_t;
#endif

typedef struct ngx_http_php_rputs_chain_list_s {
	ngx_chain_t **last;
	ngx_chain_t *out;
} ngx_http_php_rputs_chain_list_t;

typedef struct ngx_http_php_capture_node_s {
	ngx_str_t capture_uri;
	ngx_buf_t *capture_buf;
	ngx_str_t capture_str;
} ngx_http_php_capture_node_t;

typedef struct ngx_http_php_ctx_s {
	ngx_http_php_rputs_chain_list_t *rputs_chain;
	size_t body_length;
	ngx_str_t request_body_ctx;
	unsigned request_body_more : 1;		//post request flag

	unsigned enable_async : 1;
	unsigned enable_subrequest : 1;
	unsigned enable_thread : 1;
	unsigned is_capture_multi : 1;
	unsigned is_capture_multi_complete : 1;

	ngx_str_t capture_uri;
	ngx_buf_t *capture_buf;
	ngx_str_t capture_str;
	zval *closure;

	ngx_array_t *capture_multi;
	ngx_uint_t capture_multi_complete_total;

	unsigned enable_upstream : 1;
	unsigned enable_upstream_continue : 1;

	unsigned read_or_write : 1;

	ngx_str_t host;
	int port;
	ngx_str_t send_buf;
	ngx_http_status_t receive_status;
	ngx_str_t receive_buf;
	ngx_int_t receive_stat;
	ngx_int_t receive_total;
	ngx_list_t *receive_list;
	ngx_msec_t timeout;

	pthread_mutex_t mutex;
	pthread_cond_t cond;
	pthread_t pthread_id;

	sem_t sem_thread;

	ngx_int_t error;

	ngx_http_request_t* request;

	unsigned enable_sleep : 1;
	ngx_int_t delay_time;
	ngx_event_t sleep;

	ngx_php_thread_task_t *thread_task;
	ngx_php_thread_pool_t *thread_pool;
	ngx_int_t thread_wait;

	ngx_php_uthread_t *uthread;
	ngx_php_uthread_t *rewrite_uthread;
	ngx_php_uthread_t *access_uthread;
	ngx_php_uthread_t *content_uthread;

	unsigned rewrite_phase : 1;
	unsigned access_phase : 1;
	unsigned content_phase : 1;

	ngx_int_t phase_status;

	void **uthread_ctx;

	zval *generator_closure;
	
} ngx_http_php_ctx_t;


ngx_http_php_code_t *ngx_http_php_code_from_file(ngx_pool_t *pool, ngx_str_t *code_file_path);
ngx_http_php_code_t *ngx_http_php_code_from_string(ngx_pool_t *pool, ngx_str_t *code_str);

#define NGX_HTTP_PHP_NGX_INIT ngx_http_php_request_init(r TSRMLS_CC); 	\
		php_ngx_request_init(TSRMLS_C);									\
		zend_first_try {

#define NGX_HTTP_PHP_NGX_SHUTDOWN } zend_catch {		\
		} zend_end_try();								\
		ngx_http_php_request_clean(TSRMLS_C);			\
		php_ngx_request_shutdown(TSRMLS_C);

#define NGX_HTTP_PHP_R_INIT(r) ngx_http_php_request_init(r TSRMLS_CC); 	\
		php_ngx_request_init(TSRMLS_C);									\
		PHP_NGX_G(global_r) = r;

#define NGX_HTTP_PHP_R_SHUTDOWN(r) ngx_http_php_request_clean(TSRMLS_C);	\
		php_ngx_request_shutdown(TSRMLS_C);

// php_ngx run
ngx_int_t ngx_php_embed_run(ngx_http_request_t *r, ngx_http_php_code_t *code);
ngx_int_t ngx_php_ngx_run(ngx_http_request_t *r, ngx_http_php_state_t *state, ngx_http_php_code_t *code);
ngx_int_t ngx_php_eval_code(ngx_http_request_t *r, ngx_http_php_state_t *state, ngx_http_php_code_t *code TSRMLS_DC);
ngx_int_t ngx_php_eval_file(ngx_http_request_t *r, ngx_http_php_state_t *state, ngx_http_php_code_t *code TSRMLS_DC);

ngx_int_t ngx_php_get_request_status(TSRMLS_D);
ngx_int_t ngx_php_set_request_status(ngx_int_t rc TSRMLS_DC);

// php_ngx sapi call_back
int ngx_http_php_code_ub_write(const char *str, unsigned int str_length TSRMLS_DC);
void ngx_http_php_code_flush(void *server_context);
void ngx_http_php_code_log_message(char *message);
void ngx_http_php_code_register_server_variables(zval *track_vars_array TSRMLS_DC);
int ngx_http_php_code_read_post(char *buffer, uint count_bytes TSRMLS_DC);
char *ngx_http_php_code_read_cookies(TSRMLS_D);
int ngx_http_php_code_header_handler(sapi_header_struct *sapi_header, sapi_header_op_enum op, sapi_headers_struct *sapi_headers TSRMLS_DC);

void (*old_zend_error_cb)(int, const char *, const uint, const char *, va_list);
void ngx_php_error_cb(int type, const char *error_filename, const uint error_lineno, const char *format, va_list args);

#endif