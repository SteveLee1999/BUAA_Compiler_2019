#ifndef _MY_MIPS_H_
#define _MY_MIPS_H_

void mips_pass_constant();
void mips_assign_register();
void mips_print_head();
void mips_global_con();
void mips_global_var();
void mips_function(int *num);
void mips_local_con();
void mips_local_var();
void mips_func_para(int num);
void mips_add();
void mips_sub();
void mips_mul();
void mips_div();
void mips_assign();
void mips_eq();
void mips_neq();
void mips_gt();
void mips_ge();
void mips_lt();
void mips_le();
void mips_load_array();
void mips_store_array();
void mips_printf();
void mips_scanf();
void mips_bnt();
void mips_bt();
void mips_goto();
void mips_return();
void mips_label();
void mips_function_call();
void mips_initialize_pointer();
void mips_store_data();
void get_src_value_or_reg(int usable, char *str, bool *type, int *ret);
void get_dst_reg_or_addr(char *str, int *type, int *ret);
void get_array_offset(char *array, bool *type, int *offset);

void pc_function();
void pc_parameter();
void pc_beginblock();
void pc_calculate();
void pc_assign();
void pc_printf();
void pc_scanf();
void pc_branch();
void pc_return_or_push();
void pc_from_array();
void pc_to_array();
void pc_call_function();
void pc_label();
void pc_goto();
void get_src_constant_value(char *str, bool *type, int *ret);
void store_dst_constant_value(char *str, int value);
void change_dst_constant_value(char *str);

bool is_num(char *str, int *value);
bool is_local_con(char *str, struct object **ret);
bool is_local_var(char *str, struct object **ret);
bool is_global_con(char *str, struct object **ret);
bool is_global_var(char *str, struct object **ret);

#endif 
