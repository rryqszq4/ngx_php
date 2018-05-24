/**
 *    Copyright(c) 2016-2017 rryqszq4
 *
 *
 */

#ifndef _NGX_PHP_THREAD_H_
#define _NGX_PHP_THREAD_H_

#include <ngx_config.h>
#include <ngx_core.h>

#include <pthread.h>

typedef pthread_mutex_t ngx_php_thread_mutex_t;

ngx_int_t ngx_php_thread_mutex_create(ngx_php_thread_mutex_t *mutex, ngx_log_t *log);
ngx_int_t ngx_php_thread_mutex_destroy(ngx_php_thread_mutex_t *mutex, ngx_log_t *log);
ngx_int_t ngx_php_thread_mutex_lock(ngx_php_thread_mutex_t *mutex, ngx_log_t *log);
ngx_int_t ngx_php_thread_mutex_unlock(ngx_php_thread_mutex_t *mutex, ngx_log_t *log);

typedef pthread_cond_t ngx_php_thread_cond_t;

ngx_int_t ngx_php_thread_cond_create(ngx_php_thread_cond_t *cond, ngx_log_t *log);
ngx_int_t ngx_php_thread_cond_destroy(ngx_php_thread_cond_t *cond, ngx_log_t *log);
ngx_int_t ngx_php_thread_cond_signal(ngx_php_thread_cond_t *cond, ngx_log_t *log);
ngx_int_t ngx_php_thread_cond_wait(ngx_php_thread_cond_t *cond, ngx_php_thread_mutex_t *mutex, ngx_log_t *log);

#if (NGX_LINUX)

typedef pid_t ngx_php_tid_t;
#define NGX_PHP_TID_T_FMT   "%P"

#elif (NGX_FREEBSD)

typedef uint32_t ngx_php_tid_t;
#define NGX_PHP_TID_T_FMT   "%uD"

#elif (NGX_DARWIN)

typedef uint64_t ngx_php_tid_t;
#define NGX_PHP_TID_T_FMT   "%uA"

#else

typedef uint64_t ngx_php_tid_t;
#define NGX_PHP_TID_T_FMT   "%uA";

#endif

ngx_php_tid_t ngx_php_thread_tid(void);

//#define ngx_log_tid ngx_php_thread_tid()


#endif