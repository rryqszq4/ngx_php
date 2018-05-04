/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */

#include "ngx_php_debug.h"
#include "ngx_http_php_coroutine.h"

ngx_php_coroutine_t *
ngx_http_php_coroutine_alloc(ngx_http_request_t *r)
{
	ngx_http_php_ctx_t *ctx;
	ngx_php_coroutine_t *coro;

	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);
	if (ctx == NULL) {
		return NULL;
	}

	coro = ctx->coro;
	if (coro == NULL) {
		coro = ngx_pcalloc(r->pool, sizeof(ngx_php_coroutine_t));
		if (coro == NULL) {
			return NULL;
		}
		
		if (!coro->stack) {
			coro->stack = ngx_palloc(r->pool, PHP_COROUTINE_STACK_SIZE);
		}
	}

	return coro;
}

ngx_int_t 
ngx_http_php_coroutine_run(ngx_http_request_t *r)
{
	ngx_http_php_ctx_t *ctx;

	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

	if (ngx_php_coroutine_create(ctx->coro) != 0) {
		return NGX_ERROR;
	}

	return NGX_OK;
}

ngx_int_t 
ngx_http_php_coroutine_yield(ngx_http_request_t *r)
{
	ngx_http_php_ctx_t *ctx;

	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

	zend_execute_data *current_execute_data = EG(current_execute_data);
    zend_op **opline_ptr;
    opline_ptr = EG(opline_ptr);
    zend_vm_stack current_stack = EG(argument_stack);
    //ctx->op_array = (zend_op_array*)emalloc(sizeof(zend_op_array));
    ctx->op_array = EG(active_op_array);
    ngx_php_debug("%d\n", ctx->op_array->fn_flags);
    ctx->op_array->fn_flags |= ZEND_ACC_GENERATOR;
    ctx->execute_data = zend_create_execute_data_from_op_array(ctx->op_array, 0 TSRMLS_CC);
    EG(current_execute_data) = current_execute_data;
    EG(opline_ptr) = opline_ptr;
    ctx->argument_stack = EG(argument_stack);
    EG(argument_stack) = current_stack;

	if (ngx_php_coroutine_yield(ctx->coro) != 0) {
		return NGX_ERROR;
	}

	r = ngx_php_request;

    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    ctx->execute_data->opline++;
    EG(current_execute_data) = ctx->execute_data;
    EG(argument_stack) = ctx->argument_stack;
    EG(return_value_ptr_ptr) = ctx->return_value_ptr_ptr;
    ctx->op_array->fn_flags &= ~ZEND_ACC_GENERATOR;
    ngx_php_debug("%d\n", ctx->op_array->fn_flags);

	return NGX_OK;
}

ngx_int_t 
ngx_http_php_coroutine_resume(ngx_http_request_t *r)
{
	ngx_http_php_ctx_t *ctx;

	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

	if (ngx_php_coroutine_resume(ctx->coro) != 0) {
		return NGX_ERROR;
	}

	return NGX_OK;
}


