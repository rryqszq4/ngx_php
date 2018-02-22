/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_conf_file.h>
#include <ngx_event.h>
#include <nginx.h>

#include "php/php_ngx.h"
#include "php/php_ngx_core.h"
#include "php/php_ngx_generator.h"
#include "php/php_ngx_log.h"

#include "ngx_http_php_module.h"
#include "ngx_http_php_directive.h"
#include "ngx_http_php_handler.h"
#include "ngx_http_php_thread_handler.h"

// http init
static ngx_int_t ngx_http_php_init(ngx_conf_t *cf);
static ngx_int_t ngx_http_php_handler_init(ngx_http_core_main_conf_t *cmcf, ngx_http_php_main_conf_t *pmcf);

static void *ngx_http_php_create_main_conf(ngx_conf_t *cf);
static char *ngx_http_php_init_main_conf(ngx_conf_t *cf, void *conf);

static void *ngx_http_php_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_php_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child); 

// function init
static ngx_int_t ngx_http_php_init_worker(ngx_cycle_t *cycle);
static void ngx_http_php_exit_worker(ngx_cycle_t *cycle);

static ngx_command_t ngx_http_php_commands[] = {

	{ngx_string("php_ini_path"),
	 NGX_HTTP_MAIN_CONF|NGX_CONF_TAKE1,
	 ngx_http_php_ini_path,
	 NGX_HTTP_MAIN_CONF_OFFSET,
	 0,
	 NULL
	},

	{ngx_string("init_by_php"),
	 NGX_HTTP_MAIN_CONF|NGX_CONF_TAKE1,
	 ngx_http_php_init_inline_phase,
	 NGX_HTTP_MAIN_CONF_OFFSET,
	 0,
	 NULL
	},

	{ngx_string("init_by_php_file"),
	 NGX_HTTP_MAIN_CONF|NGX_CONF_TAKE1,
	 ngx_http_php_init_file_phase,
	 NGX_HTTP_MAIN_CONF_OFFSET,
	 0,
	 NULL
	},

	{ngx_string("rewrite_by_php_file"),
	 NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
	 	|NGX_CONF_TAKE1,
	 ngx_http_php_rewrite_phase,
	 NGX_HTTP_LOC_CONF_OFFSET,
	 0,
	 ngx_http_php_rewrite_file_handler
	},

	{ngx_string("rewrite_by_php"),
	 NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
	 	|NGX_CONF_TAKE1,
	 ngx_http_php_rewrite_inline_phase,
	 NGX_HTTP_LOC_CONF_OFFSET,
	 0,
	 ngx_http_php_rewrite_inline_handler
	},

	{ngx_string("access_by_php_file"),
	 NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
	 	|NGX_CONF_TAKE1,
	 ngx_http_php_access_phase,
	 NGX_HTTP_LOC_CONF_OFFSET,
	 0,
	 ngx_http_php_access_file_handler
	},

	{ngx_string("access_by_php"),
	 NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
	 	|NGX_CONF_TAKE1,
	 ngx_http_php_access_inline_phase,
	 NGX_HTTP_LOC_CONF_OFFSET,
	 0,
	 ngx_http_php_access_inline_handler
	},

	{ngx_string("content_by_php_file"),
	 NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
	 	|NGX_CONF_TAKE1,
	 ngx_http_php_content_phase,
	 NGX_HTTP_LOC_CONF_OFFSET,
	 0,
	 ngx_http_php_content_file_handler
	},

	{ngx_string("content_by_php"),
	 NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
	 	|NGX_CONF_TAKE1,
	 ngx_http_php_content_inline_phase,
	 NGX_HTTP_LOC_CONF_OFFSET,
	 0,
	 ngx_http_php_content_inline_handler
	},

	/*
	{ngx_string("content_async_by_php"),
	 NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
	 	|NGX_CONF_TAKE1,
	 ngx_http_php_content_async_inline_phase,
	 NGX_HTTP_LOC_CONF_OFFSET,
	 0,
	 ngx_http_php_content_async_inline_handler
	},

	{ngx_string("content_sync_by_php"),
	 NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
	 	|NGX_CONF_TAKE1,
	 ngx_http_php_content_inline_phase,
	 NGX_HTTP_LOC_CONF_OFFSET,
	 0,
	 ngx_http_php_content_sync_inline_handler
	},
	*/

	/*
	{ngx_string("thread_by_php_file"),
	 NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
	 	|NGX_CONF_TAKE1,
	 ngx_http_php_content_phase,
	 NGX_HTTP_LOC_CONF_OFFSET,
	 0,
	 ngx_http_php_content_file_thread_handler
	},

	{ngx_string("thread_by_php"),
	 NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
	 	|NGX_CONF_TAKE1,
	 ngx_http_php_content_inline_phase,
	 NGX_HTTP_LOC_CONF_OFFSET,
	 0,
	 ngx_http_php_content_inline_thread_handler
	},
	*/

	{ngx_string("log_by_php_file"),
	 NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
	 	|NGX_CONF_TAKE1,
	 ngx_http_php_log_phase,
	 NGX_HTTP_LOC_CONF_OFFSET,
	 0,
	 ngx_http_php_log_file_handler
	},

	{ngx_string("log_by_php"),
	 NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
	 	|NGX_CONF_TAKE1,
	 ngx_http_php_log_inline_phase,
	 NGX_HTTP_LOC_CONF_OFFSET,
	 0,
	 ngx_http_php_log_inline_handler
	},

#if defined(NDK) && NDK

	{ngx_string("set_by_php"),
	 NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
	 	|NGX_CONF_2MORE,
	 ngx_http_php_set_inline,
	 NGX_HTTP_LOC_CONF_OFFSET,
	 0,
	 ngx_http_php_set_inline_handler
	},

	{ngx_string("set_run_by_php"),
	 NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
	 	|NGX_CONF_2MORE,
	 ngx_http_php_set_run_inline,
	 NGX_HTTP_LOC_CONF_OFFSET,
	 0,
	 ngx_http_php_set_run_inline_handler
	},

	{ngx_string("set_by_php_file"),
	 NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
	 	|NGX_CONF_2MORE,
	 ngx_http_php_set_file,
	 NGX_HTTP_LOC_CONF_OFFSET,
	 0,
	 ngx_http_php_set_file_handler
	},

	{ngx_string("set_run_by_php_file"),
	 NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
	 	|NGX_CONF_2MORE,
	 ngx_http_php_set_run_file,
	 NGX_HTTP_LOC_CONF_OFFSET,
	 0,
	 ngx_http_php_set_run_file_handler
	},

#endif

	ngx_null_command
};

