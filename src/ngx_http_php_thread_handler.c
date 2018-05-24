/**
 *    Copyright(c) 2016-2017 rryqszq4
 *
 *
 */

#include "ngx_http_php_core.h"
#include "ngx_http_php_handler.h"
#include "ngx_http_php_module.h"
#include "ngx_http_php_request.h"
#include "ngx_http_php_subrequest.h"
#include "ngx_http_php_socket_tcp.h"
#include "ngx_http_php_sleep.h"

#include "php/php_ngx_core.h"
#include "php/php_ngx_location.h"
#include "php/php_ngx_socket_tcp.h"
#include "php/php_ngx_log.h"
#include "php/php_ngx_time.h"

static void *ngx_http_php_content_inline_thread_routine(void *data, ngx_log_t *log);
static void *ngx_http_php_content_file_thread_routine(void *data, ngx_log_t *log);

static void *ngx_http_php_content_thread_event_handler(ngx_event_t *ev);
//static void *ngx_http_php_content_thread_notify_handler(void *data, ngx_log_t *log);
static void *ngx_http_php_content_thread_notify_event_handler(ngx_event_t *ev);


ngx_int_t 
ngx_http_php_content_inline_thread_handler(ngx_http_request_t *r)
{
    ngx_php_thread_pool_t       *tp, **tpp;
    ngx_php_thread_task_t       *task;
    ngx_http_php_main_conf_t    *pmcf;
    ngx_http_php_ctx_t          *ctx;

    pmcf = ngx_http_get_module_main_conf(r, ngx_http_php_module);

    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (ctx == NULL){
        ctx = ngx_pcalloc(r->pool, sizeof(*ctx));
        if (ctx == NULL){
            return NGX_ERROR;
        }
    }
    ngx_http_set_ctx(r, ctx, ngx_http_php_module);

    ctx->enable_async = 0;
    ctx->enable_upstream = 0;
    ctx->enable_upstream_continue = 0;
    ctx->enable_sleep = 0;
    ctx->enable_thread = 1;

    ctx->read_or_write = 0;

    ctx->is_capture_multi = 0;
    ctx->capture_multi_complete_total = 0;
    ctx->is_capture_multi_complete = 0;

    ctx->error = NGX_OK;

    ctx->request_body_more = 1;

    ctx->receive_stat = 0;
    ctx->receive_total = 0;

    ctx->thread_wait = 0;

    pthread_mutex_init(&(ctx->mutex), NULL);
    pthread_cond_init(&(ctx->cond), NULL);

    ngx_php_request = r;

    if (r->method == NGX_HTTP_POST){
        return ngx_http_php_content_post_handler(r);
    }

    //ngx_http_php_request_cleanup_handler(r);

    tpp = pmcf->thread_pools.elts;
    tp = tpp[0];

    ctx->thread_pool = tp;

    task = ngx_php_thread_task_alloc(r->pool, 0);

    ctx->thread_task = task;

    task->ctx = r;
    task->handler = (void *)ngx_http_php_content_inline_thread_routine;

    task->event.data = r;
    task->event.handler = (void *)ngx_http_php_content_thread_event_handler;

    //task->notify_handler = (void *)ngx_http_php_content_thread_notify_handler;
    task->notify_event.data = r;
    task->notify_event.handler = (void *)ngx_http_php_content_thread_notify_event_handler;


    if (ngx_php_thread_task_post(tp, task) != NGX_OK) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    //ngx_php_thread_task_notify(task);
    //(void) ngx_notify(ngx_http_php_thread_notify_handler);

    /*if (ctx->enable_async == 1){
        if (ctx->is_capture_multi == 0){
            ngx_http_php_subrequest_post(r);
        } else {
            ngx_http_php_subrequest_post_multi(r);
        }

        return NGX_DONE;
    }

    if (ctx->enable_upstream == 1){
        ngx_http_php_socket_tcp_run(r);
        return NGX_DONE;
    }

    if (ctx->enable_sleep == 1) {
        ngx_http_php_sleep_run(r);
        return NGX_DONE;
    }*/

    r->main->count++;

    return NGX_DONE;
}

