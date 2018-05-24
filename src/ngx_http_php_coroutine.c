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
	ngx_php_request = r;

	ngx_http_php_ctx_t *ctx;

	ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);



	ngx_php_debug("not yield; opline: %p, %s, %p", EG(opline_ptr),zend_get_opcode_name((*EG(opline_ptr))->opcode), *EG(opline_ptr));
	zend_execute_data *current_execute_data;
	current_execute_data = EG(current_execute_data);
	ctx->opline = current_execute_data->opline;
    zend_vm_stack current_stack = EG(argument_stack);
    //ctx->op_array = (zend_op_array*)emalloc(sizeof(zend_op_array));
    ctx->op_array = EG(active_op_array);
    ngx_php_debug("%p\n", ctx->op_array);
    ctx->return_value_ptr_ptr = EG(return_value_ptr_ptr);

    ctx->op_array->fn_flags |= ZEND_ACC_GENERATOR;
    ctx->execute_data = zend_create_execute_data_from_op_array(ctx->op_array, 0 TSRMLS_CC);
    EG(current_execute_data) = current_execute_data;
    *(EG(argument_stack)->top-1) = NULL;
    ctx->argument_stack = EG(argument_stack);
    EG(argument_stack) = current_stack;

    ctx->execute_data->current_scope = EG(scope);
	ctx->execute_data->current_called_scope = EG(called_scope);
	ctx->execute_data->symbol_table = EG(active_symbol_table);
	ctx->execute_data->current_this = EG(This);

	ctx->execute_data->call = current_execute_data->call;

    ctx->op_array->fn_flags &= ~ZEND_ACC_GENERATOR;

	if (ngx_php_coroutine_yield(ctx->coro) != 0) {
		return NGX_ERROR;
	}

	r = ngx_php_request;

    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    ctx->execute_data->opline = ctx->opline;
    ctx->execute_data->opline++;
    EG(current_execute_data) = ctx->execute_data;
    EG(argument_stack) = ctx->argument_stack;
    EG(opline_ptr) = &(ctx->execute_data->opline);
    EG(return_value_ptr_ptr) = ctx->return_value_ptr_ptr;
    EG(active_op_array) = ctx->op_array;

    EG(active_symbol_table) = ctx->execute_data->symbol_table;
	EG(This) = ctx->execute_data->current_this;
	EG(scope) = ctx->execute_data->current_scope;
	EG(called_scope) = ctx->execute_data->current_called_scope;

    ngx_php_debug("yield; opline: %p, %s, %p, %p", EG(opline_ptr),zend_get_opcode_name((*EG(opline_ptr))->opcode), *EG(opline_ptr),
    	&ctx->execute_data->op_array->opcodes[(int)ctx->execute_data->op_array->last - 1]);

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


