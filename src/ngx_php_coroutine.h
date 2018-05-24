/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */

#ifndef _NGX_PHP_COROUTINE_H_
#define _NGX_PHP_COROUTINE_H_

#include <ngx_config.h>
#include <ngx_core.h>

#if __APPLE__ && __MACH__
	#include <sys/ucontext.h>
#else 
	#include <ucontext.h>
#endif

#define PHP_COROUTINE_DEAD    0
#define PHP_COROUTINE_READY   1
#define PHP_COROUTINE_RUNNING 2
#define PHP_COROUTINE_SUSPEND 3

#define PHP_COROUTINE_STACK_SIZE      32*1024
#define PHP_COROUTINE_MAX_SIZE        1024

ngx_uint_t ngx_php_coroutine_max;

typedef struct ngx_php_coroutine_s {

    ngx_uint_t      id;
    ngx_int_t       status;

    ucontext_t      child;
    ucontext_t      main;

    void            *stack;
    size_t          stack_size;

    void            (*routine)(void *data);
    void            *data;

    ngx_log_t       *log;

} ngx_php_coroutine_t;

ngx_int_t ngx_php_coroutine_create(ngx_php_coroutine_t *coroutine);

ngx_int_t ngx_php_coroutine_yield(ngx_php_coroutine_t *coroutine);

ngx_int_t ngx_php_coroutine_resume(ngx_php_coroutine_t *coroutine);

ngx_int_t ngx_php_coroutine_destroy(ngx_php_coroutine_t *coroutine);

ngx_uint_t ngx_php_coroutine_id(ngx_php_coroutine_t *coroutine);

#endif