ngx_int_t 
ngx_http_php_content_file_thread_handler(ngx_http_request_t *r)
{
    ngx_php_thread_pool_t       *tp, **tpp;
    ngx_php_thread_task_t       *task;
    ngx_http_php_main_conf_t    *pmcf;
    ngx_http_php_ctx_t          *ctx;

    pmcf = ngx_http_get_module_main_conf(r, ngx_http_php_module);

    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (ctx == NULL){
        ctx = ngx_pcalloc(r->pool, sizeof(*ctx));
        if (ctx == NULL){
            return NGX_ERROR;
        }
    }
    ngx_http_set_ctx(r, ctx, ngx_http_php_module);

    ctx->enable_async = 0;
    ctx->enable_upstream = 0;
    ctx->enable_upstream_continue = 0;
    ctx->enable_sleep = 0;
    ctx->enable_thread = 1;

    ctx->read_or_write = 0;

    ctx->is_capture_multi = 0;
    ctx->capture_multi_complete_total = 0;
    ctx->is_capture_multi_complete = 0;

    ctx->error = NGX_OK;

    ctx->request_body_more = 1;

    ctx->receive_stat = 0;
    ctx->receive_total = 0;

    ctx->thread_wait = 0;

    pthread_mutex_init(&(ctx->mutex), NULL);
    pthread_cond_init(&(ctx->cond), NULL);

    ngx_php_request = r;

    if (r->method == NGX_HTTP_POST){
        return ngx_http_php_content_post_handler(r);
    }

    //ngx_http_php_request_cleanup_handler(r);

    tpp = pmcf->thread_pools.elts;
    tp = tpp[0];

    ctx->thread_pool = tp;

    task = ngx_php_thread_task_alloc(r->pool, 0);

    ctx->thread_task = task;

    task->ctx = r;
    task->handler = (void *)ngx_http_php_content_file_thread_routine;

    task->event.data = r;
    task->event.handler = (void *)ngx_http_php_content_thread_event_handler;

    //task->notify_handler = (void *)ngx_http_php_content_thread_notify_handler;
    task->notify_event.data = r;
    task->notify_event.handler = (void *)ngx_http_php_content_thread_notify_event_handler;


    if (ngx_php_thread_task_post(tp, task) != NGX_OK) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    r->main->count++;

    return NGX_DONE;
}

static void *ngx_http_php_content_inline_thread_routine(void *data, ngx_log_t *log)
{
    TSRMLS_FETCH();
    ngx_http_request_t *r = (ngx_http_request_t *)data;

    //ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "pthread r %p %d", r , r->keepalive);
    ngx_http_php_main_conf_t *pmcf = ngx_http_get_module_main_conf(r, ngx_http_php_module);
    ngx_http_php_loc_conf_t *plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);

    //ngx_php_ngx_run(r, pmcf->state, plcf->content_async_inline_code);

    NGX_HTTP_PHP_NGX_INIT;

        php_ngx_core_init(0 TSRMLS_CC);
        //ngx_location_init(0 TSRMLS_CC);
        php_ngx_log_init(0 TSRMLS_CC);
        ngx_socket_tcp_init(0 TSRMLS_CC);
        php_ngx_time_init(0 TSRMLS_CC);

        PHP_NGX_G(global_r) = r;

        ngx_php_ngx_run(r, pmcf->state, plcf->content_inline_code);

    NGX_HTTP_PHP_NGX_SHUTDOWN;

    ngx_http_php_ctx_t *ctx;
    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);
    ctx->enable_async = 0;
    ctx->enable_thread = 0;
    ngx_http_set_ctx(r, ctx, ngx_http_php_module);

    return NULL;
}

static void *ngx_http_php_content_file_thread_routine(void *data, ngx_log_t *log)
{
    TSRMLS_FETCH();
    ngx_http_request_t *r = (ngx_http_request_t *)data;

    //ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "pthread r %p %d", r , r->keepalive);
    ngx_http_php_main_conf_t *pmcf = ngx_http_get_module_main_conf(r, ngx_http_php_module);
    ngx_http_php_loc_conf_t *plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);

    //ngx_php_ngx_run(r, pmcf->state, plcf->content_async_inline_code);

    NGX_HTTP_PHP_NGX_INIT;

        php_ngx_core_init(0 TSRMLS_CC);
        //ngx_location_init(0 TSRMLS_CC);
        php_ngx_log_init(0 TSRMLS_CC);
        ngx_socket_tcp_init(0 TSRMLS_CC);
        php_ngx_time_init(0 TSRMLS_CC);

        PHP_NGX_G(global_r) = r;

        ngx_php_ngx_run(r, pmcf->state, plcf->content_code);

    NGX_HTTP_PHP_NGX_SHUTDOWN;

    ngx_http_php_ctx_t *ctx;
    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);
    ctx->enable_async = 0;
    ctx->enable_thread = 0;
    ngx_http_set_ctx(r, ctx, ngx_http_php_module);

    return NULL;
}