static ngx_http_module_t ngx_http_php_module_ctx = {
	NULL,                          /* preconfiguration */
	ngx_http_php_init,             /* postconfiguration */

	ngx_http_php_create_main_conf, /* create main configuration */
	ngx_http_php_init_main_conf,   /* init main configuration */

	NULL,                          /* create server configuration */
	NULL,                          /* merge server configuration */

	ngx_http_php_create_loc_conf,  /* create location configuration */
	ngx_http_php_merge_loc_conf    /* merge location configuration */

};


ngx_module_t ngx_http_php_module = {
	NGX_MODULE_V1,
	&ngx_http_php_module_ctx,    /* module context */
	ngx_http_php_commands,       /* module directives */
	NGX_HTTP_MODULE,               /* module type */
	NULL,                          /* init master */
	NULL,                          /* init module */
	ngx_http_php_init_worker,      /* init process */
	NULL,                          /* init thread */
	NULL,                          /* exit thread */
	ngx_http_php_exit_worker,      /* exit process */
	NULL,                          /* exit master */
	NGX_MODULE_V1_PADDING
};

static ngx_str_t  ngx_http_php_hide_headers[] = {
    ngx_string("Date"),
    ngx_string("Server"),
    ngx_string("X-Pad"),
    ngx_string("X-Accel-Expires"),
    ngx_string("X-Accel-Redirect"),
    ngx_string("X-Accel-Limit-Rate"),
    ngx_string("X-Accel-Buffering"),
    ngx_string("X-Accel-Charset"),
    ngx_null_string
};

