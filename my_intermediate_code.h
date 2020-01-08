#ifndef _MY_INTER_CODE_H_
#define _MY_INTER_CODE_H_

void code_function_state(int type, char *name, int para_num, struct symbol *para_begin);
void code_function_call(int cnt, struct function *target, int index);
void code_constant_define();
void code_variable_state(int type, bool isarray, char *name, int bound);

#endif 