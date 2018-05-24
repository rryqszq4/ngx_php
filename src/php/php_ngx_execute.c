/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */

#include "../ngx_php_debug.h"
#include "../ngx_http_php_module.h"

#include "php_ngx_execute.h"

void ngx_coexecute_ex(zend_execute_data *execute_data TSRMLS_DC)
{
    ngx_php_debug("start");
    ngx_php_debug("execute_data: %p, %p", execute_data, execute_data->opline);
    //ori_execute_ex(execute_data TSRMLS_CC);

    while(1){
        int ret;
        ngx_php_debug(" loop: \nexecute_data: %p, \nexecute_data->opline: %p, \nEG(current_execute_data): %p, \nEG(current_execute_data)->opline: %p, \nEG(opline_ptr): %p", 
            execute_data, execute_data->opline, EG(current_execute_data),EG(current_execute_data)->opline, *EG(opline_ptr));
        ngx_php_debug("%p, %p", execute_data->call, EG(current_execute_data)->call);
        if (execute_data->call || EG(current_execute_data)->call){
        ngx_php_debug("%p, %d, %p, %d\n", 
        	execute_data->call->called_scope, execute_data->call->num_additional_args, 
        	EG(current_execute_data)->call->called_scope, EG(current_execute_data)->call->num_additional_args);
        	
        	//EG(current_execute_data)->call->num_additional_args = 0;

        }
        zend_op_array op_array;
        zend_op op;
        int i;
        op_array = *EG(current_execute_data)->op_array;
        for (i = 0; i < (int)op_array.last; i++) {
            op = op_array.opcodes[i];
            ngx_php_debug("|        [%d].opcode = %p(%s)\n", i, &op_array.opcodes[i], zend_get_opcode_name(op.opcode));
                }

        if ((ret = EG(current_execute_data)->opline->handler(EG(current_execute_data) TSRMLS_CC)) > 0) {
            if (ret == 1) {
                return ;
            } 
        }
    }

    ngx_php_debug("EG(argument_stack): %p", EG(argument_stack));
    ngx_php_debug("end: %p, %p", execute_data, execute_data->opline);
}

void 
ngx_execute_internal(zend_execute_data *execute_data_ptr, zend_fcall_info *fci, int return_value_used TSRMLS_DC)
{

    execute_internal(execute_data_ptr, fci, return_value_used TSRMLS_CC);

}