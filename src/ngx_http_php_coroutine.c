/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */

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
		coro = ngx_palloc(r->pool, sizeof(ngx_php_coroutine_t));
		if (coro == NULL) {
			return NULL;
		}
	}

	coro->stack = ngx_pcalloc(r->pool, PHP_COROUTINE_STACK_SIZE);

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

	if (ngx_php_coroutine_yield(ctx->coro) != 0) {
		return NGX_ERROR;
	}

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


