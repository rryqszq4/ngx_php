/**
 *    Copyright(c) 2016-2017 rryqszq4
 *
 *
 */

#include "php_ngx_time.h"
#include "../ngx_http_php_module.h"

static zend_class_entry *php_ngx_time_class_entry;

ZEND_BEGIN_ARG_INFO_EX(ngx_time_sleep_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, time)
ZEND_END_ARG_INFO()

/*static void
_ngx_time_pthread_cleanup(void *arg)
{
    TSRMLS_FETCH();

    ngx_http_request_t *r = PHP_NGX_G(global_r);

    ngx_http_php_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    pthread_mutex_unlock(&(ctx->mutex));

    ctx->enable_thread = 0;
    ngx_http_set_ctx(r, ctx, ngx_http_php_module);

    return ;
}*/

PHP_METHOD(ngx_time, sleep)
{
    long time;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &time) == FAILURE) {
        RETURN_NULL();
    }

    ngx_http_request_t *r = PHP_NGX_G(global_r);

    ngx_http_php_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (ctx == NULL) {

    }

    ctx->delay_time = time * 1000;

    ctx->enable_sleep = 1;

    //ngx_http_set_ctx(r, ctx, ngx_http_php_module);

    //ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "sleep start");

    if (ctx->thread_task) {
        ngx_php_thread_task_notify(ctx->thread_task);
    }

    //pthread_cleanup_push(_ngx_time_pthread_cleanup, NULL);
    pthread_mutex_lock(&(ctx->mutex));
    pthread_cond_wait(&(ctx->cond), &(ctx->mutex));
    pthread_mutex_unlock(&(ctx->mutex));
    //pthread_cleanup_pop(0);

    //ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "sleep");
}

static const zend_function_entry php_ngx_time_class_functions[] = {
    PHP_ME(ngx_time, sleep, ngx_time_sleep_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    {NULL, NULL, NULL, 0, 0}
};

void
php_ngx_time_init(int module_number TSRMLS_DC)
{
    zend_class_entry ngx_time_class_entry;
    INIT_CLASS_ENTRY(ngx_time_class_entry, "ngx_time", php_ngx_time_class_functions);
    php_ngx_time_class_entry = zend_register_internal_class(&ngx_time_class_entry TSRMLS_CC);
}




