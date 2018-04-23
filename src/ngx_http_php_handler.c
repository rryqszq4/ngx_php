/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */

#include "ngx_php_debug.h"
#include "ngx_http_php_core.h"
#include "ngx_http_php_handler.h"
#include "ngx_http_php_module.h"
#include "ngx_http_php_request.h"
#include "ngx_http_php_zend_uthread.h"



//static void ngx_http_php_rewrite_inline_uthread_routine(void *data);
static void ngx_http_php_rewrite_file_uthread_routine(void *data);

//static void ngx_http_php_access_inline_uthread_routine(void *data);
static void ngx_http_php_access_file_uthread_routine(void *data);

//static void ngx_http_php_content_inline_uthread_routine(void *data);
static void ngx_http_php_content_file_uthread_routine(void *data);

ngx_int_t
ngx_http_php_post_read_handler(ngx_http_request_t *r)
{
	TSRMLS_FETCH();
	
	ngx_http_php_ctx_t *ctx;
	
	ngx_php_request = r;

	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

	if (ctx == NULL){
		ctx = ngx_pcalloc(r->pool, sizeof(*ctx));
		if (ctx == NULL){
			return NGX_ERROR;
		}

		ctx->rewrite_phase = 0;
		ctx->access_phase = 0;
		ctx->content_phase = 0;

		ctx->phase_status = NGX_DECLINED;
	}
	ngx_http_set_ctx(r, ctx, ngx_http_php_module);

	return NGX_OK;
}

void 
ngx_http_php_request_cleanup_handler(void *data)
{
	return ;
}

static void 
ngx_http_php_rewrite_file_uthread_routine(void *data)
{
	ngx_http_request_t *r;
	ngx_http_php_ctx_t *ctx;
	ngx_http_php_main_conf_t *pmcf;
	ngx_http_php_loc_conf_t *plcf;

	TSRMLS_FETCH();

	r = data;
	pmcf = ngx_http_get_module_main_conf(r, ngx_http_php_module);
	plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);
	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

	ctx->rewrite_phase = 1;
	ctx->phase_status = NGX_OK;

	PHP_NGX_G(global_r) = r;

	//NGX_HTTP_PHP_R_INIT(r);

	//php_ngx_core_init(0 TSRMLS_CC);
	ngx_php_set_request_status(NGX_DECLINED TSRMLS_CC);
	zend_first_try {

		ngx_php_eval_file(r, pmcf->state, plcf->rewrite_code TSRMLS_CC);

	}zend_end_try();
	//NGX_HTTP_PHP_R_SHUTDOWN(r);

	//ctx->rewrite_phase = 1;

}

/*
static void
ngx_http_php_rewrite_inline_uthread_routine(void *data)
{
	ngx_http_request_t *r;
	ngx_http_php_ctx_t *ctx;
	ngx_http_php_main_conf_t *pmcf;
	ngx_http_php_loc_conf_t *plcf;

	TSRMLS_FETCH();

	r = data;
	pmcf = ngx_http_get_module_main_conf(r, ngx_http_php_module);
	plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);
	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

	ctx->rewrite_phase = 1;
	ctx->phase_status = NGX_OK;

	//NGX_HTTP_PHP_R_INIT(r);
	
	PHP_NGX_G(global_r) = r;

	//ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "r:%p ", r);

	//php_ngx_core_init(0 TSRMLS_CC);
	//php_co_ngx_init(0 TSRMLS_CC);
	ngx_php_set_request_status(NGX_DECLINED TSRMLS_CC);
	zend_first_try {

		ngx_php_eval_code(r, pmcf->state, plcf->rewrite_inline_code TSRMLS_CC);

	}zend_end_try();
	//NGX_HTTP_PHP_R_SHUTDOWN(r);

}
*/

ngx_int_t 
ngx_http_php_rewrite_handler(ngx_http_request_t *r)
{
	ngx_http_php_loc_conf_t *plcf;
	plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);
	if (plcf->rewrite_handler == NULL){
		return NGX_DECLINED;
	}
	return plcf->rewrite_handler(r);
}

ngx_int_t 
ngx_http_php_rewrite_file_handler(ngx_http_request_t *r)
{
	ngx_int_t 							rc;
	ngx_http_php_rputs_chain_list_t 	*chain;
	ngx_http_php_ctx_t 					*ctx;

	TSRMLS_FETCH();

	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

	if (ctx == NULL){
		ctx = ngx_pcalloc(r->pool, sizeof(*ctx));
		if (ctx == NULL){
			return NGX_ERROR;
		}
	}
	ngx_http_set_ctx(r, ctx, ngx_http_php_module);

	ngx_php_request = r;

	//if (!ctx->rewrite_phase) {
	if (ctx->phase_status == NGX_DECLINED){

		/*if (ctx->rewrite_uthread == NULL) {
			ctx->rewrite_uthread = ngx_pcalloc(r->pool, sizeof(ngx_php_uthread_t));
			ctx->rewrite_uthread->stack = ngx_pcalloc(r->pool, PHP_UTHREAD_STACK_SIZE);
		}
		ctx->rewrite_uthread->routine = ngx_http_php_rewrite_file_uthread_routine;
		ctx->rewrite_uthread->data = r;

		ctx->uthread = ctx->rewrite_uthread;

		ngx_php_uthread_create(ctx->uthread);*/

		ngx_http_php_rewrite_file_uthread_routine(r);

		if (ctx->phase_status == NGX_AGAIN){
			
			r->main->count++;

			return NGX_AGAIN;
		} else {

			goto set_output;

		}

		/*if (ctx->rewrite_phase) {

			goto set_output;
			
		} else {
			
			r->main->count++;

			return NGX_AGAIN;
		}*/

	}else {

		if (ctx->phase_status == NGX_AGAIN) {
			r->main->count++;

			return NGX_AGAIN;
		}else {
			goto set_output;
		}

		/*ctx->rewrite_phase = 0;
		goto set_output;*/
	}

set_output:

	rc = ngx_php_get_request_status(TSRMLS_C);

	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

	if (ctx->generator_closure ) {
		//zval_ptr_dtor(&ctx->generator_closure);
	}

	ctx->phase_status = NGX_DECLINED;

	if (rc == NGX_OK || rc == NGX_HTTP_OK) {

		chain = ctx->rputs_chain;

		if (ctx->rputs_chain == NULL){
			return NGX_DECLINED;
		}

		if (!r->headers_out.status){
			r->headers_out.status = NGX_HTTP_OK;
		}

		if (r->method == NGX_HTTP_HEAD){
			rc = ngx_http_send_header(r);
			if (rc != NGX_OK){
				return rc;
			}
		}

		if (chain != NULL){
			(*chain->last)->buf->last_buf = 1;
		}

		rc = ngx_http_send_header(r);
		if (rc != NGX_OK){
			return rc;
		}

		ngx_http_output_filter(r, chain->out);
		
		ngx_http_set_ctx(r, NULL, ngx_http_php_module);

		return NGX_OK;

	}

	if (rc == NGX_ERROR || rc > NGX_OK) {

		return rc;
	}

	return rc;
}

