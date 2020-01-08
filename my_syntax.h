#ifndef _MY_SYNTAX_H_
#define _MY_SYNTAX_H_

void gram_procedure();
void gram_constant_state();
void gram_constant_define();
void gram_variable_state();
void gram_variable_define();
void gram_return_function_define();
void gram_head_assert(int *addr);
void gram_void_function_define();
void gram_main_function();
void gram_sentence();
void gram_composite_sentence();
void gram_array_sentence();
void gram_condition_sentence();
void gram_condition();
void gram_loop_sentence();
void gram_return_call_sentence(int *a, char *b);
void gram_void_call_sentence();
void gram_array_sentence();
void gram_assign_sentence();
void gram_read_sentence();
void gram_write_sentence();
void gram_return_sentence();
void gram_parameter_table();
void gram_expression(int *a, char *b);
void gram_term(int *a, char *b);
void gram_factor(int *a, char *b);
void gram_step(int *a);
void gram_value_parameter_table(struct function *f);
int gram_identifier(int a, int *b);
int gram_string();
int gram_unsigned_integer(int *a);
bool gram_integer(int *a);
bool gram_character(int *a);
void p_move();
void initialize_pointer();
void fill_symbol_table(int type, bool isarray, bool isfunction, bool isconst);
void check_symbol_table(int *type_addr);
void check_function_parameter(int a, int *b, struct function *f, int line);
void check_const();

#endif 