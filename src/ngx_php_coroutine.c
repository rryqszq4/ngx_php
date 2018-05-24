/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */

#include "ngx_php_coroutine.h"

static void ngx_php_coroutine_routine(ngx_php_coroutine_t *coroutine);

static void 
ngx_php_coroutine_routine(ngx_php_coroutine_t *coroutine)
{
	coroutine->routine(coroutine->data);
}

ngx_int_t 
ngx_php_coroutine_create(ngx_php_coroutine_t *coroutine)
{
    if (!coroutine->stack) {
        return -1;
    }

	if (getcontext(&coroutine->child) == -1) {
		return -1;
	}

	ngx_php_coroutine_max++;
	coroutine->id = ngx_php_coroutine_max;
	coroutine->status = PHP_COROUTINE_READY;

	coroutine->child.uc_stack.ss_size = PHP_COROUTINE_STACK_SIZE;
    coroutine->child.uc_stack.ss_sp = coroutine->stack;
    coroutine->child.uc_link = &coroutine->main;

    makecontext(&coroutine->child, (void (*)(void)) ngx_php_coroutine_routine, 1, coroutine);

    if (swapcontext(&coroutine->main, &coroutine->child) == -1) {
        return -1;
    }

    return 0;
}

ngx_int_t 
ngx_php_coroutine_yield(ngx_php_coroutine_t *coroutine)
{
    coroutine->status = PHP_COROUTINE_RUNNING;
    
    if (swapcontext(&coroutine->child, &coroutine->main) == -1) {
        return -1;
    }

    return 0;
}

ngx_int_t 
ngx_php_coroutine_resume(ngx_php_coroutine_t *coroutine)
{
    coroutine->status = PHP_COROUTINE_SUSPEND;

    if (swapcontext(&coroutine->main, &coroutine->child) == -1) {
        return -1;
    }

    return 0;
}

ngx_int_t 
ngx_php_coroutine_destroy(ngx_php_coroutine_t *coroutine)
{
    return 0;
}

ngx_uint_t 
ngx_php_coroutine_id(ngx_php_coroutine_t *coroutine)
{
    if (coroutine->id) {
        return coroutine->id;
    }

    return 0;
}