ngx_int_t 
ngx_http_php_rewrite_inline_handler(ngx_http_request_t *r)
{
	ngx_int_t 							rc;
	ngx_http_php_rputs_chain_list_t 	*chain;
	ngx_http_php_ctx_t 					*ctx;

	TSRMLS_FETCH();

	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

	if (ctx == NULL){
		ctx = ngx_pcalloc(r->pool, sizeof(*ctx));
		if (ctx == NULL){
			return NGX_ERROR;
		}
	}
	ngx_http_set_ctx(r, ctx, ngx_http_php_module);

	ngx_php_request = r;

	if (ctx->phase_status == NGX_DECLINED){

		//ngx_http_php_rewrite_inline_uthread_routine(r);
		ngx_http_php_zend_uthread_rewrite_inline_routine(r);

		if (ctx->phase_status == NGX_AGAIN){
			
			r->main->count++;

			return NGX_AGAIN;
		} else {

			goto set_output;

		}

		/*if (ctx->rewrite_phase == NGX_OK) {

			goto set_output;
			
		} else {
			
			r->main->count++;

			return NGX_AGAIN;
		}*/

	}else {
		if (ctx->phase_status == NGX_AGAIN) {
			r->main->count++;

			return NGX_AGAIN;
		}else {
			goto set_output;
		}
		
		/*if (ctx->rewrite_phase == 2) {
			//r->main->count++;
			//return NGX_AGAIN;
			ctx->rewrite_phase = 0;
			goto set_output;
		}else {
			ctx->rewrite_phase = 0;
			goto set_output;
		}*/
	}

set_output:
	rc = ngx_php_get_request_status(TSRMLS_C);

	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

	if (ctx->generator_closure ) {
		zval_ptr_dtor(&ctx->generator_closure);
	}

	ctx->phase_status = NGX_DECLINED;

	//NGX_HTTP_PHP_R_SHUTDOWN(r);
	if (rc == NGX_OK || rc == NGX_HTTP_OK) {
		chain = ctx->rputs_chain;

		if (ctx->rputs_chain == NULL){
			return NGX_DECLINED;
		}

		if (!r->headers_out.status){
			r->headers_out.status = NGX_HTTP_OK;
		}

		if (r->method == NGX_HTTP_HEAD){
			rc = ngx_http_send_header(r);
			if (rc != NGX_OK){
				return rc;
			}
		}

		if (chain != NULL){
			(*chain->last)->buf->last_buf = 1;
		}

		rc = ngx_http_send_header(r);
		if (rc != NGX_OK){
			return rc;
		}

		ngx_http_output_filter(r, chain->out);
		
		ngx_http_set_ctx(r, NULL, ngx_http_php_module);

		return NGX_OK;

	}

	if (rc == NGX_ERROR || rc > NGX_OK) {

		return rc;
	}

	return rc;

}

static void 
ngx_http_php_access_file_uthread_routine(void *data)
{
	ngx_http_request_t *r;
	ngx_http_php_ctx_t *ctx;
	ngx_http_php_main_conf_t *pmcf;
	ngx_http_php_loc_conf_t *plcf;

	TSRMLS_FETCH();

	r = data;
	pmcf = ngx_http_get_module_main_conf(r, ngx_http_php_module);
	plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);
	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

	ctx->phase_status = NGX_OK;
	PHP_NGX_G(global_r) = r;

	//NGX_HTTP_PHP_R_INIT(r);
	//php_ngx_core_init(0 TSRMLS_CC);
	ngx_php_set_request_status(NGX_DECLINED TSRMLS_CC);
	zend_first_try {

		ngx_php_eval_file(r, pmcf->state, plcf->access_code TSRMLS_CC);

	}zend_end_try();
	//NGX_HTTP_PHP_R_SHUTDOWN(r);


	//ctx->access_phase = 1;

}

/*
static void
ngx_http_php_access_inline_uthread_routine(void *data)
{
	ngx_http_request_t *r;
	ngx_http_php_ctx_t *ctx;
	ngx_http_php_main_conf_t *pmcf;
	ngx_http_php_loc_conf_t *plcf;

	TSRMLS_FETCH();

	r = data;
	pmcf = ngx_http_get_module_main_conf(r, ngx_http_php_module);
	plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);
	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

	ctx->phase_status = NGX_OK;
	PHP_NGX_G(global_r) = r;

	//NGX_HTTP_PHP_R_INIT(r);
	//php_ngx_core_init(0 TSRMLS_CC);
	ngx_php_set_request_status(NGX_DECLINED TSRMLS_CC);
	zend_first_try {

		ngx_php_eval_code(r, pmcf->state, plcf->access_inline_code TSRMLS_CC);

	}zend_end_try();
	//NGX_HTTP_PHP_R_SHUTDOWN(r);

	//ctx->access_phase = 1;

}
*/

ngx_int_t 
ngx_http_php_access_handler(ngx_http_request_t *r)
{
	ngx_http_php_loc_conf_t *plcf;
	plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);
	if (plcf->access_handler == NULL){
		return NGX_DECLINED;
	}
	return plcf->access_handler(r);
}

