/**
 *    Copyright(c) 2016-2017 rryqszq4
 *
 *
 */

#include "php_ngx_location.h"
#include "../ngx_http_php_module.h"
#include "../ngx_http_php_request.h"
#include "../ngx_http_php_subrequest.h"

static zend_class_entry *php_ngx_location_class_entry;

ZEND_BEGIN_ARG_INFO_EX(arginfo_ngx_location_construct, 0, 0, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_ngx_location_capture_async, 0, 0, 2)
    ZEND_ARG_INFO(0, uri)
    ZEND_ARG_INFO(0, closure)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_ngx_location_capture_multi_async, 0, 0, 2)
    ZEND_ARG_INFO(0, uri_arr)
    ZEND_ARG_INFO(0, closure)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_ngx_location_capture, 0, 0, 1)
    ZEND_ARG_INFO(0, uri)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_ngx_location_capture_multi, 0, 0, 1)
    ZEND_ARG_INFO(0, uri_arr)
ZEND_END_ARG_INFO()


static int _php_count_recursive(zval *array, long mode TSRMLS_DC) /* {{{ */
{
  long cnt = 0;
  zval **element;

  if (Z_TYPE_P(array) == IS_ARRAY) {
    if (Z_ARRVAL_P(array)->nApplyCount > 1) {
      php_error_docref(NULL TSRMLS_CC, E_WARNING, "recursion detected");
      return 0;
    }

    cnt = zend_hash_num_elements(Z_ARRVAL_P(array));
    if (mode == COUNT_RECURSIVE) {
      HashPosition pos;

      for (zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(array), &pos);
        zend_hash_get_current_data_ex(Z_ARRVAL_P(array), (void **) &element, &pos) == SUCCESS;
        zend_hash_move_forward_ex(Z_ARRVAL_P(array), &pos)
      ) {
        Z_ARRVAL_P(array)->nApplyCount++;
        cnt += _php_count_recursive(*element, COUNT_RECURSIVE TSRMLS_CC);
        Z_ARRVAL_P(array)->nApplyCount--;
      }
    }
  }

  return cnt;
}


PHP_METHOD(ngx_location, capture_async)
{
	char *uri_str, *name_str, *lcname, *tmp;
	int uri_len, name_len, tmp_len;

	zval *closure = NULL;
	zval *classname;

	zend_class_entry **pce;
	zend_class_entry *ce;
	zend_function *fptr;

	zval *val;

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "sO", &uri_str, &uri_len, &closure, zend_ce_closure) == SUCCESS) {
    	fptr = (zend_function*)zend_get_closure_method_def(closure TSRMLS_CC);
    	Z_ADDREF_P(closure);
  	} else if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &uri_str, &uri_len, &name_str, &name_len) == SUCCESS) {
		if ((tmp = strstr(name_str, "::")) == NULL) {
			char *nsname;

			lcname = zend_str_tolower_dup(name_str, name_len);

			// Ignore leading "\" 
			nsname = lcname;
			if (lcname[0] == '\\') {
			nsname = &lcname[1];
			name_len--;
			}

			if (zend_hash_find(EG(function_table), nsname, name_len + 1, (void **)&fptr) == FAILURE) {
				efree(lcname);
				php_error_docref(NULL TSRMLS_CC, E_WARNING, 
		    	"Function %s() does not exist", name_str);
		   		return;
			}
			efree(lcname);
		}else {
			tmp_len = tmp - name_str;
			MAKE_STD_ZVAL(classname);
			ZVAL_STRINGL(classname, name_str, tmp_len, 1);
			name_len = name_len - (tmp_len + 2);
			name_str = tmp + 2;
			//php_printf("classname: %s, method: %s\n", Z_STRVAL_P(classname), name_str);
			if (zend_lookup_class(Z_STRVAL_P(classname), Z_STRLEN_P(classname), &pce TSRMLS_CC) == FAILURE) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Class %s does exist", Z_STRVAL_P(classname));
				//zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC,
				//    "Class %s does not exist", Z_STRVAL_P(classname)); 
				zval_dtor(classname);
				return;
			}
			ce = *pce;

			lcname = zend_str_tolower_dup(name_str, name_len);

			if (zend_hash_find(&ce->function_table, lcname, name_len + 1, (void **) &fptr) == FAILURE) {
				efree(lcname);
				php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Method %s::%s() does not exist", ce->name, name_str);
				//zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, 
				//  "Method %s::%s() does not exist", ce->name, name_str);
				return;
			}
		  	efree(lcname);
		}
	}else {
    	return ;
	}

	MAKE_STD_ZVAL(val);

