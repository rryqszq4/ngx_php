/**
 *    Copyright(c) 2016-2017 rryqszq4
 *
 *
 */

#ifndef _NGX_PHP_UTHREAD_H_
#define _NGX_PHP_UTHREAD_H_

#include <ngx_config.h>
#include <ngx_core.h>

#include <ucontext.h>

#define PHP_UTHREAD_DEAD    0
#define PHP_UTHREAD_READY   1
#define PHP_UTHREAD_RUNNING 2
#define PHP_UTHREAD_SUSPEND 3

#define PHP_UTHREAD_STACK_SIZE      32*1024
#define PHP_UTHREAD_MAX_SIZE        1024

ngx_uint_t ngx_php_uthread_max;

typedef struct ngx_php_uthread_s {

    ngx_uint_t      id;
    ngx_int_t       status;

    ucontext_t      child;
    ucontext_t      main;

    void            *stack;
    size_t          stack_size;

    void            (*routine)(void *data);
    void            *data;

    ngx_log_t       *log;

} ngx_php_uthread_t;

ngx_int_t ngx_php_uthread_create(ngx_php_uthread_t *uthread);

ngx_int_t ngx_php_uthread_yield(ngx_php_uthread_t *uthread);

ngx_int_t ngx_php_uthread_resume(ngx_php_uthread_t *uthread);

ngx_int_t ngx_php_uthread_destroy(ngx_php_uthread_t *uthread);

ngx_uint_t ngx_php_uthread_id(ngx_php_uthread_t *uthread);

#endif