ngx_int_t 
ngx_http_php_access_file_handler(ngx_http_request_t *r)
{
	ngx_int_t 							rc;
	ngx_http_php_rputs_chain_list_t 	*chain;
	ngx_http_php_ctx_t 					*ctx;

	TSRMLS_FETCH();

	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

	if (ctx == NULL){
		ctx = ngx_pcalloc(r->pool, sizeof(*ctx));
		if (ctx == NULL){
			return NGX_ERROR;
		}
	}
	ngx_http_set_ctx(r, ctx, ngx_http_php_module);

	ngx_php_request = r;

	//if (!ctx->access_phase) {
	if (ctx->phase_status == NGX_DECLINED) {

		/*if (ctx->access_uthread == NULL) {
			ctx->access_uthread = ngx_pcalloc(r->pool, sizeof(ngx_php_uthread_t));
			ctx->access_uthread->stack = ngx_pcalloc(r->pool, PHP_UTHREAD_STACK_SIZE);
		}
		ctx->access_uthread->routine = ngx_http_php_access_file_uthread_routine;
		ctx->access_uthread->data = r;

		ctx->uthread = ctx->access_uthread;

		ngx_php_uthread_create(ctx->uthread);*/

		ngx_http_php_access_file_uthread_routine(r);

		if (ctx->phase_status == NGX_AGAIN){
			
			r->main->count++;

			return NGX_AGAIN;
		} else {

			goto set_output;

		}

		/*if (ctx->access_uthread) {

			goto set_output;
			
		} else {
			
			r->main->count++;

			return NGX_AGAIN;
		}*/

	}else {
		
		if (ctx->phase_status == NGX_AGAIN) {
			r->main->count++;

			return NGX_AGAIN;
		}else {
			goto set_output;
		}

		//ctx->access_phase = 0;
		//goto set_output;
	}

set_output:
	rc = ngx_php_get_request_status(TSRMLS_C);

	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

	if (ctx->generator_closure ) {
		zval_ptr_dtor(&ctx->generator_closure);
	}

	ctx->phase_status = NGX_DECLINED;

	if (rc == NGX_OK || rc == NGX_HTTP_OK) {

		chain = ctx->rputs_chain;

		if (ctx->rputs_chain == NULL){
			return NGX_DECLINED;
		}

		if (!r->headers_out.status){
			r->headers_out.status = NGX_HTTP_OK;
		}

		if (r->method == NGX_HTTP_HEAD){
			rc = ngx_http_send_header(r);
			if (rc != NGX_OK){
				return rc;
			}
		}

		if (chain != NULL){
			(*chain->last)->buf->last_buf = 1;
		}

		rc = ngx_http_send_header(r);
		if (rc != NGX_OK){
			return rc;
		}

		ngx_http_output_filter(r, chain->out);
		
		ngx_http_set_ctx(r, NULL, ngx_http_php_module);

		return NGX_HTTP_OK;

	}

	if (rc == NGX_ERROR || rc > NGX_OK) {
		
		return rc;
	}

	return rc;
}

ngx_int_t 
ngx_http_php_access_inline_handler(ngx_http_request_t *r)
{
	ngx_int_t 							rc;
	ngx_http_php_rputs_chain_list_t 	*chain;
	ngx_http_php_ctx_t 					*ctx;

	TSRMLS_FETCH();

	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

	if (ctx == NULL){
		ctx = ngx_pcalloc(r->pool, sizeof(*ctx));
		if (ctx == NULL){
			return NGX_ERROR;
		}
	}
	ngx_http_set_ctx(r, ctx, ngx_http_php_module);

	ngx_php_request = r;

	//if (!ctx->access_phase) {
	if (ctx->phase_status == NGX_DECLINED) {

		//ngx_http_php_access_inline_uthread_routine(r);
		ngx_http_php_zend_uthread_access_inline_routine(r);

		if (ctx->phase_status == NGX_AGAIN){
			
			r->main->count++;

			return NGX_AGAIN;
		} else {

			goto set_output;

		}

		/*if (ctx->access_uthread) {

			goto set_output;
			
		} else {
			
			r->main->count++;

			return NGX_AGAIN;
		}*/

	}else {
		
		if (ctx->phase_status == NGX_AGAIN) {
			r->main->count++;

			return NGX_AGAIN;
		}else {
			goto set_output;
		}

		//ctx->access_phase = 0;
		//goto set_output;
	}

set_output:
	rc = ngx_php_get_request_status(TSRMLS_C);

	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

	if (ctx->generator_closure ) {
		zval_ptr_dtor(&ctx->generator_closure);
	}

	ctx->phase_status = NGX_DECLINED;

	if (rc == NGX_OK || rc == NGX_HTTP_OK) {

		chain = ctx->rputs_chain;

		if (ctx->rputs_chain == NULL){
			return NGX_DECLINED;
		}

		if (!r->headers_out.status){
			r->headers_out.status = NGX_HTTP_OK;
		}

		if (r->method == NGX_HTTP_HEAD){
			rc = ngx_http_send_header(r);
			if (rc != NGX_OK){
				return rc;
			}
		}

		if (chain != NULL){
			(*chain->last)->buf->last_buf = 1;
		}

		rc = ngx_http_send_header(r);
		if (rc != NGX_OK){
			return rc;
		}

		ngx_http_output_filter(r, chain->out);
		
		ngx_http_set_ctx(r, NULL, ngx_http_php_module);

		return NGX_HTTP_OK;

	}

	if (rc == NGX_ERROR || rc > NGX_OK) {
		
		return rc;
	}

	return rc;
}

static void 
ngx_http_php_content_file_uthread_routine(void *data)
{
	ngx_http_request_t *r;
	ngx_http_php_ctx_t *ctx;
	ngx_http_php_main_conf_t *pmcf;
	ngx_http_php_loc_conf_t *plcf;

	TSRMLS_FETCH();

	r = data;
	pmcf = ngx_http_get_module_main_conf(r, ngx_http_php_module);
	plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);
	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

	ctx->phase_status = NGX_OK;
	PHP_NGX_G(global_r) = r;

	//NGX_HTTP_PHP_R_INIT(r);

	//php_ngx_core_init(0 TSRMLS_CC);
	ngx_php_set_request_status(NGX_OK TSRMLS_CC);
	zend_first_try {

		ngx_php_eval_file(r, pmcf->state, plcf->content_code TSRMLS_CC);

	} zend_end_try();
	//NGX_HTTP_PHP_R_SHUTDOWN(r);

	//ctx->content_phase = 1;

}

/*
static void 
ngx_http_php_content_inline_uthread_routine(void *data)
{
	ngx_http_request_t *r;
	ngx_http_php_ctx_t *ctx;
	ngx_http_php_main_conf_t *pmcf;
	ngx_http_php_loc_conf_t *plcf;

	TSRMLS_FETCH();

	r = data;
	pmcf = ngx_http_get_module_main_conf(r, ngx_http_php_module);
	plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);
	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

	ctx->phase_status = NGX_OK;
	PHP_NGX_G(global_r) = r;

	//NGX_HTTP_PHP_R_INIT(r);

	//php_ngx_core_init(0 TSRMLS_CC);
	ngx_php_set_request_status(NGX_OK TSRMLS_CC);
	zend_first_try {

		ngx_php_eval_code(r, pmcf->state, plcf->content_inline_code TSRMLS_CC);

	} zend_end_try();
	//NGX_HTTP_PHP_R_SHUTDOWN(r);

	//ctx->content_phase = 1;

}
*/

