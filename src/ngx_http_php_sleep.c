/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */


#include "ngx_php_debug.h"
#include "ngx_http_php_sleep.h"
#include "ngx_http_php_coroutine.h"
#include "ngx_http_php_zend_uthread.h"

static void ngx_http_php_sleep_cleanup(void *data);

static void ngx_http_php_sleep_handler(ngx_event_t *ev);

static void ngx_http_php_cosleep_cleanup(void *data);

static void ngx_http_php_cosleep_handler(ngx_event_t *ev);

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

static void 
ngx_http_php_cosleep_cleanup(void *data)
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

/*static zend_vm_stack ngx_zend_vm_stack_new_page(int count) {
    zend_vm_stack page = (zend_vm_stack)emalloc(ZEND_MM_ALIGNED_SIZE(sizeof(*page)) + sizeof(void*) * count);

    page->top = ZEND_VM_STACK_ELEMETS(page);
    page->end = page->top + count;
    page->prev = NULL;
    return page;
}

static void** ngx_zend_vm_stack_frame_base(zend_execute_data *ex)
{
    return (void**)((char*)ex->call_slots +
        ZEND_MM_ALIGNED_SIZE(sizeof(call_slot)) * ex->op_array->nested_calls);
}*/

/*static void ngx_zend_vm_stack_destroy(TSRMLS_D)
{
    zend_vm_stack stack = EG(argument_stack);

    while (stack != NULL) {
        zend_vm_stack p = stack->prev;
        efree(stack);
        stack = p;
    }
}*/

