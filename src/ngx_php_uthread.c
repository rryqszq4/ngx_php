/**
 *    Copyright(c) 2016-2017 rryqszq4
 *
 *
 */

#include "ngx_php_uthread.h"

static void ngx_php_uthread_routine(ngx_php_uthread_t *uthread);

static void 
ngx_php_uthread_routine(ngx_php_uthread_t *uthread)
{
    uthread->routine(uthread->data);
}

ngx_int_t 
ngx_php_uthread_create(ngx_php_uthread_t *uthread)
{
    uthread->stack = malloc(PHP_UTHREAD_STACK_SIZE);

    if (getcontext(&uthread->child) == -1) {
        return -1;
    }

    ngx_php_uthread_max++;
    uthread->id = ngx_php_uthread_max;
    uthread->status = PHP_UTHREAD_READY;

    uthread->child.uc_stack.ss_size = PHP_UTHREAD_STACK_SIZE;
    uthread->child.uc_stack.ss_sp = uthread->stack;
    uthread->child.uc_link = &uthread->main;

    makecontext(&uthread->child, (void (*)(void)) ngx_php_uthread_routine, 1, uthread);

    if (swapcontext(&uthread->main, &uthread->child) == -1) {
        return -1;
    }

    return 0;

}

ngx_int_t 
ngx_php_uthread_yield(ngx_php_uthread_t *uthread)
{
    uthread->status = PHP_UTHREAD_RUNNING;
    
    if (swapcontext(&uthread->child, &uthread->main) == -1) {
        return -1;
    }

    return 0;
}

ngx_int_t 
ngx_php_uthread_resume(ngx_php_uthread_t *uthread)
{
    uthread->status = PHP_UTHREAD_SUSPEND;

    if (swapcontext(&uthread->main, &uthread->child) == -1) {
        return -1;
    }

    return 0;
}

ngx_int_t 
ngx_php_uthread_destroy(ngx_php_uthread_t *uthread)
{
    free(uthread->stack);
    uthread->stack = NULL;
    return 0;
}

ngx_uint_t 
ngx_php_uthread_id(ngx_php_uthread_t *uthread)
{
    if (uthread->id) {
        return uthread->id;
    }

    return 0;
}