static ngx_int_t 
ngx_http_php_init(ngx_conf_t *cf)
{
	ngx_http_core_main_conf_t *cmcf;
	ngx_http_php_main_conf_t *pmcf;

	cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);
	pmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_php_module);

	ngx_php_request = NULL;

	if (ngx_http_php_handler_init(cmcf, pmcf) != NGX_OK){
		return NGX_ERROR;
	}

	return NGX_OK;
}

static ngx_int_t 
ngx_http_php_handler_init(ngx_http_core_main_conf_t *cmcf, ngx_http_php_main_conf_t *pmcf)
{
	ngx_int_t i;
	ngx_http_handler_pt *h;
	ngx_http_phases phase;
	ngx_http_phases phases[] = {
		NGX_HTTP_POST_READ_PHASE,
		NGX_HTTP_REWRITE_PHASE,
		NGX_HTTP_ACCESS_PHASE,
		NGX_HTTP_CONTENT_PHASE,
		NGX_HTTP_LOG_PHASE,
	};
	ngx_int_t phases_c;

	phases_c = sizeof(phases) / sizeof(ngx_http_phases);
	for (i = 0; i < phases_c; i++){
		phase = phases[i];
		switch (phase){
			case NGX_HTTP_POST_READ_PHASE:
				h = ngx_array_push(&cmcf->phases[phase].handlers);
				if (h == NULL){
					return NGX_ERROR;
				}
				*h = ngx_http_php_post_read_handler;
				break;
			case NGX_HTTP_REWRITE_PHASE:
				if (pmcf->enabled_rewrite_handler){
					h = ngx_array_push(&cmcf->phases[phase].handlers);
					if (h == NULL){
						return NGX_ERROR;
					}
					*h = ngx_http_php_rewrite_handler;
				}
				break;
			case NGX_HTTP_ACCESS_PHASE:
				if (pmcf->enabled_access_handler){
					h = ngx_array_push(&cmcf->phases[phase].handlers);
					if (h == NULL){
						return NGX_ERROR;
					}
					*h = ngx_http_php_access_handler;
				}
				break;
			case NGX_HTTP_CONTENT_PHASE:
				if (pmcf->enabled_content_handler){
					h = ngx_array_push(&cmcf->phases[phase].handlers);
					if (h == NULL){
						return NGX_ERROR;
					}
					*h = ngx_http_php_content_handler;
				}
				/*if (pmcf->enabled_content_async_handler){
					h = ngx_array_push(&cmcf->phases[phase].handlers);
					if (h == NULL){
						return NGX_ERROR;
					}
					*h = ngx_http_php_content_async_handler;
				}*/
				break;
			case NGX_HTTP_LOG_PHASE:
				if (pmcf->enabled_log_handler) {
					h = ngx_array_push(&cmcf->phases[phase].handlers);
					if (h == NULL) {
						return NGX_ERROR;
					}
					*h = ngx_http_php_log_handler;
				}
				break;
			default:
				break;
		}
	}

	return NGX_OK;
}

static void *
ngx_http_php_create_main_conf(ngx_conf_t *cf)
{
	ngx_http_php_main_conf_t *pmcf;

	pmcf = ngx_pcalloc(cf->pool, sizeof(ngx_http_php_main_conf_t));
	if (pmcf == NULL){
		return NULL;
	}

	//-> alloc array thread_pools
	if (ngx_array_init(&pmcf->thread_pools, cf->pool, 4, sizeof(ngx_php_thread_pool_t *)) != NGX_OK) {
		return NULL;
	}

	pmcf->state = ngx_pcalloc(cf->pool, sizeof(ngx_http_php_state_t));
	if (pmcf->state == NULL){
		return NULL;
	}

	pmcf->state->php_init = 0;
	pmcf->state->php_shutdown = 0;

	pmcf->ini_path.len = 0;
	pmcf->init_code = NGX_CONF_UNSET_PTR;
	pmcf->init_inline_code = NGX_CONF_UNSET_PTR;

	return pmcf;
}