static void 
ngx_http_php_read_request_body_callback(ngx_http_request_t *r)
{
    ngx_http_php_ctx_t *ctx;

    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    ctx->read_request_body_done = 1;

    ngx_php_debug("%d, %d", (int)ctx->request_body_more, (int)ctx->read_request_body_done);

    if (ctx->request_body_more) {
        ctx->request_body_more = 0;
        ngx_http_core_run_phases(r);
    }
}

ngx_int_t
ngx_http_php_content_handler(ngx_http_request_t *r)
{
	ngx_http_php_loc_conf_t *plcf;
	plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);
	if (plcf->content_handler == NULL){
		return NGX_DECLINED;
	}
	return plcf->content_handler(r);
}

ngx_int_t 
ngx_http_php_content_file_handler(ngx_http_request_t *r)
{
	ngx_int_t 							rc;
	ngx_http_php_rputs_chain_list_t 	*chain;
	ngx_http_php_ctx_t 					*ctx;

	TSRMLS_FETCH();

	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

	if (ctx == NULL){
		ctx = ngx_pcalloc(r->pool, sizeof(*ctx));
		if (ctx == NULL){
			return NGX_ERROR;
		}
	}

	ctx->request_body_more = 1;
	ngx_http_set_ctx(r, ctx, ngx_http_php_module);

	ngx_php_request = r;

	ngx_php_set_request_status(NGX_OK TSRMLS_CC);

	if (r->method == NGX_HTTP_POST){
		return ngx_http_php_content_post_handler(r);
	}

	//if (!ctx->content_phase) {
	if (ctx->phase_status == NGX_DECLINED) {

		/*if (ctx->content_uthread == NULL) {
			ctx->content_uthread = ngx_pcalloc(r->pool, sizeof(ngx_php_uthread_t));
			ctx->content_uthread->stack = ngx_pcalloc(r->pool, PHP_UTHREAD_STACK_SIZE);
		}
		ctx->content_uthread->routine = ngx_http_php_content_file_uthread_routine;
		ctx->content_uthread->data = r;

		ctx->uthread = ctx->content_uthread;

		ngx_php_uthread_create(ctx->uthread);*/

		ngx_http_php_content_file_uthread_routine(r);

		if (ctx->phase_status == NGX_AGAIN){
			
			r->main->count++;

			return NGX_AGAIN;
		} else {

			goto set_output;

		}

		/*if (ctx->content_uthread) {

			goto set_output;
			
		} else {
			
			r->main->count++;

			return NGX_AGAIN;
		}*/

	}else {

		if (ctx->phase_status == NGX_AGAIN) {
			r->main->count++;

			return NGX_AGAIN;
		}else {
			goto set_output;
		}

		//ctx->content_phase = 0;
		//goto set_output;
	}

set_output:
	rc = ngx_php_get_request_status(TSRMLS_C);

	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

	if (ctx->generator_closure ) {
		//zval_ptr_dtor(&ctx->generator_closure);
	}

	ctx->phase_status = NGX_DECLINED;

	if (rc == NGX_OK || rc == NGX_DECLINED) {

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

		if (!r->headers_out.status){
			r->headers_out.status = NGX_HTTP_OK;
		}

		if (r->method == NGX_HTTP_HEAD){
			rc = ngx_http_send_header(r);
			if (rc != NGX_OK){
				return rc;
			}
		}

		if (chain != NULL){
			(*chain->last)->buf->last_buf = 1;
		}

		rc = ngx_http_send_header(r);
		if (rc != NGX_OK){
			return rc;
		}

		ngx_http_output_filter(r, chain->out);

		ngx_http_set_ctx(r, NULL, ngx_http_php_module);

		return NGX_OK;
	}

	if (rc == NGX_ERROR || rc > NGX_OK) {
		return rc;
	}

	return NGX_OK;
}

ngx_int_t 
ngx_http_php_content_inline_handler(ngx_http_request_t *r)
{
	ngx_int_t 							rc;
	ngx_http_php_rputs_chain_list_t 	*chain;
	ngx_http_php_ctx_t 					*ctx;

	TSRMLS_FETCH();

	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

	if (ctx == NULL){
		ctx = ngx_pcalloc(r->pool, sizeof(*ctx));
		if (ctx == NULL){
			return NGX_ERROR;
		}
	}

	ctx->request_body_more = 1;
	ngx_http_set_ctx(r, ctx, ngx_http_php_module);

	ngx_php_request = r;

	ngx_php_set_request_status(NGX_OK TSRMLS_CC);

	/*if (r->method == NGX_HTTP_POST){
		return ngx_http_php_content_post_handler(r);
	}*/

	if (r->method == NGX_HTTP_POST && !ctx->read_request_body_done) {
        r->request_body_in_single_buf = 1;
        r->request_body_in_persistent_file = 1;
        r->request_body_in_clean_file = 1;

        ngx_php_debug("read_request_body_done: %d", (int)ctx->read_request_body_done);

        rc = ngx_http_read_client_request_body(r, ngx_http_php_read_request_body_callback);
        ngx_php_debug("%d, %d, %d", (int)rc, (int)ctx->request_body_more, (int)ctx->read_request_body_done);
        if (rc == NGX_ERROR || rc >= NGX_HTTP_SPECIAL_RESPONSE) {
#if (nginx_version < 1002006) || (nginx_version >= 1003000 && nginx_version < 1003009)
            r->main->count--;
#endif
            return rc;
        }

        if (rc == NGX_AGAIN) {
            ctx->request_body_more = 1;
            ctx->read_request_body_done = 0;
            return NGX_AGAIN;
        }

        return rc;
    }

	//if (!ctx->content_phase) {
	if (ctx->phase_status == NGX_DECLINED) {

		//ngx_http_php_content_inline_uthread_routine(r);
		ngx_http_php_zend_uthread_content_inline_routine(r);

		ngx_php_debug("ctx->phase_status: %d", (int) ctx->phase_status);

		if (ctx->phase_status == NGX_AGAIN){
			
			r->main->count++;

			return NGX_AGAIN;
		} else {

			goto set_output;

		}

		/*if (ctx->content_uthread) {

			goto set_output;
			
		} else {
			
			r->main->count++;

			return NGX_AGAIN;
		}*/

	}else {
		
		if (ctx->phase_status == NGX_AGAIN) {
			r->main->count++;

			return NGX_AGAIN;
		}else {
			goto set_output;
		}

		//ctx->content_phase = 0;
		//goto set_output;
	}

set_output:
	rc = ngx_php_get_request_status(TSRMLS_C);

	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

	if (ctx->generator_closure ) {
		//zval_ptr_dtor(&ctx->generator_closure);
	}

	ctx->phase_status = NGX_DECLINED;

	if (rc == NGX_OK || rc == NGX_DECLINED) {

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

		if (!r->headers_out.status){
			r->headers_out.status = NGX_HTTP_OK;
		}

		if (r->method == NGX_HTTP_HEAD){
			rc = ngx_http_send_header(r);
			if (rc != NGX_OK){
				return rc;
			}
		}

		if (chain != NULL){
			(*chain->last)->buf->last_buf = 1;
		}

		rc = ngx_http_send_header(r);
		if (rc != NGX_OK){
			return rc;
		}

		ngx_http_output_filter(r, chain->out);

		ngx_http_set_ctx(r, NULL, ngx_http_php_module);

		return NGX_OK;
	}

	if (rc == NGX_ERROR || rc > NGX_OK) {
		
		return rc;
	}

	return NGX_OK;
}