ngx_int_t 
ngx_http_php_cosleep(ngx_http_request_t *r)
{
    ngx_http_cleanup_t *cln;
    ngx_http_php_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (ctx == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    
    ctx->phase_status = NGX_AGAIN;

    //ngx_memzero(&ctx->sleep, sizeof(ngx_event_t));

    ctx->sleep.handler = ngx_http_php_cosleep_handler;
    ctx->sleep.log = r->connection->log;
    ctx->sleep.data = r;

    //ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,"%p %p %d", r, &ctx->sleep, ctx->delay_time);

    ngx_php_debug("r:%p, &ctx->sleep:%p, ctx->delay_time:%d", r, &ctx->sleep, (int)ctx->delay_time);

    ngx_add_timer(&ctx->sleep, (ngx_msec_t) ctx->delay_time);

    cln = ngx_http_cleanup_add(r, 0);
    if (cln == NULL) {
        return NGX_ERROR;
    }

    cln->handler = ngx_http_php_cosleep_cleanup;
    cln->data = r;

    r->keepalive = 0;

    /*
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
    */

    //ctx->ori_stack = EG(argument_stack);
    //ctx->execute_data = EG(current_execute_data);
    //ctx->execute_data->opline++;

    /*size_t execute_data_size = ZEND_MM_ALIGNED_SIZE(sizeof(zend_execute_data));
    size_t CVs_size = ZEND_MM_ALIGNED_SIZE(sizeof(zval **) * ctx->execute_data->op_array->last_var * (EG(active_symbol_table) ? 1 : 2));
    size_t Ts_size = ZEND_MM_ALIGNED_SIZE(sizeof(temp_variable)) * ctx->execute_data->op_array->T;
    size_t call_slots_size = ZEND_MM_ALIGNED_SIZE(sizeof(call_slot)) * ctx->execute_data->op_array->nested_calls;
    size_t stack_size = ZEND_MM_ALIGNED_SIZE(sizeof(zval*)) * ctx->execute_data->op_array->used_stack;
    size_t total_size = execute_data_size + Ts_size + CVs_size + call_slots_size + stack_size;
*/
    /*EG(argument_stack) = ngx_zend_vm_stack_new_page(ZEND_VM_STACK_PAGE_SIZE);
    EG(argument_stack)->prev = NULL;
    EG(argument_stack)->top = ngx_zend_vm_stack_frame_base(ctx->execute_data);
    ctx->argument_stack = EG(argument_stack);
*/
    //ctx->execute_data = EG(current_execute_data);
    
    //ctx->opline_ptr = EG(opline_ptr);
    //ctx->return_value_ptr_ptr = EG(return_value_ptr_ptr);
    
    //ctx->argument_stack = EG(argument_stack);
    //ctx->op_array = EG(active_op_array);
    //ctx->symbol_table = EG(active_symbol_table);
    //ngx_php_debug("EG(argument_stack): %p, %p, ", EG(argument_stack), ctx->ori_stack
        //ctx->execute_data->op_array->function_name, 
        //ctx->execute_data->prev_execute_data,
        //zend_get_opcode_name(ctx->execute_data->opline->opcode)
     //   );
    //ctx->execute_data->opline->opcode = 40;
    /*ngx_php_debug("\nys ctx->execute_data : %p %s\n, ctx->opline: %p\n, ctx->op_array: %p %p\n", 
        ctx->execute_data,
        zend_get_opcode_name(ctx->execute_data->opline->opcode), 
        EG(opline_ptr),
        EG(active_op_array),
        EG(active_symbol_table)
        );
    */
    /*zend_op_array op_array;
    zend_op op;
    int i;
    op_array = *ctx->execute_data->op_array;
    for (i = 0; i < (int)op_array.last; i++) {
        op = op_array.opcodes[i];
        php_printf("|        [%d].opcode = %p(%s)\n", i, &op_array.opcodes[i], zend_get_opcode_name(op.opcode));
                } */
    ngx_http_php_coroutine_yield(r);
    /*
    r = ngx_php_request;

    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    ctx->execute_data->opline++;
    EG(current_execute_data) = ctx->execute_data;
    EG(argument_stack) = ctx->argument_stack;
    EG(return_value_ptr_ptr) = ctx->return_value_ptr_ptr;
    ctx->op_array->fn_flags &= ~ZEND_ACC_GENERATOR;
    ngx_php_debug("%d\n", ctx->op_array->fn_flags);
    */
    //EG(active_op_array) = ctx->op_array;
    //EG(active_symbol_table) = ctx->symbol_table;
    //ngx_php_debug("EG(argument_stack): %p, %p, %p, %s", EG(argument_stack), ctx->ori_stack, 
    //    EG(current_execute_data), 
    //    zend_get_opcode_name(ctx->execute_data->opline->opcode)
    //);

    //sleep(5);
    //EG(opline_ptr) = ctx->opline_ptr + 1;
    //EG(current_execute_data) = ctx->execute_data->prev_execute_data;
    //ngx_php_debug("ye ctx->execute_data : %p, ctx->opline: %p", EG(current_execute_data), EG(opline_ptr));

    //zend_execute_ex(ctx->execute_data);

    return NGX_OK;
}

void func1(void *arg)
{
    ngx_http_request_t *r;
    ngx_http_php_ctx_t *ctx;

    r = arg;
    ngx_php_request = r;

    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    zend_execute_ex(ctx->execute_data);
    ngx_http_php_zend_uthread_continue(r);
}

static void 
ngx_http_php_cosleep_handler(ngx_event_t *ev)
{
    ngx_http_request_t *r;

    r = ev->data;
    
    ngx_php_request = r;
    
    //ngx_http_php_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);
    /*
    EG(current_execute_data) = ctx->execute_data;
    EG(opline_ptr) = ctx->opline_ptr;
    EG(argument_stack) = ctx->argument_stack;
    EG(active_op_array) = ctx->execute_data->op_array;
    ngx_php_debug("\nrs ctx->execute_data : %p %s\n, ctx->opline: %p\n, ctx->op_array: %p %p\n", 
        EG(current_execute_data), 
        zend_get_opcode_name(EG(current_execute_data)->opline->opcode), 
        EG(opline_ptr),
        EG(active_op_array),
        EG(current_execute_data)->symbol_table
    );*/
    ngx_http_php_coroutine_resume(r);

    ngx_php_debug("coroutine resume");
    //efree(ctx->op_array);
    ngx_http_php_zend_uthread_continue(r);
    /*
    r = ngx_php_request;

    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    
    ctx->execute_data->opline++;
    EG(current_execute_data) = ctx->execute_data;

    ngx_php_debug("\nye ctx->execute_data : %p %s\n, ctx->opline: %p\n, ctx->op_array: %p %p\n", 
        EG(current_execute_data), 
        zend_get_opcode_name(EG(current_execute_data)->opline->opcode), 
        EG(opline_ptr),
        EG(active_op_array),
        EG(current_execute_data)->symbol_table
    );

    ctx->coro = ngx_http_php_coroutine_alloc(r);
    ctx->coro->routine = func1;
    ctx->coro->data = r;
    ngx_http_php_coroutine_run(r);
    */
}