static char *
ngx_http_php_init_main_conf(ngx_conf_t *cf, void *conf)
{
	/*ngx_http_php_main_conf_t *pmcf = conf;

	//ngx_uint_t i;
	ngx_php_thread_pool_t  *tp, **tpp;

	//tpp = pmcf->thread_pools.elts;

	tp = ngx_pcalloc(cf->pool, sizeof(ngx_php_thread_pool_t));
	if (tp == NULL) {
		return NULL;
	}

	tp->name = (ngx_str_t) ngx_string("ngx_php tp");
	tp->threads = 32;
	tp->max_queue = 65536;

	tpp = ngx_array_push(&pmcf->thread_pools);
	if (tpp == NULL) {
		return NULL;
	}

	*tpp = tp;*/

	//ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "%d", pmcf->thread_pools.nelts);

	/*for (i = 0; i < pmcf->thread_pools.nelts; i++) {
		if (tpp[i]->threads) {
			continue;
		}

		tpp[i]->threads = 32;
		tpp[i]->max_queue = 65536;
	}*/

	return NGX_CONF_OK;
}

static void *
ngx_http_php_create_loc_conf(ngx_conf_t *cf)
{
	ngx_http_php_loc_conf_t *plcf;

	plcf = ngx_pcalloc(cf->pool, sizeof(ngx_http_php_loc_conf_t));
	if (plcf == NULL){
		return NGX_CONF_ERROR;
	}

	plcf->document_root.len = 0;

	plcf->rewrite_code = NGX_CONF_UNSET_PTR;
	plcf->rewrite_inline_code = NGX_CONF_UNSET_PTR;

	plcf->access_code = NGX_CONF_UNSET_PTR;
	plcf->access_inline_code = NGX_CONF_UNSET_PTR;

	plcf->content_code = NGX_CONF_UNSET_PTR;
	plcf->content_inline_code = NGX_CONF_UNSET_PTR;

	plcf->content_async_inline_code = NGX_CONF_UNSET_PTR;

	plcf->log_code = NGX_CONF_UNSET_PTR;
	plcf->log_inline_code = NGX_CONF_UNSET_PTR;

	plcf->upstream.connect_timeout = 60000;
    plcf->upstream.send_timeout = 60000;
    plcf->upstream.read_timeout = 60000;
    plcf->upstream.store_access = 0600;

    plcf->upstream.buffering = 1;
    plcf->upstream.bufs.num = 8;
    plcf->upstream.bufs.size = ngx_pagesize;
    plcf->upstream.buffer_size = ngx_pagesize;
    plcf->upstream.busy_buffers_size = 2 * ngx_pagesize;
    plcf->upstream.temp_file_write_size = 2 * ngx_pagesize;
    plcf->upstream.max_temp_file_size = 1024 * 1024 * 1024;

    plcf->upstream.hide_headers = NGX_CONF_UNSET_PTR;
    plcf->upstream.pass_headers = NGX_CONF_UNSET_PTR;

    //plcf->upstream.ignore_client_abort = 1;

    //-> feature: upstream connection pool
    /*plcf->upstream.upstream = ngx_pcalloc(cf->pool,
                       sizeof(ngx_http_upstream_srv_conf_t));

    if (plcf->upstream.upstream == NULL) {
    	return NGX_CONF_ERROR;
    }*/

	return plcf;
}

