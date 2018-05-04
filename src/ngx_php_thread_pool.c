/**
 *    Copyright(c) 2016-2017 rryqszq4
 *
 *
 */

#include <ngx_config.h>
#include <ngx_core.h>

#include "ngx_php_thread.h"
#include "ngx_php_thread_pool.h"

static void ngx_php_thread_pool_exit_handler(void *data, ngx_log_t *log);
static void *ngx_php_thread_pool_cycle(void *data);
static void ngx_php_thread_pool_handler(ngx_event_t *ev);
static void ngx_php_thread_task_notify_handler(ngx_event_t *ev);

//static ngx_uint_t                   ngx_php_thread_pool_task_id;
//static ngx_atomic_t                 ngx_php_thread_pool_done_lock;

ngx_int_t
ngx_php_thread_pool_init(ngx_php_thread_pool_t *tp, ngx_log_t *log, ngx_pool_t *pool)
{
    int             err;
    pthread_t       tid;
    ngx_uint_t      n;
    pthread_attr_t  attr;

    if (ngx_notify == NULL) {
        ngx_log_error(NGX_LOG_ALERT, log, 0,
               "the configured event method cannot be used with thread pools");
        return NGX_ERROR;
    }

    ngx_php_thread_pool_queue_init(&tp->queue);

    if (ngx_php_thread_mutex_create(&tp->mutex, log) != NGX_OK) {
        return NGX_ERROR;
    }

    if (ngx_php_thread_cond_create(&tp->cond, log) != NGX_OK) {
        (void) ngx_php_thread_mutex_destroy(&tp->mutex, log);
        return NGX_ERROR;
    }

    tp->log = log;

    err = pthread_attr_init(&attr);
    if (err) {
        ngx_log_error(NGX_LOG_ALERT, log, err,
                      "pthread_attr_init() failed");
        return NGX_ERROR;
    }

#if 0
    err = pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);
    if (err) {
        ngx_log_error(NGX_LOG_ALERT, log, err,
                      "pthread_attr_setstacksize() failed");
        return NGX_ERROR;
    }
#endif

    for (n = 0; n < tp->threads; n++) {
        err = pthread_create(&tid, &attr, ngx_php_thread_pool_cycle, tp);
        if (err) {
            ngx_log_error(NGX_LOG_ALERT, log, err,
                          "pthread_create() failed");
            return NGX_ERROR;
        }
    }

    (void) pthread_attr_destroy(&attr);

    return NGX_OK;
}

void
ngx_php_thread_pool_destroy(ngx_php_thread_pool_t *tp)
{
    ngx_uint_t              n;
    ngx_php_thread_task_t   task;
    volatile ngx_uint_t     lock;

    ngx_memzero(&task, sizeof(ngx_php_thread_task_t));

    task.handler = ngx_php_thread_pool_exit_handler;
    task.ctx = (void *) &lock;

    for (n = 0; n < tp->threads; n++) {
        lock = 1;

        if (ngx_php_thread_task_post(tp, &task) != NGX_OK) {
            return ;
        }

        while (lock) {
            ngx_sched_yield();
        }

        task.event.active = 0;
    }

    (void) ngx_php_thread_cond_destroy(&tp->cond, tp->log);

    (void) ngx_php_thread_mutex_destroy(&tp->mutex, tp->log);
}

static void
ngx_php_thread_pool_exit_handler(void *data, ngx_log_t *log)
{
    ngx_uint_t *lock = data;

    *lock = 0;

    pthread_exit(0);
}

ngx_php_thread_task_t *
ngx_php_thread_task_alloc(ngx_pool_t *pool, size_t size)
{
    ngx_php_thread_task_t *task;

    task = ngx_pcalloc(pool, sizeof(ngx_php_thread_task_t) + size);
    if (task == NULL) {
        return NULL;
    }

    task->ctx = task + 1;

    return task;
}

ngx_int_t
ngx_php_thread_task_post(ngx_php_thread_pool_t *tp, ngx_php_thread_task_t *task)
{
    if (task->event.active) {
        ngx_log_error(NGX_LOG_ALERT, tp->log, 0,
                      "task #%ui already active", task->id);
        return NGX_ERROR;
    }

    if (ngx_php_thread_mutex_lock(&tp->mutex, tp->log) != NGX_OK) {
        return NGX_ERROR;
    }

    if (tp->waiting >= tp->max_queue) {
        (void) ngx_php_thread_mutex_unlock(&tp->mutex, tp->log);

        ngx_log_error(NGX_LOG_ERR, tp->log, 0,
                      "thread pool \"%V\" queue overflow: %i tasks waiting",
                      &tp->name, tp->waiting);
        return NGX_ERROR;
    }

    task->event.active = 1;

    task->id = ngx_php_thread_pool_task_id++;
    task->next = NULL;

    if (ngx_php_thread_cond_signal(&tp->cond, tp->log) != NGX_OK) {
        (void) ngx_php_thread_mutex_unlock(&tp->mutex, tp->log);
        return NGX_ERROR;
    }

    *tp->queue.last = task;
    tp->queue.last = &task->next;

    tp->waiting++;

    (void) ngx_php_thread_mutex_unlock(&tp->mutex, tp->log);

    ngx_log_debug2(NGX_LOG_DEBUG_CORE, tp->log, 0,
                   "task #%ui added to thread pool name: \"%V\" complete",
                   task->id, &tp->name);

    return NGX_OK;
}

