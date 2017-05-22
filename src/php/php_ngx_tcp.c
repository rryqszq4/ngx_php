/**
 *    Copyright(c) 2016-2017 rryqszq4
 *
 *
 */

#include "php_ngx_tcp.h"
#include "../ngx_http_php_module.h"
#include "../ngx_http_php_upstream.h"

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

}

PHP_METHOD(ngx_tcp, send)
{

}

PHP_METHOD(ngx_tcp, receive)
{

}

PHP_METHOD(ngx_tcp, close)
{

}

PHP_METHOD(ngx_tcp, settimeout)
{

}

PHP_METHOD(ngx_tcp, setkeepalive)
{

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
ngx_tcp_init(int module_number TSRMLS_DC)
{
    zend_class_entry ngx_tcp_class_entry;
    INIT_CLASS_ENTRY(ngx_tcp_class_entry, "ngx_tcp", php_ngx_tcp_class_functions);
    php_ngx_tcp_class_entry = zend_register_internal_class(&ngx_tcp_class_entry TSRMLS_CC);
}


