/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */


#include "ngx_php_debug.h"
#include "ngx_http_php_sleep.h"

static void ngx_http_php_sleep_cleanup(void *data);

static void ngx_http_php_sleep_handler(ngx_event_t *ev);

static void
ngx_http_php_sleep_cleanup(void *data) 
{
    ngx_http_request_t *r = data;
    ngx_http_php_ctx_t *ctx;

    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (ctx == NULL) {
        return ;
    }

    if (ctx->sleep.timer_set) {
        ngx_del_timer(&ctx->sleep);
        return ;
    }
    
}

ngx_int_t
ngx_http_php_sleep(ngx_http_request_t *r) 
{
    ngx_http_cleanup_t *cln;
    ngx_http_php_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (ctx == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    
    ctx->phase_status = NGX_AGAIN;

    //ngx_memzero(&ctx->sleep, sizeof(ngx_event_t));

    ctx->sleep.handler = ngx_http_php_sleep_handler;
    ctx->sleep.log = r->connection->log;
    ctx->sleep.data = r;

    //ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,"%p %p %d", r, &ctx->sleep, ctx->delay_time);

    ngx_php_debug("r:%p, &ctx->sleep:%p, ctx->delay_time:%d", r, &ctx->sleep, (int)ctx->delay_time);

    ngx_add_timer(&ctx->sleep, (ngx_msec_t) ctx->delay_time);

    cln = ngx_http_cleanup_add(r, 0);
    if (cln == NULL) {
        return NGX_ERROR;
    }

    cln->handler = ngx_http_php_sleep_cleanup;
    cln->data = r;

    r->keepalive = 0;

    return NGX_OK;
}

static void 
ngx_http_php_sleep_handler(ngx_event_t *ev)
{
    TSRMLS_FETCH();
    
    ngx_http_request_t *r;

    r = ev->data;

    zend_first_try {
        PHP_NGX_G(global_r) = r;
        
        zend_eval_string_ex("ngx_php::next();", NULL, "ngx_php eval code", 1 TSRMLS_CC);

    }zend_end_try();

    ngx_http_core_run_phases(r);
}

/*
ngx_int_t ngx_http_php_sleep_run(ngx_http_request_t *r)
{
    //ngx_event_t ev;
    //ngx_event_t *wev;
    //ngx_connection_t *c;
    ngx_http_cleanup_t *cln;

    ngx_php_request = r;

    ngx_http_php_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (ctx == NULL){
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    r->main->count++;
    //c = r->connection;
    //c->fd = (ngx_socket_t) -1;
    //c->data = r;

    ngx_memzero(&ctx->sleep, sizeof(ngx_event_t));

    //ev.timer_set = 0;
    ctx->sleep.handler = ngx_http_php_sleep_handler;
    ctx->sleep.log = r->connection->log;
    ctx->sleep.data = r;

    //wev = c->write;
    //wev->log = c->log;
    //wev->handler = ngx_http_php_sleep_handler;
    //wev->data = c;

    //ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "ngx_add_timer");

    ngx_add_timer(&ctx->sleep, (ngx_msec_t) ctx->delay_time);

    cln = ngx_http_cleanup_add(r, 0);
    if (cln == NULL) {
        return NGX_ERROR;
    }

    cln->handler = ngx_http_php_sleep_cleanup;
    cln->data = r;

    return NGX_OK;
}

void ngx_http_php_sleep_handler(ngx_event_t *ev)
{
    ngx_http_request_t *r;
    //ngx_connection_t *c;

    //c = ev->data;
    //r = c->data;

    r = ev->data;

    ngx_php_request = r;

    //ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "sleep handler");

    ngx_http_php_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    // close keep-alive
    r->keepalive = 0;

    //r->main->count--;

    ctx->enable_sleep = 0;
    //ngx_http_set_ctx(r, ctx, ngx_http_php_module);

    pthread_mutex_lock(&(ctx->mutex));
    pthread_cond_signal(&(ctx->cond));
    pthread_mutex_unlock(&(ctx->mutex));


    for ( ;; ){
        usleep(1);
        //ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);
        //ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "main %d %d", ctx->enable_async, ctx->enable_thread);

        if (ctx->enable_async == 1 || 
            ctx->enable_upstream == 1 || 
            ctx->enable_sleep == 1 ||
            ctx->enable_thread == 0){
            break;
        }
    }

    pthread_cond_destroy(&(ctx->cond));
    pthread_mutex_destroy(&(ctx->mutex));

    ngx_int_t rc;

    ngx_http_php_rputs_chain_list_t *chain;

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
            
        }
    }

    if (chain != NULL){
        (*chain->last)->buf->last_buf = 1;
    }

    rc = ngx_http_send_header(r);
    if (rc != NGX_OK){
        
    }

    rc = ngx_http_output_filter(r, chain->out);

    ngx_http_set_ctx(r, NULL, ngx_http_php_module);

    ngx_http_finalize_request(r,rc);
}

ngx_int_t 
ngx_http_php_sleep_thread_run(ngx_http_request_t *r)
{
    ngx_http_cleanup_t *cln;

    //ngx_php_request = r;

    ngx_http_php_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (ctx == NULL){
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    ngx_memzero(&ctx->sleep, sizeof(ngx_event_t));

    //ev.timer_set = 0;
    ctx->sleep.handler = ngx_http_php_sleep_thread_handler;
    ctx->sleep.log = r->connection->log;
    ctx->sleep.data = r;

    //wev = c->write;
    //wev->log = c->log;
    //wev->handler = ngx_http_php_sleep_handler;
    //wev->data = c;

    //ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "ngx_add_timer %p",ctx->sleep);

    ngx_add_timer(&ctx->sleep, (ngx_msec_t) ctx->delay_time);

    cln = ngx_http_cleanup_add(r, 0);
    if (cln == NULL) {
        return NGX_ERROR;
    }

    cln->handler = ngx_http_php_sleep_cleanup;
    cln->data = r;

    r->keepalive = 0;

    return NGX_OK;
}

void 
ngx_http_php_sleep_thread_handler(ngx_event_t *ev)
{
    ngx_http_request_t *r;

    r = ev->data;

    //ngx_php_request = r;

    //ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "sleep handler");

    ngx_http_php_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    // close keep-alive
    //r->keepalive = 0;


    ctx->enable_sleep = 0;
    //ngx_http_set_ctx(r, ctx, ngx_http_php_module);

    pthread_mutex_lock(&(ctx->mutex));
    pthread_cond_signal(&(ctx->cond));
    pthread_mutex_unlock(&(ctx->mutex));
}

ngx_int_t 
ngx_http_php_sleep_generator_run(ngx_http_request_t *r)
{
    ngx_http_cleanup_t *cln;

    ngx_http_php_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (ctx == NULL){
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    ctx->rewrite_phase = 1;
    ctx->phase_status = NGX_AGAIN;

    //TSRMLS_FETCH();
    //TSRMLS_SET_CTX(ctx->uthread_ctx);

    ngx_memzero(&ctx->sleep, sizeof(ngx_event_t));

    //ev.timer_set = 0;
    ctx->sleep.handler = ngx_http_php_sleep_generator_handler;
    ctx->sleep.log = r->connection->log;
    ctx->sleep.data = r;

    ngx_add_timer(&ctx->sleep, (ngx_msec_t) ctx->delay_time);

    cln = ngx_http_cleanup_add(r, 0);
    if (cln == NULL) {
        return NGX_ERROR;
    }

    cln->handler = ngx_http_php_sleep_cleanup;
    cln->data = r;

    r->keepalive = 0;

    return NGX_OK;
}

void 
ngx_http_php_sleep_generator_handler(ngx_event_t *ev)
{
    ngx_http_request_t *r;

    r = ev->data;

    //ngx_http_php_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    TSRMLS_FETCH();
    //TSRMLS_FETCH_FROM_CTX(ctx->uthread_ctx);

    //ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "rewrite_phase: %d r:%p closure:%p", ctx->rewrite_phase,r,ctx->generator_closure);

    zend_first_try {
        PHP_NGX_G(global_r) = r;
        //zend_eval_string_ex("foreach (coroutine_run() as $number) {echo $number.\"\n\";echo \"break\n\";break;}", NULL, "ngx_php eval code", 1 TSRMLS_CC);
        //zend_eval_string_ex("var_dump($run->valid());$run->next();var_dump($run->valid());", NULL, "ngx_php eval code", 1 TSRMLS_CC);
        zend_eval_string_ex("ngx_generator::next();", NULL, "ngx_php eval code", 1 TSRMLS_CC);

        //zval *func_next;
        //zval retval;
        //MAKE_STD_ZVAL(func_next);
        //ZVAL_STRING(func_next, "next", 1);

        //call_user_function(NULL, &(ctx->generator_closure), func_next, &retval, 0, NULL TSRMLS_CC);

        //zval_ptr_dtor(&func_next);

    }zend_end_try();

    //if (ctx->phase_status == NGX_AGAIN) {
        ngx_http_core_run_phases(r);
    //}
}
*/