#if PHP_VERSION_ID < 50399
	zend_create_closure(val, fptr TSRMLS_CC);
#else
	zend_create_closure(val, fptr, NULL, NULL TSRMLS_CC);
#endif


	//ngx_http_php_request_context_t *context = (ngx_http_php_request_context_t *)SG(server_context);
	//ngx_http_request_t *r = (ngx_http_request_t *)context->r;

    ngx_http_request_t *r = PHP_NGX_G(global_r);

	ngx_http_php_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

	if (ctx == NULL){
		
	}

	ngx_str_t ns;
	ns.data = (u_char *)uri_str;
	ns.len = uri_len;

	//ctx->capture_uri.data = (u_char *)uri_str;
	ctx->capture_uri.len = uri_len;

	ctx->capture_uri.data = ngx_pstrdup(r->pool, &ns);

	ctx->closure = val;

	ctx->enable_async = 1;

	ctx->is_capture_multi = 0;

	//ngx_http_set_ctx(r, ctx, ngx_http_php_module);

	//ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "ngx_location %s", (u_char *)uri_str);
	//ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "ngx_location %d", ctx->capture_uri.len);

	//ngx_http_php_subrequest_post(r);

	pthread_mutex_lock(&(ctx->mutex));
	pthread_cond_wait(&(ctx->cond), &(ctx->mutex));
	pthread_mutex_unlock(&(ctx->mutex));

	return ;
}

PHP_METHOD(ngx_location, capture_multi_async)
{
	zval *uri_arr;

	char *name_str, *lcname, *tmp;
	int name_len, tmp_len;

	zval *closure = NULL;
	zval *classname;

	zend_class_entry **pce;
	zend_class_entry *ce;
	zend_function *fptr;

	zval *val;

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "aO", &uri_arr, &closure, zend_ce_closure) == SUCCESS) {
    	fptr = (zend_function*)zend_get_closure_method_def(closure TSRMLS_CC);
    	Z_ADDREF_P(closure);
  	} else if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "as", &uri_arr, &name_str, &name_len) == SUCCESS) {
		if ((tmp = strstr(name_str, "::")) == NULL) {
			char *nsname;

			lcname = zend_str_tolower_dup(name_str, name_len);

			// Ignore leading "\" 
			nsname = lcname;
			if (lcname[0] == '\\') {
			nsname = &lcname[1];
			name_len--;
			}

			if (zend_hash_find(EG(function_table), nsname, name_len + 1, (void **)&fptr) == FAILURE) {
				efree(lcname);
				php_error_docref(NULL TSRMLS_CC, E_WARNING, 
		    	"Function %s() does not exist", name_str);
		   		return;
			}
			efree(lcname);
		}else {
			tmp_len = tmp - name_str;
			MAKE_STD_ZVAL(classname);
			ZVAL_STRINGL(classname, name_str, tmp_len, 1);
			name_len = name_len - (tmp_len + 2);
			name_str = tmp + 2;
			//php_printf("classname: %s, method: %s\n", Z_STRVAL_P(classname), name_str);
			if (zend_lookup_class(Z_STRVAL_P(classname), Z_STRLEN_P(classname), &pce TSRMLS_CC) == FAILURE) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Class %s does exist", Z_STRVAL_P(classname));
				//zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC,
				//    "Class %s does not exist", Z_STRVAL_P(classname)); 
				zval_dtor(classname);
				return;
			}
			ce = *pce;

			lcname = zend_str_tolower_dup(name_str, name_len);

			if (zend_hash_find(&ce->function_table, lcname, name_len + 1, (void **) &fptr) == FAILURE) {
				efree(lcname);
				php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Method %s::%s() does not exist", ce->name, name_str);
				//zend_throw_exception_ex(reflection_exception_ptr, 0 TSRMLS_CC, 
				//  "Method %s::%s() does not exist", ce->name, name_str);
				return;
			}
		  	efree(lcname);
		}
	}else {
    	return ;
	}

	MAKE_STD_ZVAL(val);