ngx_int_t 
ngx_http_php_content_post_handler(ngx_http_request_t *r)
{
	TSRMLS_FETCH();

	ngx_http_php_main_conf_t *pmcf = ngx_http_get_module_main_conf(r, ngx_http_php_module);
	ngx_http_php_loc_conf_t *plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);

	ngx_int_t rc;
	ngx_http_php_rputs_chain_list_t *chain;

	ngx_http_php_ctx_t *ctx;
	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

	if (ctx == NULL){
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "Get ngx_http_php_ctx_t fail");
		return NGX_HTTP_INTERNAL_SERVER_ERROR;
	}

	if (ctx->request_body_more){
		rc = ngx_http_php_request_read_body(r);
		return rc;
	}

	/*NGX_HTTP_PHP_NGX_INIT;
		// main init
		if (pmcf->init_inline_code != NGX_CONF_UNSET_PTR){
			ngx_php_ngx_run(r, pmcf->state, pmcf->init_inline_code);
		}
		if (pmcf->init_code != NGX_CONF_UNSET_PTR){
			ngx_php_ngx_run(r, pmcf->state, pmcf->init_code);
		}
		// location rewrite
		if (plcf->rewrite_code != NGX_CONF_UNSET_PTR){
			ngx_php_ngx_run(r, pmcf->state, plcf->rewrite_code);
		}
		if (plcf->rewrite_inline_code != NGX_CONF_UNSET_PTR){
			ngx_php_ngx_run(r, pmcf->state, plcf->rewrite_inline_code);
		}
		// location access
		if (plcf->access_code != NGX_CONF_UNSET_PTR){
			ngx_php_ngx_run(r, pmcf->state, plcf->access_code);
		}
		if (plcf->access_inline_code != NGX_CONF_UNSET_PTR){
			ngx_php_ngx_run(r, pmcf->state, plcf->access_inline_code);
		}
		// location content
		ngx_php_ngx_run(r, pmcf->state, plcf->content_inline_code);
	NGX_HTTP_PHP_NGX_SHUTDOWN;*/

	NGX_HTTP_PHP_R_INIT(r);

	zend_first_try {

		//ngx_php_ngx_run(r, pmcf->state, plcf->content_inline_code);
		if (plcf->content_inline_code != NGX_CONF_UNSET_PTR) {
			ngx_php_eval_code(r, pmcf->state, plcf->content_inline_code TSRMLS_CC);
		}

		if (plcf->content_code != NGX_CONF_UNSET_PTR) {
			ngx_php_eval_file(r, pmcf->state, plcf->content_code TSRMLS_CC);
		}

		
	} zend_end_try();

	NGX_HTTP_PHP_R_SHUTDOWN(r);

	rc = ngx_php_get_request_status(TSRMLS_C);

	if (rc == NGX_OK || rc == NGX_HTTP_OK) {

		ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);
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

		if (!r->headers_out.status){
			r->headers_out.status = NGX_HTTP_OK;
		}

		if (r->method == NGX_HTTP_HEAD){
			rc = ngx_http_send_header(r);
			if (rc != NGX_OK){
				return rc;
			}
		}

		if (chain != NULL){
			(*chain->last)->buf->last_buf = 1;
		}

		rc = ngx_http_send_header(r);
		if (rc != NGX_OK){
			return rc;
		}

		ngx_http_output_filter(r, chain->out);

		ngx_http_set_ctx(r, NULL, ngx_http_php_module);

		return NGX_OK;
	}

	if (rc == NGX_ERROR || rc > NGX_OK) {
		
		return rc;
	}

	return NGX_OK;
}

