/**
 *    Copyright(c) 2016-2017 rryqszq4
 *
 *
 */

#ifndef _NGX_PHP_THREAD_POOL_H_
#define _NGX_PHP_THREAD_POOL_H_

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>

typedef struct ngx_php_thread_task_s ngx_php_thread_task_t;
typedef struct ngx_php_thread_pool_s ngx_php_thread_pool_t;

struct ngx_php_thread_task_s {
    ngx_php_thread_task_t   *next;
    ngx_uint_t              id;
    void                    *ctx;
    void                    (*handler)(void *data, ngx_log_t *log);
    ngx_event_t             event;

    //void                    (*notify_handler)(void *data, ngx_log_t *log);
    ngx_event_t             notify_event;
};

typedef struct {
    ngx_php_thread_task_t   *first;

    ngx_php_thread_task_t   **last;
} ngx_php_thread_pool_queue_t;

struct ngx_php_thread_pool_s {
    ngx_php_thread_mutex_t          mutex;
    ngx_php_thread_pool_queue_t     queue;
    ngx_int_t                       waiting;
    ngx_php_thread_cond_t           cond;

    ngx_log_t                       *log;

    ngx_str_t                       name;
    ngx_uint_t                      threads;
    ngx_int_t                       max_queue;
};

#define ngx_php_thread_pool_queue_init(q)   \
    (q)->first = NULL;                      \
    (q)->last = &(q)->first;

ngx_php_thread_pool_queue_t  ngx_php_thread_pool_done;
ngx_php_thread_pool_queue_t  ngx_php_thread_pool_running;
ngx_uint_t                   ngx_php_thread_pool_task_id;
ngx_atomic_t                 ngx_php_thread_pool_done_lock;

ngx_int_t ngx_php_thread_pool_init(ngx_php_thread_pool_t *tp, ngx_log_t *log, ngx_pool_t *pool);
void ngx_php_thread_pool_destroy(ngx_php_thread_pool_t *tp);

ngx_php_thread_pool_t *ngx_php_thread_pool_add(ngx_conf_t *cf, ngx_str_t *name);
ngx_php_thread_pool_t *ngx_php_thread_pool_get(ngx_cycle_t *cycle, ngx_str_t *name);

ngx_php_thread_task_t *ngx_php_thread_task_alloc(ngx_pool_t *pool, size_t size);
ngx_int_t ngx_php_thread_task_post(ngx_php_thread_pool_t *tp, ngx_php_thread_task_t *task);
void ngx_php_thread_task_notify(ngx_php_thread_task_t *task);

#endif