#if PHP_VERSION_ID < 50399
	zend_create_closure(val, fptr TSRMLS_CC);
#else
	zend_create_closure(val, fptr, NULL, NULL TSRMLS_CC);
#endif

	//ngx_http_php_request_context_t *context = (ngx_http_php_request_context_t *)SG(server_context);
	//ngx_http_request_t *r = (ngx_http_request_t *)context->r;

    ngx_http_request_t *r = PHP_NGX_G(global_r);

	ngx_http_php_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

	if (ctx == NULL){
		
	}

	int count_uri = 0;
	count_uri = _php_count_recursive(uri_arr, 0 TSRMLS_CC);
	ctx->capture_multi = ngx_array_create(r->pool, (ngx_uint_t)count_uri, sizeof(ngx_http_php_capture_node_t));

	zval **current;
 	ulong hash_index = 0;
 	char *hash_key = NULL;

  	for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(uri_arr));
    	zend_hash_get_current_data(Z_ARRVAL_P(uri_arr), (void **) &current) == SUCCESS;
    	zend_hash_move_forward(Z_ARRVAL_P(uri_arr))
 	){
    	zend_hash_get_current_key(Z_ARRVAL_P(uri_arr), &hash_key, &hash_index, 0);
    	SEPARATE_ZVAL(current);
    	//php_printf("key: %s, index: %d", hash_key, hash_index);
    	if (Z_TYPE_PP(current) == IS_STRING){
    		ngx_str_t ns;
			ns.data = (u_char *)Z_STRVAL_PP(current);
			ns.len = Z_STRLEN_PP(current);
    		ngx_http_php_capture_node_t *tmp_node = ngx_array_push(ctx->capture_multi);
    		tmp_node->capture_uri.len = ns.len;
    		tmp_node->capture_uri.data = ngx_pstrdup(r->pool, &ns);
    		
    		//ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "ngx_location : => %s [%d]", tmp_node->capture_uri.data,tmp_node->capture_uri.len);
    	}

	}

	//ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "count : ----[%d]", count_uri);

	ctx->closure = val;
	ctx->enable_async = 1;
	ctx->is_capture_multi = 1;

	//ngx_http_set_ctx(r, ctx, ngx_http_php_module);


	pthread_mutex_lock(&(ctx->mutex));
	pthread_cond_wait(&(ctx->cond), &(ctx->mutex));
	pthread_mutex_unlock(&(ctx->mutex));

	return ;
}

PHP_METHOD(ngx_location, capture)
{
    char *uri_str;
    int uri_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &uri_str, &uri_len) == FAILURE){
        return ;
    }

    //ngx_http_php_request_context_t *context = (ngx_http_php_request_context_t *)SG(server_context);
    //ngx_http_request_t *r = (ngx_http_request_t *)context->r;
    
    //ngx_http_php_request_context_t *context = (ngx_http_php_request_context_t *) PHP_NGX_G(request_context);
    ngx_http_request_t *r = PHP_NGX_G(global_r);

    ngx_http_php_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (ctx == NULL){
        
    }

    ngx_str_t ns;
    ns.data = (u_char *)uri_str;
    ns.len = uri_len;

    //ctx->capture_uri.data = (u_char *)uri_str;
    ctx->capture_uri.len = uri_len;

    ctx->capture_uri.data = ngx_pstrdup(r->pool, &ns);

    ctx->enable_async = 1;

    ctx->is_capture_multi = 0;

    //ngx_http_set_ctx(r, ctx, ngx_http_php_module);

    pthread_mutex_lock(&(ctx->mutex));
    pthread_cond_wait(&(ctx->cond), &(ctx->mutex));
    pthread_mutex_unlock(&(ctx->mutex));

    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    RETVAL_STRINGL((char *)ctx->capture_str.data, ctx->capture_str.len, 1);

    return ;
}