/*
ngx_int_t 
ngx_http_php_content_async_handler(ngx_http_request_t *r)
{
	ngx_http_php_loc_conf_t *plcf;
	plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);
	if (plcf->content_async_handler == NULL){
		return NGX_DECLINED;
	}
	return plcf->content_async_handler(r);
}

void *
ngx_http_php_async_inline_thread(void *arg)
{
	TSRMLS_FETCH();
	ngx_http_request_t *r = (ngx_http_request_t *)arg;

	//ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "pthread");
	ngx_http_php_main_conf_t *pmcf = ngx_http_get_module_main_conf(r, ngx_http_php_module);
	ngx_http_php_loc_conf_t *plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);

	//ngx_php_ngx_run(r, pmcf->state, plcf->content_async_inline_code);

	NGX_HTTP_PHP_NGX_INIT;
		
		ngx_location_init(0 TSRMLS_CC);
		PHP_NGX_G(global_r) = r;

		ngx_php_ngx_run(r, pmcf->state, plcf->content_async_inline_code);
		//zend_eval_string_ex("echo 0;", NULL, "ngx_php run code", 1 TSRMLS_CC);

		ngx_http_php_ctx_t *ctx;
		ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

		//ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "child: %p", &(ctx->cond));
		//ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "child uri %d", ctx->capture_uri.len);

		//pthread_mutex_lock(&(ctx->mutex));
		//pthread_cond_wait(&(ctx->cond), &(ctx->mutex));
		//pthread_mutex_unlock(&(ctx->mutex));

  		if (ctx->is_capture_multi == 1){
  			zval *argv[1];
  			zval retval;

  			MAKE_STD_ZVAL(argv[0]);
  			array_init(argv[0]);

  			ngx_http_php_capture_node_t *capture_node = ctx->capture_multi->elts;
  			ngx_uint_t i;
  			for (i = 0; i < ctx->capture_multi->nelts; i++,capture_node++){
  				add_next_index_stringl(argv[0], (char *)capture_node->capture_str.data, capture_node->capture_str.len, 1);
  			}

  			if (call_user_function(EG(function_table), NULL, ctx->closure, &retval, 1, argv TSRMLS_CC) == FAILURE)
	  		{
	    		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed calling closure");
	    		//return ;
	  		}
	  		//zval_dtor(args[0]);
	  		zval_ptr_dtor(&argv[0]);
	  		zval_dtor(&retval);

  			ngx_array_destroy(ctx->capture_multi);
  		}else {
  			zval *argv[1];
			zval retval;

			MAKE_STD_ZVAL(argv[0]);
			//ZVAL_STRINGL(argv[0], (char *)ctx->capture_buf->pos, ctx->capture_buf->last - ctx->capture_buf->pos, 1);
			ZVAL_STRINGL(argv[0], (char *)ctx->capture_str.data, ctx->capture_str.len, 1);

			if (call_user_function(EG(function_table), NULL, ctx->closure, &retval, 1, argv TSRMLS_CC) == FAILURE)
	  		{
	    		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed calling closure");
	    		//return ;
	  		}
	  		//zval_dtor(args[0]);
	  		zval_ptr_dtor(&argv[0]);
	  		zval_dtor(&retval);
  		}
	NGX_HTTP_PHP_NGX_SHUTDOWN;

	ngx_http_php_ctx_t *ctx;
	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);
	ctx->enable_async = 0;
	ctx->enable_thread = 0;
	ngx_http_set_ctx(r, ctx, ngx_http_php_module);

	return NULL;
}

ngx_int_t 
ngx_http_php_content_async_inline_handler(ngx_http_request_t *r)
{
	//TSRMLS_FETCH();

	//ngx_http_php_main_conf_t *pmcf = ngx_http_get_module_main_conf(r, ngx_http_php_module);
	//ngx_http_php_loc_conf_t *plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);

	ngx_int_t rc;
	ngx_http_php_ctx_t *ctx;
	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

	if (ctx == NULL){
		ctx = ngx_pcalloc(r->pool, sizeof(*ctx));
		if (ctx == NULL){
			return NGX_ERROR;
		}
	}

	ctx->enable_async = 0;
	ctx->enable_thread = 1;

	ctx->is_capture_multi = 0;
	ctx->capture_multi_complete_total = 0;
	ctx->is_capture_multi_complete = 0;

	ctx->error = NGX_OK;

	ctx->request_body_more = 1;

	pthread_mutex_init(&(ctx->mutex), NULL);
	pthread_cond_init(&(ctx->cond), NULL);
	ngx_http_set_ctx(r, ctx, ngx_http_php_module);

	ngx_php_request = r;

	if (r->method == NGX_HTTP_POST){
		return ngx_http_php_content_post_handler(r);
	}

	//pthread_t id_1;
	//pthread_create(&id_1, NULL, ngx_http_php_test_thread, r);
	//pthread_join(id_1, NULL);

	//ngx_http_php_request_init(r TSRMLS_CC);
	//php_ngx_request_init(TSRMLS_C);	

	//NGX_HTTP_PHP_NGX_INIT;
		// main init
		
		//pthread_t id_1;
		pthread_create(&(ctx->pthread_id), NULL, ngx_http_php_async_inline_thread, r);


		//ctx->capture_uri.data = (u_char *)"/list=s_sh000001";
		//ctx->capture_uri.len = 16;

		//ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "main %d", ctx->enable_async);

		for ( ;; ){
			usleep(1);
			//ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);
			//ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "main %d", ctx->enable_async);

			if (ctx->enable_async == 1 || ctx->enable_thread == 0){
				//ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "main %d", ctx->enable_async);
				break;
			}
		}

		//ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "main %d", ctx->enable_async);

		//ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "main uri %s", ctx->capture_uri.data);

		if (ctx->enable_async == 1){
			if (ctx->is_capture_multi == 0){
				ngx_http_php_subrequest_post(r);
			} else {
				ngx_http_php_subrequest_post_multi(r);
			}
			//ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "%d", ctx->enable_async);

				//pthread_mutex_lock(&(ctx->mutex));
				//pthread_cond_signal(&(ctx->cond));
				//pthread_mutex_unlock(&(ctx->mutex));

			return NGX_DONE;
		}
		// location content
		//ngx_php_ngx_run(r, pmcf->state, plcf->content_async_inline_code);

		//ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

		//if (ctx->enable_async == 1){
		//	return NGX_DONE;
		//}


	//NGX_HTTP_PHP_NGX_SHUTDOWN;

	ngx_http_php_rputs_chain_list_t *chain;
	
	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);
	//if (ctx->enable_async == 1){
	//	return NGX_DONE;
	//}
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
			return rc;
		}
	}

	if (chain != NULL){
		(*chain->last)->buf->last_buf = 1;
	}

	rc = ngx_http_send_header(r);
	if (rc != NGX_OK){
		return rc;
	}

	ngx_http_output_filter(r, chain->out);

	ngx_http_set_ctx(r, NULL, ngx_http_php_module);

	return NGX_OK;

}
*/

/*
void *
ngx_http_php_sync_inline_thread(void *arg)
{
	TSRMLS_FETCH();
	ngx_http_request_t *r = (ngx_http_request_t *)arg;

	//ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "pthread");
	ngx_http_php_main_conf_t *pmcf = ngx_http_get_module_main_conf(r, ngx_http_php_module);
	ngx_http_php_loc_conf_t *plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);

	//ngx_php_ngx_run(r, pmcf->state, plcf->content_async_inline_code);

	NGX_HTTP_PHP_NGX_INIT;

		php_ngx_core_init(0 TSRMLS_CC);
		ngx_location_init(0 TSRMLS_CC);
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
*/