static void *ngx_http_php_content_thread_event_handler(ngx_event_t *ev)
{
    ngx_int_t rc;
    ngx_http_request_t *r;
    ngx_http_php_ctx_t *ctx;
    ngx_http_php_rputs_chain_list_t *chain;
    
    r = ev->data;
    //r->keepalive = 0;
    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "content_thread_event_handler");

    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    //ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "task #%ui output event handler test thread_wait:%d, %d %d", 
    //        ctx->thread_task->id, ctx->thread_wait,ctx->enable_upstream_continue, ctx->enable_upstream );

    pthread_cond_destroy(&(ctx->cond));
    pthread_mutex_destroy(&(ctx->mutex));

    chain = ctx->rputs_chain;

    if (ctx->rputs_chain == NULL){
        ngx_buf_t *b;
        ngx_str_t ns;
        u_char *u_str;
        ns.data = (u_char *)" ";
        ns.len = 1;
        
        chain = ngx_pcalloc(r->pool, sizeof(ngx_http_php_rputs_chain_list_t));
        chain->out = ngx_alloc_chain_link(r->pool);
        chain->last = &chain->out;
    
        b = ngx_calloc_buf(r->pool);
        (*chain->last)->buf = b;
        (*chain->last)->next = NULL;

        u_str = ngx_pstrdup(r->pool, &ns);
        //u_str[ns.len] = '\0';
        (*chain->last)->buf->pos = u_str;
        (*chain->last)->buf->last = u_str + ns.len;
        (*chain->last)->buf->memory = 1;
        ctx->rputs_chain = chain;

        if (r->headers_out.content_length_n == -1){
            r->headers_out.content_length_n += ns.len + 1;
        }else {
            r->headers_out.content_length_n += ns.len;
        }
    }

    //r->headers_out.content_type.len = sizeof("text/html") - 1;
    //r->headers_out.content_type.data = (u_char *)"text/html";
    if (!r->headers_out.status){
        r->headers_out.status = NGX_HTTP_OK;
    }

    if (r->method == NGX_HTTP_HEAD){
        rc = ngx_http_send_header(r);
        if (rc != NGX_OK){
            ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
            return NULL;
        }
    }

    if (chain != NULL){
        (*chain->last)->buf->last_buf = 1;
    }

    rc = ngx_http_send_header(r);
    if (rc != NGX_OK){
        ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
        return NULL;
    }

    ngx_http_output_filter(r, chain->out);

    ngx_http_set_ctx(r, NULL, ngx_http_php_module);

    ngx_http_finalize_request(r, rc);
    return NULL;
}

/*static void *
ngx_http_php_content_thread_notify_handler(void *data, ngx_log_t *log)
{
    ngx_http_request_t *r;
    ngx_http_php_ctx_t *ctx;

    r = data;
    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    //sleep(3);
    ngx_log_error(NGX_LOG_ERR, log, 0, "notify handler test");

    if (ctx->enable_sleep) {
        ngx_http_php_sleep_thread_run(r);
    }

    //pthread_mutex_lock(&(ctx->mutex));
    //pthread_cond_signal(&(ctx->cond));
    //pthread_mutex_unlock(&(ctx->mutex));

    return NULL;
}*/

static void *
ngx_http_php_content_thread_notify_event_handler(ngx_event_t *ev)
{
    ngx_http_request_t *r;
    ngx_http_php_ctx_t *ctx;
    
    r = ev->data;
    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    //ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,"ctx: %p %p", r,ctx);

    if (r == NULL || ctx == NULL) {
        return NULL;
    }

    /*for (;;) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "task #%ui notify event handler test %d %d %d", 
            ctx->thread_task->id, ctx->thread_wait,ctx->enable_upstream_continue, ctx->enable_upstream );

        if (ctx->thread_wait == 1) {
            break;
        }
    }*/

    if (ctx->enable_sleep == 1) {
        ngx_http_php_sleep_thread_run(r);
    }

    if (ctx->enable_upstream_continue == 0 && ctx->enable_upstream == 1){
        ngx_http_php_socket_tcp_thread_run(r);
    }

    if (ctx->enable_upstream_continue == 1) {
        ngx_http_php_socket_tcp_thread_rediscovery(r);
    }

    //pthread_mutex_lock(&(ctx->mutex));
    //pthread_cond_signal(&(ctx->cond));
    //pthread_mutex_unlock(&(ctx->mutex));

    return NULL;
}