PHP_METHOD(ngx_location, capture_multi)
{
    zval *uri_arr;
    zval *result;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &uri_arr) == FAILURE){
        return ;
    }

    //ngx_http_php_request_context_t *context = (ngx_http_php_request_context_t *)SG(server_context);
    //ngx_http_request_t *r = (ngx_http_request_t *)context->r;

    ngx_http_request_t *r = PHP_NGX_G(global_r);

    ngx_http_php_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (ctx == NULL){
        
    }

    int count_uri = 0;
    count_uri = _php_count_recursive(uri_arr, 0 TSRMLS_CC);
    ctx->capture_multi = ngx_array_create(r->pool, (ngx_uint_t)count_uri, sizeof(ngx_http_php_capture_node_t));

    zval **current;
    ulong hash_index = 0;
    char *hash_key = NULL;

    for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(uri_arr));
        zend_hash_get_current_data(Z_ARRVAL_P(uri_arr), (void **) &current) == SUCCESS;
        zend_hash_move_forward(Z_ARRVAL_P(uri_arr))
    ){
        zend_hash_get_current_key(Z_ARRVAL_P(uri_arr), &hash_key, &hash_index, 0);
        SEPARATE_ZVAL(current);
        //php_printf("key: %s, index: %d", hash_key, hash_index);
        if (Z_TYPE_PP(current) == IS_STRING){
            ngx_str_t ns;
            ns.data = (u_char *)Z_STRVAL_PP(current);
            ns.len = Z_STRLEN_PP(current);
            ngx_http_php_capture_node_t *tmp_node = ngx_array_push(ctx->capture_multi);
            tmp_node->capture_uri.len = ns.len;
            tmp_node->capture_uri.data = ngx_pstrdup(r->pool, &ns);
            
            //ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "ngx_location : => %s [%d]", tmp_node->capture_uri.data,tmp_node->capture_uri.len);
        }

    }

    //ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "count : ----[%d]", count_uri);

    ctx->enable_async = 1;
    ctx->is_capture_multi = 1;

    //ngx_http_set_ctx(r, ctx, ngx_http_php_module);


    pthread_mutex_lock(&(ctx->mutex));
    pthread_cond_wait(&(ctx->cond), &(ctx->mutex));
    pthread_mutex_unlock(&(ctx->mutex));

    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);


    MAKE_STD_ZVAL(result);
    array_init(result);

    ngx_http_php_capture_node_t *capture_node = ctx->capture_multi->elts;
    ngx_uint_t i;
    for (i = 0; i < ctx->capture_multi->nelts; i++,capture_node++){
        add_next_index_stringl(result, (char *)capture_node->capture_str.data, capture_node->capture_str.len, 1);
    }

    RETVAL_ZVAL(result, 1, 0);

    return ;
}

static const zend_function_entry php_ngx_location_class_functions[] = {
	//PHP_ME(ngx_location, capture_async, arginfo_ngx_location_capture_async, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	//PHP_ME(ngx_location, capture_multi_async, arginfo_ngx_location_capture_multi_async, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ngx_location, capture, arginfo_ngx_location_capture, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ngx_location, capture_multi, arginfo_ngx_location_capture_multi, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL, 0, 0}
};

void 
ngx_location_init(int module_number TSRMLS_DC)
{
	zend_class_entry ngx_location_class_entry;
	INIT_CLASS_ENTRY(ngx_location_class_entry, "ngx_location", php_ngx_location_class_functions);
	php_ngx_location_class_entry = zend_register_internal_class(&ngx_location_class_entry TSRMLS_CC);
}