/*
ngx_int_t 
ngx_http_php_content_sync_inline_handler(ngx_http_request_t *r)
{
	ngx_int_t rc;

	ngx_http_php_ctx_t *ctx;
	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

	if (ctx == NULL){
		ctx = ngx_pcalloc(r->pool, sizeof(*ctx));
		if (ctx == NULL){
			return NGX_ERROR;
		}
	}

	ctx->enable_async = 0;
	ctx->enable_upstream = 0;
	ctx->enable_thread = 1;

	ctx->read_or_write = 0;

	ctx->is_capture_multi = 0;
	ctx->capture_multi_complete_total = 0;
	ctx->is_capture_multi_complete = 0;

	ctx->error = NGX_OK;

	ctx->request_body_more = 1;

	ctx->receive_stat = 0;
	ctx->receive_total = 0;

	pthread_mutex_init(&(ctx->mutex), NULL);
	pthread_cond_init(&(ctx->cond), NULL);
	ngx_http_set_ctx(r, ctx, ngx_http_php_module);

	ngx_php_request = r;

	if (r->method == NGX_HTTP_POST){
		return ngx_http_php_content_post_handler(r);
	}

	pthread_create(&(ctx->pthread_id), NULL, ngx_http_php_sync_inline_thread, r);

	pthread_detach(ctx->pthread_id);

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

	if (ctx->enable_async == 1){
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

	ngx_http_php_rputs_chain_list_t *chain;
	
	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);
	
	//pthread_join(ctx->pthread_id, NULL);

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
			return rc;
		}
	}

	if (chain != NULL){
		(*chain->last)->buf->last_buf = 1;
	}

	rc = ngx_http_send_header(r);
	if (rc != NGX_OK){
		return rc;
	}

	ngx_http_output_filter(r, chain->out);

	ngx_http_set_ctx(r, NULL, ngx_http_php_module);

	return NGX_OK;


}
*/

/*
ngx_int_t 
ngx_http_php_content_thread_inline_handler(ngx_http_request_t *r)
{
	ngx_int_t rc;

	ngx_http_php_ctx_t *ctx;
	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

	if (ctx == NULL){
		ctx = ngx_pcalloc(r->pool, sizeof(*ctx));
		if (ctx == NULL){
			return NGX_ERROR;
		}
	}

	ctx->enable_async = 0;
	ctx->enable_upstream = 0;
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

	pthread_mutex_init(&(ctx->mutex), NULL);
	pthread_cond_init(&(ctx->cond), NULL);
	ngx_http_set_ctx(r, ctx, ngx_http_php_module);

	ngx_php_request = r;

	if (r->method == NGX_HTTP_POST){
		return ngx_http_php_content_post_handler(r);
	}

	//ngx_http_php_request_cleanup_handler(r);

	pthread_create(&(ctx->pthread_id), NULL, ngx_http_php_sync_inline_thread, r);

	pthread_detach(ctx->pthread_id);
	
	for ( ;; ){
		usleep(1);
		//ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);
		//ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "main %d %d", ctx->enable_sleep, ctx->enable_thread);

		if (ctx->enable_async == 1 || 
			ctx->enable_upstream == 1 || 
			ctx->enable_sleep == 1 ||
			ctx->enable_thread == 0){
			break;
		}
	}

	if (ctx->enable_async == 1){
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
	}
	
	ngx_http_php_rputs_chain_list_t *chain;
	
	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);
	
	//pthread_join(ctx->pthread_id, NULL);

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
			return rc;
		}
	}

	if (chain != NULL){
		(*chain->last)->buf->last_buf = 1;
	}

	rc = ngx_http_send_header(r);
	if (rc != NGX_OK){
		return rc;
	}

	ngx_http_output_filter(r, chain->out);

	ngx_http_set_ctx(r, NULL, ngx_http_php_module);

	return NGX_OK;

}

void *
ngx_http_php_sync_file_thread(void *arg)
{
	TSRMLS_FETCH();
	ngx_http_request_t *r = (ngx_http_request_t *)arg;

	//ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "pthread");
	ngx_http_php_main_conf_t *pmcf = ngx_http_get_module_main_conf(r, ngx_http_php_module);
	ngx_http_php_loc_conf_t *plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);

	//ngx_php_ngx_run(r, pmcf->state, plcf->content_async_inline_code);

	NGX_HTTP_PHP_NGX_INIT;

		php_ngx_core_init(0 TSRMLS_CC);
		ngx_location_init(0 TSRMLS_CC);
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

ngx_int_t 
ngx_http_php_content_thread_file_handler(ngx_http_request_t *r)
{
	ngx_int_t rc;

	ngx_http_php_ctx_t *ctx;
	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

	if (ctx == NULL){
		ctx = ngx_pcalloc(r->pool, sizeof(*ctx));
		if (ctx == NULL){
			return NGX_ERROR;
		}
	}

	ctx->enable_async = 0;
	ctx->enable_upstream = 0;
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

	pthread_mutex_init(&(ctx->mutex), NULL);
	pthread_cond_init(&(ctx->cond), NULL);
	ngx_http_set_ctx(r, ctx, ngx_http_php_module);

	ngx_php_request = r;

	if (r->method == NGX_HTTP_POST){
		return ngx_http_php_content_post_handler(r);
	}

	//ngx_http_php_request_cleanup_handler(r);

	pthread_create(&(ctx->pthread_id), NULL, ngx_http_php_sync_file_thread, r);

	pthread_detach(ctx->pthread_id);

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

	if (ctx->enable_async == 1){
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
	}

	ngx_http_php_rputs_chain_list_t *chain;
	
	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);
	
	//pthread_join(ctx->pthread_id, NULL);

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
			return rc;
		}
	}

	if (chain != NULL){
		(*chain->last)->buf->last_buf = 1;
	}

	rc = ngx_http_send_header(r);
	if (rc != NGX_OK){
		return rc;
	}

	ngx_http_output_filter(r, chain->out);

	ngx_http_set_ctx(r, NULL, ngx_http_php_module);

	return NGX_OK;
}
*/

ngx_int_t 
ngx_http_php_log_handler(ngx_http_request_t *r)
{
	ngx_http_php_loc_conf_t *plcf;
	plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);
	if (plcf->log_handler == NULL){
		return NGX_DECLINED;
	}
	return plcf->log_handler(r);
}

