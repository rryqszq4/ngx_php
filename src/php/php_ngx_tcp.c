/**
 *    Copyright(c) 2016-2017 rryqszq4
 *
 *
 */

#include "php_ngx_tcp.h"
#include "../ngx_http_php_module.h"
#include "../ngx_http_php_upstream.h"
#include "../ngx_http_php_socket_tcp.h"

static zend_class_entry *php_ngx_tcp_class_entry;

ZEND_BEGIN_ARG_INFO_EX(ngx_tcp_construct_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ngx_tcp_connect_arginfo, 0, 0, 3)
    ZEND_ARG_INFO(0, host)
    ZEND_ARG_INFO(0, port)
    ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ngx_tcp_send_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, buf)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ngx_tcp_receive_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, size)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ngx_tcp_close_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ngx_tcp_settimeout_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, time)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ngx_tcp_setkeepalive_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, size)
ZEND_END_ARG_INFO()

PHP_METHOD(ngx_tcp, __construct)
{

}

PHP_METHOD(ngx_tcp, connect)
{
    char *host_str;
    int host_len;
    long port;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl", &host_str, &host_len, &port) == FAILURE){
        RETURN_NULL();
    }

    ngx_http_request_t *r = PHP_NGX_G(global_r);

    ngx_http_php_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (ctx == NULL){
        
    }

    ctx->host.data = ngx_palloc(r->pool, host_len + 1);

    ctx->host.len = host_len;

    ngx_memcpy(ctx->host.data, (u_char *)host_str, host_len + 1);
    ctx->host.data[host_len] = '\0';

    ctx->port = port;

}

PHP_METHOD(ngx_tcp, send)
{
    char *buf_str;
    int buf_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &buf_str, &buf_len) == FAILURE){
        RETURN_NULL();
    }

    ngx_http_request_t *r = PHP_NGX_G(global_r);

    ngx_http_php_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (ctx == NULL){
        
    }

    ngx_str_t ns;
    ns.data = (u_char *)buf_str;
    ns.len = buf_len;

    ctx->send_buf.len = buf_len;
    ctx->send_buf.data = ngx_pstrdup(r->pool, &ns);


    ctx->read_or_write = 0;

    ngx_http_php_socket_tcp_run(r);
}

PHP_METHOD(ngx_tcp, receive)
{

    ngx_http_request_t *r = PHP_NGX_G(global_r);

    ngx_http_php_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (ctx == NULL){
        
    }

    ctx->read_or_write = 1;
    ctx->enable_upstream = 1;

    if (ctx->receive_stat == 0){
        RETVAL_STRINGL((char *)ctx->receive_buf.data, ctx->receive_buf.len, 1);
    } else {

        char *receive_str = emalloc(ctx->receive_total + 1);
        int tmp_mark = 0;

        ngx_list_part_t *part = &(ctx->receive_list)->part;
        ngx_str_t *str = part->elts;
        //ngx_str_t *header;
        ngx_uint_t i;
        for (i = 0; /* void */; i++) {
            if (i >= part->nelts){
                if ( NULL == part->next){
                    break;
                }
                part = part->next;
                //header = part->elts;
                i = 0;
            }

            if (i == 0){
                memcpy(receive_str, str[i].data, str[i].len);
            }else {
                memcpy(receive_str + tmp_mark, str[i].data, str[i].len);
            }

            tmp_mark += str[i].len;

        }
        //receive_str[ctx->receive_total] = '\0';
        
        RETVAL_STRINGL((char *)receive_str, ctx->receive_total, 1);
        efree(receive_str);
    }

    ctx->receive_stat = 0;
    ctx->receive_total = 0;

    return ;
}

PHP_METHOD(ngx_tcp, close)
{
    ngx_http_request_t *r = PHP_NGX_G(global_r);

    ngx_http_php_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (ctx == NULL){
        
    }

    ctx->enable_upstream_continue = 0;

    ctx->send_buf.len = 0;
    ctx->send_buf.data = NULL;
}

PHP_METHOD(ngx_tcp, settimeout)
{
    long time;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &time) == FAILURE){
        RETURN_NULL();
    }

    ngx_http_request_t *r = PHP_NGX_G(global_r);

    ngx_http_php_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (ctx == NULL){
        
    }

    ctx->timeout = time;
}

PHP_METHOD(ngx_tcp, setkeepalive)
{
    long size;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &size) == FAILURE){
        RETURN_NULL();
    }

    ngx_http_request_t *r = PHP_NGX_G(global_r); 

    ngx_http_php_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);
    
    if (ctx == NULL){
        
    }
}

static const zend_function_entry php_ngx_tcp_class_functions[] = {
    PHP_ME(ngx_tcp, __construct, ngx_tcp_construct_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(ngx_tcp, connect, ngx_tcp_connect_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(ngx_tcp, send, ngx_tcp_send_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(ngx_tcp, receive, ngx_tcp_receive_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(ngx_tcp, close, ngx_tcp_close_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(ngx_tcp, settimeout, ngx_tcp_settimeout_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(ngx_tcp, setkeepalive, ngx_tcp_setkeepalive_arginfo, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL, 0, 0}
};

void 
php_ngx_tcp_init(int module_number TSRMLS_DC)
{
    zend_class_entry ngx_tcp_class_entry;
    INIT_CLASS_ENTRY(ngx_tcp_class_entry, "ngx_tcp", php_ngx_tcp_class_functions);
    php_ngx_tcp_class_entry = zend_register_internal_class(&ngx_tcp_class_entry TSRMLS_CC);
}