static void *
ngx_php_thread_pool_cycle(void *data)
{
    ngx_php_thread_pool_t *tp = data;

    int                     err;
    sigset_t                set;
    ngx_php_thread_task_t   *task;

#if 0
    ngx_time_update();
#endif

    ngx_log_debug1(NGX_LOG_DEBUG_CORE, tp->log, 0,
                   "thread in pool \"%V\" started", &tp->name);

    sigfillset(&set);

    sigdelset(&set, SIGILL);
    sigdelset(&set, SIGFPE);
    sigdelset(&set, SIGSEGV);
    sigdelset(&set, SIGBUS);

    err = pthread_sigmask(SIG_BLOCK, &set, NULL);
    if (err) {
        ngx_log_error(NGX_LOG_ALERT, tp->log, err, "pthread_sigmask() failed");
        return NULL;
    }

    for ( ;; ) {
        if (ngx_php_thread_mutex_lock(&tp->mutex, tp->log) != NGX_OK) {
            return NULL;
        }

        tp->waiting--;

        while (tp->queue.first == NULL) {
            if (ngx_php_thread_cond_wait(&tp->cond, &tp->mutex, tp->log) != NGX_OK) {
                (void) ngx_php_thread_mutex_unlock(&tp->mutex, tp->log);
                return NULL;
            }
        }

        task = tp->queue.first;
        tp->queue.first = task->next;

        if (tp->queue.first == NULL) {
            tp->queue.last = &tp->queue.first;
        }

        if (ngx_php_thread_mutex_unlock(&tp->mutex, tp->log) != NGX_OK) {
            return NULL;
        }

#if 0
        ngx_time_update();
#endif

        ngx_log_debug2(NGX_LOG_DEBUG_CORE, tp->log, 0,
                       "run task #%ui in thread pool name:\"%V\"",
                       task->id, &tp->name);

        task->handler(task->ctx, tp->log);

        ngx_log_debug2(NGX_LOG_DEBUG_CORE, tp->log, 0,
                       "complete task #%ui in thread pool name: \"%V\"",
                       task->id, &tp->name);

        task->next = NULL;

        ngx_spinlock(&ngx_php_thread_pool_done_lock, 1, 2048);

        *ngx_php_thread_pool_done.last = task;
        ngx_php_thread_pool_done.last = &task->next;

        ngx_unlock(&ngx_php_thread_pool_done_lock);

        (void) ngx_notify(ngx_php_thread_pool_handler);

    }
}

static void
ngx_php_thread_pool_handler(ngx_event_t *ev)
{
    ngx_event_t             *event;
    ngx_php_thread_task_t   *task;

    ngx_log_debug0(NGX_LOG_DEBUG_CORE, ev->log, 0, "thread pool handler");

    ngx_spinlock(&ngx_php_thread_pool_done_lock, 1, 2048);

    task = ngx_php_thread_pool_done.first;
    ngx_php_thread_pool_done.first = NULL;
    ngx_php_thread_pool_done.last = &ngx_php_thread_pool_done.first;

    ngx_unlock(&ngx_php_thread_pool_done_lock);

    while (task) {
        ngx_log_debug1(NGX_LOG_DEBUG_CORE, ev->log, 0,
                       "run completion handler for task #%ui", task->id);

        event = &task->event;
        task = task->next;

        event->complete = 1;
        event->active = 0;

        event->handler(event);
    }
}

void 
ngx_php_thread_task_notify(ngx_php_thread_task_t *task)
{
    task->next = NULL;

    ngx_spinlock(&ngx_php_thread_pool_done_lock, 1, 2048);

    *ngx_php_thread_pool_running.last = task;
    ngx_php_thread_pool_running.last = &task->next;

    ngx_unlock(&ngx_php_thread_pool_done_lock);

    (void) ngx_notify(ngx_php_thread_task_notify_handler);
}

static void 
ngx_php_thread_task_notify_handler(ngx_event_t *ev)
{
    ngx_event_t             *event;
    ngx_php_thread_task_t   *task;

    ngx_spinlock(&ngx_php_thread_pool_done_lock, 1, 2048);

    task = ngx_php_thread_pool_running.first;
    ngx_php_thread_pool_running.first = NULL;
    ngx_php_thread_pool_running.last = &ngx_php_thread_pool_running.first;

    ngx_unlock(&ngx_php_thread_pool_done_lock);

    while (task) {
        
        /*if (task->notify_handler) {
            task->notify_handler(task->ctx, ev->log);
        }*/

        event = &task->notify_event;
        task = task->next;

        //event->complete = 1;
        //event->active = 0;


        //ngx_log_error(NGX_LOG_ERR, ev->log, 0, "notify handler event %p ", event);
        if (event) {
            event->handler(event);
        }

    }
}