static char *
ngx_http_php_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
	ngx_http_core_loc_conf_t  *clcf;
	clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);

	ngx_http_php_loc_conf_t *prev = parent;
	ngx_http_php_loc_conf_t *conf = child;

	conf->document_root.len = clcf->root.len;
	conf->document_root.data = clcf->root.data;

	prev->rewrite_code = conf->rewrite_code;
	prev->rewrite_inline_code = conf->rewrite_inline_code;

	prev->access_code = conf->access_code;
	prev->access_inline_code = conf->access_inline_code;

	prev->content_code = conf->content_code;
	prev->content_inline_code = conf->content_inline_code;

	prev->content_async_inline_code = conf->content_async_inline_code;

	prev->log_code = conf->log_code;
	prev->log_inline_code = conf->log_inline_code;

	ngx_hash_init_t		hash;
	hash.max_size = 512;
	hash.bucket_size = 1024;
	hash.name = "ngx_php_headers_hash";

	if (ngx_http_upstream_hide_headers_hash(cf, &conf->upstream,
            &prev->upstream, ngx_http_php_hide_headers, &hash)
        != NGX_OK)
    {
        return NGX_CONF_ERROR;
    }

	return NGX_CONF_OK;
}

static ngx_int_t 
ngx_http_php_init_worker(ngx_cycle_t *cycle)
{
	//TSRMLS_FETCH();
	/*ngx_uint_t					i;
	ngx_php_thread_pool_t 		**tpp;*/
	ngx_http_php_main_conf_t 	*pmcf;

	pmcf = ngx_http_cycle_get_module_main_conf(cycle, ngx_http_php_module);

	//-> init run thread_pool
	ngx_php_thread_pool_queue_init(&ngx_php_thread_pool_done);
	ngx_php_thread_pool_queue_init(&ngx_php_thread_pool_running);

	//ngx_log_error(NGX_LOG_ERR, cycle->log, 0, "thread pool done %p", &ngx_php_thread_pool_done);
	//ngx_log_error(NGX_LOG_ERR, cycle->log, 0, "thread pool running %p", &ngx_php_thread_pool_running);

	/*tpp = pmcf->thread_pools.elts;

	for (i = 0; i < pmcf->thread_pools.nelts; i++) {
		if (ngx_php_thread_pool_init(tpp[i], cycle->log, cycle->pool) != NGX_OK) {
			return NGX_ERROR;
		}
	}*/

	//-> init run php
	php_ngx_module.ub_write = ngx_http_php_code_ub_write;
	php_ngx_module.flush = ngx_http_php_code_flush;
	//php_ngx_module.log_message = ngx_http_php_code_log_message;
	//php_ngx_module.register_server_variables = ngx_http_php_code_register_server_variables;
	//php_ngx_module.read_post = ngx_http_php_code_read_post;
	//php_ngx_module.read_cookies = ngx_http_php_code_read_cookies;
	//php_ngx_module.header_handler = ngx_http_php_code_header_handler;

	if (pmcf->ini_path.len != 0){
		php_ngx_module.php_ini_path_override = (char *)pmcf->ini_path.data;
	}
	
	php_ngx_module_init();

	zend_startup_module(&php_ngx_module_entry);

	old_zend_error_cb = zend_error_cb;
	zend_error_cb = ngx_php_error_cb;

	TSRMLS_FETCH();
	php_ngx_request_init(TSRMLS_C);
	
	php_ngx_core_init(0 TSRMLS_CC);
	php_ngx_generator_init(0 TSRMLS_CC);
	php_ngx_log_init(0 TSRMLS_CC);

	return NGX_OK;
}

static void 
ngx_http_php_exit_worker(ngx_cycle_t *cycle)
{
	TSRMLS_FETCH();

	ngx_uint_t 					i;
	ngx_php_thread_pool_t 		**tpp;
	ngx_http_php_main_conf_t 	*pmcf;

	pmcf = ngx_http_cycle_get_module_main_conf(cycle, ngx_http_php_module);

	php_ngx_request_shutdown(TSRMLS_C);
	php_ngx_module_shutdown(TSRMLS_C);

	tpp = pmcf->thread_pools.elts;

	for (i = 0; i < pmcf->thread_pools.nelts; i++) {
		ngx_php_thread_pool_destroy(tpp[i]);
	}
}


















