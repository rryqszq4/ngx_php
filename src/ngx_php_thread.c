/**
 *    Copyright(c) 2016-2017 rryqszq4
 *
 *
 */

#include <ngx_config.h>
#include <ngx_core.h>
#include "ngx_php_thread.h"

#if (NGX_LINUX)

ngx_php_tid_t
ngx_php_thread_tid(void)
{
    return syscall(SYS_gettid);
}

#elif (NGX_FREEBSD) && (__FreeBSD_version >= 900031)

#include <pthread_np.h>

ngx_php_tid_t
ngx_php_thread_tid(void)
{
    return pthread_getthreadid_np();
}

#elif (NGX_DARWIN)

ngx_php_tid_t
ngx_php_thread_tid(void)
{
    uint64_t tid;

    (void) pthread_threadid_np(NULL, &tid);
    return tid;
}

#else

ngx_php_tid_t
ngx_php_thread_tid(void)
{
    return (uint64_t) (uintptr_t) pthread_self();
}

#endif

ngx_int_t
ngx_php_thread_mutex_create(ngx_php_thread_mutex_t *mutex, ngx_log_t *log)
{
    ngx_err_t err;
    pthread_mutexattr_t attr;

    err = pthread_mutexattr_init(&attr);
    if (err != 0) {
        ngx_log_error(NGX_LOG_EMERG, log, err, 
            "pthread_mutexattr_init() failed");
        return NGX_ERROR;
    }

    err = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    if (err != 0) {
        ngx_log_error(NGX_LOG_EMERG, log, err, 
            "pthread_mutexattr_settype (PTHREAD_MUTEX_ERRORCHECK) failed");
        return NGX_ERROR;
    }

    err = pthread_mutex_init(mutex, &attr);
    if (err != 0) {
        ngx_log_error(NGX_LOG_EMERG, log, err, 
            "pthread_mutex_init() failed");
        return NGX_ERROR;
    }

    err = pthread_mutexattr_destroy(&attr);
    if (err != 0) {
        ngx_log_error(NGX_LOG_ALERT, log, err, 
            "pthread_mutexattr_destroy() failed");
    }

    ngx_log_debug1(NGX_LOG_DEBUG_CORE, log, 0, 
        "pthread_mutex_init(%p)", mutex);

    return NGX_OK;
}

ngx_int_t
ngx_php_thread_mutex_destroy(ngx_php_thread_mutex_t *mutex, ngx_log_t *log)
{
    ngx_err_t err;

    err = pthread_mutex_destroy(mutex);
    if (err != 0) {
        ngx_log_error(NGX_LOG_ALERT, log, err, 
            "pthread_mutex_destroy() failed");
        return NGX_ERROR;
    }

    ngx_log_debug1(NGX_LOG_DEBUG_CORE, log, 0,
        "pthread_mutex_destroy(%p)", mutex);
    return NGX_OK;
}

ngx_int_t
ngx_php_thread_mutex_lock(ngx_php_thread_mutex_t *mutex, ngx_log_t *log)
{
    ngx_err_t err;

    ngx_log_debug1(NGX_LOG_DEBUG_CORE, log, 0, 
        "pthread_mutex_lock(%p) enter", mutex);

    err = pthread_mutex_lock(mutex);
    if (err == 0) {
        return NGX_OK;
    }

    ngx_log_error(NGX_LOG_ALERT, log, err, "pthread_mutex_lock() failed");

    return NGX_ERROR;
}

ngx_int_t
ngx_php_thread_mutex_unlock(ngx_php_thread_mutex_t *mutex, ngx_log_t *log)
{
    ngx_err_t err;

    err = pthread_mutex_unlock(mutex);

    if (err == 0) {
        ngx_log_debug1(NGX_LOG_DEBUG_CORE, log, 0, 
            "pthread_mutex_unlock(%p) exit", mutex);
        return NGX_OK;
    }

    ngx_log_error(NGX_LOG_ALERT, log, err, 
        "pthread_mutex_unlock() failed");

    return NGX_ERROR;
}

ngx_int_t
ngx_php_thread_cond_create(ngx_php_thread_cond_t *cond, ngx_log_t *log)
{
    ngx_err_t err;

    err = pthread_cond_init(cond, NULL);
    if (err == 0) {
        ngx_log_debug1(NGX_LOG_DEBUG_CORE, log, 0, 
            "pthread_cond_init(%p)", cond);
        return NGX_OK;
    }

    ngx_log_error(NGX_LOG_EMERG, log, err, "pthread_cond_init() failed");
    return NGX_ERROR;
}

ngx_int_t
ngx_php_thread_cond_destroy(ngx_php_thread_cond_t *cond, ngx_log_t *log)
{
    ngx_err_t err;

    err = pthread_cond_destroy(cond);
    if (err == 0) {
        ngx_log_debug1(NGX_LOG_DEBUG_CORE, log, 0,
            "pthread_cond_destroy(%p)", cond);
        return NGX_OK;
    }

    ngx_log_error(NGX_LOG_EMERG, log, err, "pthread_cond_destroy() failed");
    return NGX_ERROR;
}

ngx_int_t
ngx_php_thread_cond_signal(ngx_php_thread_cond_t *cond, ngx_log_t *log)
{
    ngx_err_t err;

    err = pthread_cond_signal(cond);
    if (err == 0) {
        ngx_log_debug1(NGX_LOG_DEBUG_CORE, log, 0,
            "pthread_cond_signal(%p)", cond);
        return NGX_OK;
    }

    ngx_log_error(NGX_LOG_EMERG, log, err, "pthread_cond_signal() failed");
    return NGX_ERROR;
}

ngx_int_t
ngx_php_thread_cond_wait(ngx_php_thread_cond_t *cond, ngx_php_thread_mutex_t *mutex, 
    ngx_log_t *log)
{
    ngx_err_t err;

    ngx_log_debug1(NGX_LOG_DEBUG_CORE, log, 0, 
        "pthread_cond_wait(%p) enter", cond);

    err = pthread_cond_wait(cond, mutex);

    if (err == 0) {
        ngx_log_debug1(NGX_LOG_DEBUG_CORE, log, 0,
            "pthread_cond_wait(%p) exit", cond);
        return NGX_OK;
    }

    ngx_log_error(NGX_LOG_ALERT, log, err, "pthread_cond_wait() failed");

    return NGX_ERROR;
}