ngx_int_t 
ngx_http_php_log_file_handler(ngx_http_request_t *r)
{
	TSRMLS_FETCH();

	ngx_http_php_main_conf_t *pmcf = ngx_http_get_module_main_conf(r, ngx_http_php_module);
	ngx_http_php_loc_conf_t *plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);
	ngx_http_php_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

	if (ctx == NULL){
		ctx = ngx_pcalloc(r->pool, sizeof(*ctx));
		if (ctx == NULL){
			return NGX_ERROR;
		}
	}
	ngx_http_set_ctx(r, ctx, ngx_http_php_module);

	ngx_php_request = r;

	/*if (r->method == NGX_HTTP_POST){
		return ngx_http_php_content_post_handler(r);
	}*/

	/*NGX_HTTP_PHP_NGX_INIT;
		// main init
		if (pmcf->init_inline_code != NGX_CONF_UNSET_PTR){
			ngx_php_ngx_run(r, pmcf->state, pmcf->init_inline_code);
		}
		if (pmcf->init_code != NGX_CONF_UNSET_PTR){
			ngx_php_ngx_run(r, pmcf->state, pmcf->init_code);
		}
		
		// location log
		ngx_php_ngx_run(r, pmcf->state, plcf->log_code);
	NGX_HTTP_PHP_NGX_SHUTDOWN;*/

	NGX_HTTP_PHP_R_INIT(r);
	
	zend_first_try {

		//ngx_php_ngx_run(r, pmcf->state, plcf->log_code);
		ngx_php_eval_file(r, pmcf->state, plcf->log_code TSRMLS_CC);

	} zend_end_try();

	ngx_http_php_request_cleanup_handler(r);
	
	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

	ngx_http_set_ctx(r, NULL, ngx_http_php_module);

	return NGX_OK;
}

ngx_int_t 
ngx_http_php_log_inline_handler(ngx_http_request_t *r)
{
	TSRMLS_FETCH();

	ngx_http_php_main_conf_t *pmcf = ngx_http_get_module_main_conf(r, ngx_http_php_module);
	ngx_http_php_loc_conf_t *plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);

	ngx_http_php_ctx_t *ctx;
	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

	if (ctx == NULL){
		ctx = ngx_pcalloc(r->pool, sizeof(*ctx));
		if (ctx == NULL){
			return NGX_ERROR;
		}
	}

	ngx_http_set_ctx(r, ctx, ngx_http_php_module);

	ngx_php_request = r;

	/*if (r->method == NGX_HTTP_POST){
		return ngx_http_php_content_post_handler(r);
	}*/

	//ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, " %s", plcf->log_inline_code->code.string);

	/*NGX_HTTP_PHP_NGX_INIT;
		// main init
		if (pmcf->init_inline_code != NGX_CONF_UNSET_PTR){
			ngx_php_ngx_run(r, pmcf->state, pmcf->init_inline_code);
		}
		if (pmcf->init_code != NGX_CONF_UNSET_PTR){
			ngx_php_ngx_run(r, pmcf->state, pmcf->init_code);
		}
		
		// location log
		ngx_php_ngx_run(r, pmcf->state, plcf->log_inline_code);
	NGX_HTTP_PHP_NGX_SHUTDOWN;*/

	NGX_HTTP_PHP_R_INIT(r);
	
	zend_first_try {

		//ngx_php_ngx_run(r, pmcf->state, plcf->log_inline_code);
		ngx_php_eval_file(r, pmcf->state, plcf->log_inline_code TSRMLS_CC);
		
	} zend_end_try();

	ngx_http_php_request_cleanup_handler(r);
	
	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

	ngx_http_set_ctx(r, NULL, ngx_http_php_module);

	return NGX_OK;
}


ngx_int_t 
ngx_http_php_set_inline_handler(ngx_http_request_t *r, ngx_str_t *val, ngx_http_variable_value_t *v, void *data)
{
	//ngx_http_php_main_conf_t *pmcf = ngx_http_get_module_main_conf(r, ngx_http_php_module);
	ngx_http_php_loc_conf_t *plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);

	ngx_http_php_set_var_data_t *filter_data;
	filter_data = data;

	/*if (filter_data->result.len <= 0){
		val->data = NULL;
		val->len = 0;
	}else {
		val->data = ngx_palloc(r->pool, filter_data->result.len);
		ngx_memcpy(val->data, filter_data->result.data, filter_data->result.len);
		val->len = filter_data->result.len;
	}*/

	if (ngx_strncmp(plcf->content_inline_code->code.string, filter_data->var_name.data, filter_data->var_name.len) == 0){
		plcf->content_inline_code = filter_data->code;
	}
	
	//ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "%s", plcf->content_inline_code->code.string);

	//ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "%s", filter_data->script.data);

	return NGX_OK;
}

ngx_int_t 
ngx_http_php_set_run_inline_handler(ngx_http_request_t *r, ngx_str_t *val, ngx_http_variable_value_t *v, void *data)
{
	//ngx_http_php_main_conf_t *pmcf = ngx_http_get_module_main_conf(r, ngx_http_php_module);
	ngx_http_php_loc_conf_t *plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);

	ngx_http_php_set_var_data_t *filter_data;
	filter_data = data;

	/*if (filter_data->result.len <= 0){
		val->data = NULL;
		val->len = 0;
	}else {
		val->data = ngx_palloc(r->pool, filter_data->result.len);
		ngx_memcpy(val->data, filter_data->result.data, filter_data->result.len);
		val->len = filter_data->result.len;
	}*/

	if (ngx_strncmp(plcf->content_inline_code->code.string, filter_data->var_name.data, filter_data->var_name.len) == 0){
		//plcf->content_inline_code = filter_data->code;
		if (filter_data->result.data != NULL){
			plcf->content_inline_code = filter_data->code;
		}

	}
	
	//ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "%s", plcf->content_inline_code->code.string);

	//ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "%s", filter_data->script.data);

	/*if (filter_data->result.data != NULL){
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "%s", filter_data->code->code.string);
	}*/

	return NGX_OK;
}

ngx_int_t 
ngx_http_php_set_file_handler(ngx_http_request_t *r, ngx_str_t *val, ngx_http_variable_value_t *v, void *data)
{
	ngx_http_php_loc_conf_t *plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);

	ngx_http_php_set_var_data_t *filter_data;
	filter_data = data;

	if (ngx_strncmp(plcf->content_code->code.file, filter_data->var_name.data, filter_data->var_name.len) == 0){
		plcf->content_code = filter_data->code;
	}

	return NGX_OK;
}

ngx_int_t 
ngx_http_php_set_run_file_handler(ngx_http_request_t *r, ngx_str_t *val, ngx_http_variable_value_t *v, void *data)
{
	ngx_http_php_loc_conf_t *plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);

	ngx_http_php_set_var_data_t *filter_data;
	filter_data = data;

	if (ngx_strncmp(plcf->content_inline_code->code.string, filter_data->var_name.data, filter_data->var_name.len) == 0){
		if (filter_data->result.data != NULL){
			plcf->content_inline_code = filter_data->code;
		}

	}

	/*if (filter_data->result.data != NULL){
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "%s", filter_data->result.data);
	}*/

	return NGX_OK;
}






