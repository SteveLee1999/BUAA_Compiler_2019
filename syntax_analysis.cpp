#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include "my_structure.h"
#include "my_syntax.h"
#include "my_error.h"
#include "my_intermediate_code.h"

extern struct word *p;
extern int line;
int a;

struct function *f, *f_head; // 不包含主函数
struct symbol *s, *s_global, *s_local;
struct constant *c, *c_head;

bool iden_check = false; //true表示进行标识符是否定义的检查
bool have_return_sentence; // true表示已经有了返回语句
int *b; // 填充gram_identifier的参数
int return_state; // true表示<有返回值函数定义>的分析中
int label_count;
int temp_count;

void gram_procedure() // <程序> 
{
	initialize_pointer(); // 初始化各种全局指针
	if (strcmp(p->code, "CONSTTK") == 0)
	{
		gram_constant_state();
	}
	if (strcmp(p->code, "INTTK") == 0 || strcmp(p->code, "CHARTK") == 0)
	{
		if (strcmp(p->next->next->code, "LPARENT") != 0)
		{
			gram_variable_state();
		}
	}
	while (strcmp(p->next->code, "MAINTK") != 0)
	{
		if (strcmp(p->code, "INTTK") == 0 || strcmp(p->code, "CHARTK") == 0)
		{
			gram_return_function_define();
		}
		else if (strcmp(p->code, "VOIDTK") == 0)
		{
			gram_void_function_define();
		}
		else
		{
			printf("error in gram_procedure!\n");
		}
	}
	gram_main_function();
	
	/*
	struct function *q;
	for (q = f_head;q != f; q = q->next)
	{
		printf("........\n%s:",q->name);
		for (int i = 1;i <= q->para_num;i++)
			printf("%d ", q->para_table[i]);
		printf("\n");
	} 
	
	struct symbol *r;
	for (r = s_global;r != s;r = r->next)
	{
		printf("........\n%s:%d\n", r->name, r->type);
	}
	*/
}

void gram_constant_state() // <常量说明>
{
	if (strcmp(p->code, "CONSTTK") != 0)
	{
		printf("error in gram_constant_state!\n");
	}
	p_move();
	gram_constant_define();
	if (strcmp(p->code, "SEMICN") != 0)
	{
		error_k(p->before->line);
	}
	else
	{
		p_move();
	}
	while (strcmp(p->code, "CONSTTK") == 0)
	{
		p_move();
		gram_constant_define();
		if (strcmp(p->code, "SEMICN") != 0)
		{
			error_k(p->before->line);
		}
		else
		{
			p_move();
		}
	}
	// printf("<常量说明>\n");
}

void gram_constant_define() // <常量定义>
{
	if (strcmp(p->code, "INTTK") == 0)
	{
		p_move();
		gram_identifier(1, b);
		
		strcpy(c->name, p->before->name);

		if (strcmp(p->code, "ASSIGN") != 0)
		{
			printf("error in gram_constant_define!\n");
		}
		p_move(); 
		int value;
		if (!gram_integer(&value) || (strcmp(p->code, "COMMA") != 0 && strcmp(p->code, "SEMICN") != 0))
		{
			error_o(p->line);
		}
		
		c->type = 1;
		c->value = value;
		code_constant_define();
		c->next = (struct constant*)malloc(sizeof(struct constant));
		c = c->next;

		while (strcmp(p->code, "COMMA") == 0)
		{
			p_move();
			gram_identifier(1, b);

			strcpy(c->name, p->before->name);

			if (strcmp(p->code, "ASSIGN") != 0)
			{
				break;
			}
			p_move();
			if (!gram_integer(&value) || (strcmp(p->code, "COMMA") != 0 && strcmp(p->code, "SEMICN") != 0))
			{
				error_o(p->line);
			}
			
			c->type = 1;
			c->value = value;
			code_constant_define();
			c->next = (struct constant*)malloc(sizeof(struct constant));
			c = c->next;

		}
		// printf("<常量定义>\n");
	}
	else if (strcmp(p->code, "CHARTK") == 0)
	{
		p_move();
		gram_identifier(1, b);

		strcpy(c->name, p->before->name);

		if (strcmp(p->code, "ASSIGN") != 0)
		{
			printf("error in gram_constant_define!\n");
		}
		p_move();
		int value;
		if (!gram_character(&value) || (strcmp(p->code, "COMMA") != 0 && strcmp(p->code, "SEMICN") != 0))
		{
			error_o(p->line);
			
		}
		
		c->type = 2;
		c->value = value;
		code_constant_define();
		c->next = (struct constant*)malloc(sizeof(struct constant));
		c = c->next;

		while (strcmp(p->code, "COMMA") == 0)
		{
			p_move();
			gram_identifier(1, b);
			
			strcpy(c->name, p->before->name);
			
			if (strcmp(p->code, "ASSIGN") != 0)
			{
				break;
			}
			p_move();
			if (!gram_character(&value) || (strcmp(p->code, "COMMA") != 0 && strcmp(p->code, "SEMICN") != 0))
			{
				error_o(p->line);
				
			}

			c->type = 2;
			c->value = value;
			code_constant_define();
			c->next = (struct constant*)malloc(sizeof(struct constant));
			c = c->next;

		}
		// printf("<常量定义>\n");
	}
	else
	{
		printf("error in gram_constant_define!\n");
	}
}

void gram_variable_state() // <变量说明>
{
	gram_variable_define();
	if (strcmp(p->code, "SEMICN") != 0)
	{
		error_k(p->before->line);
	}
	else
	{
		p_move();
	}
	while (strcmp(p->code, "INTTK") == 0 || strcmp(p->code, "CHARTK") == 0)
	{
		if (strcmp(p->next->next->code, "COMMA") != 0 &&
			strcmp(p->next->next->code, "SEMICN") != 0 &&
			strcmp(p->next->next->code, "LBRACK") != 0) // 涓轰簡涓庡嚱鏁板０鏄庡尯鍒嗗紑
		{
			break;
		}
		gram_variable_define();
		if (strcmp(p->code, "SEMICN") != 0)
		{
			error_k(p->before->line);
		}
		else
		{
			p_move();
		}
	}
	// printf("<变量说明>\n");
}

void gram_variable_define() // <变量定义>
{
	int type;
	if (strcmp(p->code, "INTTK") == 0)
	{
		type = 1;
	}
	else if (strcmp(p->code, "CHARTK") == 0)
	{
		type = 2;
	}
	else
	{
		printf("error in gram_variable_define!\n");
	}
	p_move();
	gram_identifier(1, b);
	if (strcmp(p->code, "LBRACK") == 0)
	{
		fill_symbol_table(type, true, false, false);
		p_move();
		if (strcmp(p->name, "0") == 0 && strcmp(p->code, "INTCON") == 0) // 鏃犵鍙锋暣鏁拌繖閲?0 
		{
			printf("error in gram_variable_define!\n");
		}
		gram_unsigned_integer(b);
		code_variable_state(type, true, p->before->before->before->name, *b);

		if (strcmp(p->code, "RBRACK") != 0)
		{
			error_m(p->before->line);
		}
		p_move();
	}
	else
	{
		fill_symbol_table(type, false, false, false);
		code_variable_state(type, false, p->before->name, 0);
	}
	while (strcmp(p->code, "COMMA") == 0)
	{
		p_move();
		gram_identifier(1, b);
		if (strcmp(p->code, "LBRACK") == 0)
		{
			fill_symbol_table(type, true, false, false);
			p_move();
			if (strcmp(p->name, "0") == 0 && strcmp(p->code, "INTCON") == 0) // 鏃犵鍙锋暣鏁?0 
			{
				printf("error in gram_variable_define!\n");
			}
			gram_unsigned_integer(b);
			code_variable_state(type, true, p->before->before->before->name, *b);
			if (strcmp(p->code, "RBRACK") != 0)
			{
				error_m(p->before->line);
			}
			p_move();
		}
		else
		{
			fill_symbol_table(type, false, false, false);
			code_variable_state(type, false, p->before->name, 0);
		}
	}
	// printf("<变量定义>\n");
}

void gram_return_function_define() // <有返回值函数定义>
{	
	int return_type;
	gram_head_assert(&return_type);
	s_local = s; // 函数名也算“全局变量”
	if (strcmp(p->code, "LPARENT") != 0)
	{
		printf("error in gram_return_function_define!\n");
	}
	strcpy(f->name, p->before->name);	
	p_move();
	struct symbol *para_begin = s; 
	// s此时指向的即将填入第一个参数（如果有参数的话）
	if (strcmp(p->code, "RPARENT") == 0)
	{
		f->para_num = 0;
		// printf("<参数表>\n");
	}
	else
	{
		// 注意进入<参数表>之前f已经指向相应位置
		gram_parameter_table();
		if (strcmp(p->code, "RPARENT") != 0)
		{
			error_l(p->before->line);
		}
	}

	f->returntype = return_type;
	code_function_state(f->returntype, f->name, f->para_num, para_begin);
	f->next = (struct function*)malloc(sizeof(struct function));
	f = f->next;

	p_move(); 
	if (strcmp(p->code, "LBRACE") != 0)
	{
		printf("error in gram_return_function_define!\n");
	}
	p_move();
	have_return_sentence = false;
	return_state = return_type;
	gram_composite_sentence();
	return_state = 0;
	if (!have_return_sentence)
	{
		error_h(p->line);
	}
	if (strcmp(p->code, "RBRACE") != 0)
	{
		printf("error in gram_return_function_define!\n");
	}
	p_move();
	// printf("<有返回值函数定义>\n");

	s = s_local;
	s_local = s_global;
}

void gram_head_assert(int *addr) // <声明头部>
{
	if (strcmp(p->code, "INTTK") == 0)
	{
		*addr = 1;
	}
	else if (strcmp(p->code, "CHARTK") == 0)
	{
		*addr = 2;
	}
	p_move();
	gram_identifier(1, b);
	fill_symbol_table(*addr, false, true, false); // 注意p指针位置
	// printf("<声明头部>\n");
}

void gram_void_function_define() // <无返回值函数定义> 
{
	if (strcmp(p->code, "VOIDTK") != 0)
	{
		printf("error in gram_void_function_define!\n");
	}
	p_move();
	gram_identifier(1, b);
	fill_symbol_table(3, false, true, false);
	s_local = s;
	if (strcmp(p->code, "LPARENT") != 0)
	{
		printf("error in gram_void_function_define!\n");
	}
	strcpy(f->name, p->before->name);
	p_move();

	struct symbol *para_begin = s;
	if (strcmp(p->code, "RPARENT") == 0)
	{
		f->para_num = 0;
		// printf("<参数表>\n");
	}
	else
	{
		gram_parameter_table();
		if (strcmp(p->code, "RPARENT") != 0)
		{
			error_l(p->before->line);
		}
	}
	
	f->returntype = 0;
	code_function_state(f->returntype, f->name, f->para_num, para_begin);
	f->next = (struct function*)malloc(sizeof(struct function));
	f = f->next;
	
	p_move();
	if (strcmp(p->code, "LBRACE") != 0)
	{
		printf("error in gram_void_function_define!\n");
	}
	p_move();
	return_state = 3;
	gram_composite_sentence();
	return_state = 0;
	if (strcmp(p->code, "RBRACE") != 0)
	{
		printf("error in gram_void_function_define!\n");
	}
	p_move();
	// printf("<无返回值函数定义>\n");

	s = s_local;
	s_local = s_global;
}

void gram_main_function() // <主函数>
{
	char name[6];
	strcpy(name, "main");
	code_function_state(0, name, 0, s); // 第四个s是凑数的，因为第三个参数已经表明有0个参数

	s_local = s;
	if (strcmp(p->code, "VOIDTK") != 0)
	{
		printf("error in gram_main_function!\n");
	}
	p_move();
	if (strcmp(p->code, "MAINTK") != 0)
	{
		printf("error in gram_main_function!\n");
	}
	p_move();
	if (strcmp(p->code, "LPARENT") != 0)
	{
		printf("error in gram_main_function!\n");
	}
	p_move();
	if (strcmp(p->code, "RPARENT") != 0)
	{
		error_l(p->before->line);
	}
	p_move();
	if (strcmp(p->code, "LBRACE") != 0)
	{
		printf("error in gram_main_function!\n");
	}
	p_move();
	return_state = 3;
	gram_composite_sentence();
	return_state = 0;
	if (strcmp(p->code, "RBRACE") != 0)
	{
		printf("error in gram_main_function!\n");
	}
	p_move();
	// printf("<主函数>\n");

	s = s_local;
	s_local = s_global;
}

void gram_composite_sentence() // <复合语句>
{
	if (strcmp(p->code, "CONSTTK") == 0)
	{
		gram_constant_state();
	}
	if (strcmp(p->code, "INTTK") == 0 || strcmp(p->code, "CHARTK") == 0)
	{
		gram_variable_state();
	}
	gram_array_sentence();
	// printf("<复合语句>\n");
}

void gram_array_sentence() // <语句列>
{

	while (strcmp(p->code, "IFTK") == 0 ||
		strcmp(p->code, "WHILETK") == 0 ||
		strcmp(p->code, "FORTK") == 0 ||
		strcmp(p->code, "DOTK") == 0 ||
		strcmp(p->code, "SCANFTK") == 0 ||
		strcmp(p->code, "PRINTFTK") == 0 ||
		strcmp(p->code, "RETURNTK") == 0 ||
		strcmp(p->code, "LBRACE") == 0 ||
		strcmp(p->next->code, "ASSIGN") == 0 ||
		strcmp(p->code, "SEMICN") == 0 ||
		gram_identifier(0, b))
	{
		gram_sentence();
	}
	// printf("<语句列>\n");
}

void gram_sentence() // <语句>
{
	iden_check = true;
	temp_count = 0;

	if (strcmp(p->code, "IFTK") == 0)
	{
		gram_condition_sentence();
	}
	else if (strcmp(p->code, "WHILETK") == 0 ||
		strcmp(p->code, "FORTK") == 0 ||
		strcmp(p->code, "DOTK") == 0)
	{
		gram_loop_sentence();
	}
	else if (strcmp(p->code, "LBRACE") == 0)
	{
		p_move();
		gram_array_sentence();
		if (strcmp(p->code, "RBRACE") != 0)
		{
			printf("error in gram_sentence!\n");
		}
		p_move();
	}
	else if (strcmp(p->code, "SCANFTK") == 0)
	{
		gram_read_sentence();
		if (strcmp(p->code, "SEMICN") != 0)
		{
			error_k(p->before->line);
		}
		else
		{
			p_move();
		}
	}
	else if (strcmp(p->code, "PRINTFTK") == 0)
	{
		gram_write_sentence();
		if (strcmp(p->code, "SEMICN") != 0)
		{
			error_k(p->before->line);
		}
		else
		{
			p_move();
		}
	}
	else if (strcmp(p->code, "RETURNTK") == 0)
	{
		gram_return_sentence();
		if (strcmp(p->code, "SEMICN") != 0)
		{
			error_k(p->before->line);
		}
		else
		{
			p_move();
		}
	}
	else if (strcmp(p->code, "SEMICN") == 0)
	{
		p_move();
	}
	else if (strcmp(p->next->code, "ASSIGN") == 0 ||
		(gram_identifier(0, b) &&
		strcmp(p->next->code, "LBRACK") == 0))
	{
		gram_assign_sentence(); 
		if (strcmp(p->code, "SEMICN") != 0)
		{
			error_k(p->before->line);
		}
		else
		{
			p_move();
		}
	}
	else if (gram_identifier(0, b))
	{
		struct function *q;
		for (q = f_head; q != f; q = q->next)
		{
			if (strcmp(q->name, p->name) == 0)
			{
				break;
			}
		}
		if (q != f)
		{
			if (q->returntype == 1 || q->returntype == 2)
			{
				char forward_name[50];
				gram_return_call_sentence(b, forward_name); // 凑数参数
			}
			else if (q->returntype == 0)
			{
				gram_void_call_sentence();
			}
			else
			{
				printf("error in gram_sentence!\n");
			}
		}
		else
		{
			printf("error in gram_sentence!\n");
		}
		if (strcmp(p->code, "SEMICN") != 0)
		{
			error_k(p->before->line);
		}
		else
		{
			p_move();
		}
	}
	else
	{
		printf("error in gram_sentence!\n");
	}

	iden_check = false;
	// printf("<语句>\n");
}

void gram_condition_sentence() // <条件语句>
{
	if (strcmp(p->code, "IFTK") != 0)
	{
		printf("error in gram_condition_sentence!\n");
	}
	p_move();
	if (strcmp(p->code, "LPARENT") != 0)
	{
		printf("error in gram_condition_sentence!\n");
	}
	p_move(); 
	gram_condition();
	printf("bnt @temp1 label%d\n", ++label_count);
	printf("\n########\n");
	int label = label_count;

	if (strcmp(p->code, "RPARENT") != 0)
	{
		error_l(p->before->line);
	}
	p_move();
	gram_sentence();

	if (strcmp(p->code, "ELSETK") == 0)
	{
		printf("goto label%d\n", ++label_count);
		printf("\n########\n");
		printf("label%d:\n", label);
		label = label_count;
		p_move();
		gram_sentence();
		printf("\n########\n");
		printf("label%d:\n", label);
	}
	else
	{
		printf("\n########\n");
		printf("label%d:\n", label);
	}
	// printf("<条件语句>\n");
}

void gram_condition() // <条件> 
// 默认使用@temp1作为中间变量
{
	int type;
	char forward_name_1[100];
	char forward_name_2[100];
	gram_expression(&type, forward_name_1);
	if (type == 2) // char
	{
		error_f(p->before->line);
	}
	if (strcmp(p->code, "GRE") == 0 ||
		strcmp(p->code, "GEQ") == 0 ||
		strcmp(p->code, "LSS") == 0 ||
		strcmp(p->code, "LEQ") == 0 ||
		strcmp(p->code, "EQL") == 0 ||
		strcmp(p->code, "NEQ") == 0)
	{
		char op[5];
		strcpy(op, p->name); // 注意存的是name：==,!=之类
		p_move();
		gram_expression(&type, forward_name_2);
		if (type == 2)
		{
			error_f(p->before->line);
		}
		printf("%s %s %s @temp1\n", op, forward_name_1, forward_name_2);
	}
	else
	{
		printf("!= %s 0 @temp1\n", forward_name_1);
	}
	// printf("<条件>\n");
}

void gram_loop_sentence() // <循环语句>
{
	int label;
	if (strcmp(p->code, "WHILETK") == 0)
	{
		p_move();
		if (strcmp(p->code, "LPARENT") != 0)
		{
			printf("error in gram_loop_sentence!\n");
		}
		p_move();

		printf("\n########\n");
		printf("label%d:\n", ++label_count);
		label = label_count;
		gram_condition(); // 条件每次跳转回来都要重新计算
		printf("bnt @temp1 label%d\n", ++label_count);
		printf("\n########\n");

		if (strcmp(p->code, "RPARENT") != 0)
		{
			error_l(p->before->line);
		}
		p_move();
		gram_sentence();
		// printf("<循环语句>\n");

		printf("goto label%d\n", label);
		printf("\n########\n");
		printf("label%d:\n", label + 1);
	}
	else if (strcmp(p->code, "DOTK") == 0)
	{
		printf("\n########\n");
		printf("label%d:\n", ++label_count);
		label = label_count;

		p_move();
		gram_sentence();
		if (strcmp(p->code, "WHILETK") != 0)
		{
			error_n(p->line);
		}
		p_move();
		if (strcmp(p->code, "LPARENT") != 0)
		{
			printf("error in gram_loop_sentence!\n");
		}
		p_move();
		
		gram_condition();
		printf("bt @temp1 label%d\n", label);
		printf("\n########\n");

		if (strcmp(p->code, "RPARENT") != 0)
		{
			error_l(p->before->line);
		}
		p_move();
		// printf("<循环语句>\n");
	}
	else if (strcmp(p->code, "FORTK") == 0)
	{
		char name_1[50];
		char name_2[50];
		char forward_name[50];
		char op;
		int step;

		p_move();
		if (strcmp(p->code, "LPARENT") != 0)
		{
			printf("error in gram_loop_sentence!\n");
		}
		p_move();
		gram_identifier(1, b);
		strcpy(name_1, p->before->name);
		if (strcmp(p->code, "ASSIGN") != 0)
		{
			printf("error in gram_loop_sentence!\n");
		}
		p_move();
		gram_expression(b, forward_name);
		printf("= %s %s\n", forward_name, name_1);

		if (strcmp(p->code, "SEMICN") != 0)
		{
			error_k(p->before->line);
		}
		else
		{
			p_move();
		}
		printf("\n########\n");
		printf("label%d:\n", ++label_count);
		gram_condition();
		printf("bnt @temp1 label%d\n", ++label_count);
		printf("\n########\n");
		label = label_count;
		
		if (strcmp(p->code, "SEMICN") != 0)
		{
			error_k(p->before->line);
		}
		else
		{
			p_move();
		}
		
		gram_identifier(1, b);
		strcpy(name_1, p->before->name);
		
		if (strcmp(p->code, "ASSIGN") != 0)
		{
			printf("error in gram_loop_sentence!\n");
		}
		p_move();

		gram_identifier(1, b);
		strcpy(name_2, p->before->name);

		if (strcmp(p->code, "PLUS") == 0 || strcmp(p->code, "MINU") == 0)
		{
			op = p->name[0]; // 注意是name!
		}
		else
		{
			printf("error in gram_loop_sentence!\n");
		}
		p_move();
		gram_step(&step);
		if (strcmp(p->code, "RPARENT") != 0)
		{
			error_l(p->before->line);
		}
		p_move();
		gram_sentence();
		// printf("<循环语句>\n");

		printf("%c %s %d %s\n", op, name_2, step, name_1);
		printf("goto label%d\n", label - 1);
		printf("\n########\n");
		printf("label%d:\n", label);
	}
	else
	{
		printf("error in gram_loop_sentence!\n");
	}
}

void gram_return_call_sentence(int *type_addr, char *back_name) // <有返回值函数调用语句> 
{
	struct function *target_func; // 调用语句对应的函数
	if (!gram_identifier(1, b))
	{
		printf("error in gram_return_call_sentence\n");
	}
	for (target_func = f_head; target_func != f; target_func = target_func->next)
	{
		if (strcmp(target_func->name, p->before->name) == 0)
		{
			break;
		}
	}
	if (target_func == f)
	{
		printf("error in gram_return_call_sentence!\n");
	}
	if (strcmp(p->code, "LPARENT") != 0)
	{
		printf("error in gram_return_call_sentence\n");
	}
	p_move();

	if (strcmp(p->code, "RPARENT") == 0)
	{
		int t; // 凑数用
		check_function_parameter(0, &t, target_func, p->line);
		// printf("<值参数表>\n");
	}
	else
	{
		gram_value_parameter_table(target_func);
		if (strcmp(p->code, "RPARENT") != 0)
		{
			error_l(p->before->line);
		}
	}
	printf("call %s\n", target_func->name);
	memset(back_name, 0, sizeof(back_name));
	sprintf(back_name, "@temp%d", ++temp_count);
	printf("%s = RET\n", back_name);
	p_move();
	*type_addr = target_func->returntype;
	// printf("<有返回值函数调用语句>\n");
}

void gram_void_call_sentence() // <无返回值函数调用语句>
{
	struct function *target_func; // 调用语句对应的函数
	if (!gram_identifier(1, b))
	{
		printf("error in gram_void_call_sentence\n");
	}
	for (target_func = f_head; target_func != f; target_func = target_func->next)
	{
		if (strcmp(target_func->name, p->before->name) == 0)
		{
			break;
		}
	}
	if (target_func == f)
	{
		printf("error in gram_void_call_sentence!\n");
	}
	if (strcmp(p->code, "LPARENT") != 0)
	{
		printf("error in gram_void_call_sentence\n");
	}
	p_move();

	struct symbol *para_begin = s;
	if (strcmp(p->code, "RPARENT") == 0)
	{
		int t;
		check_function_parameter(0, &t, target_func, p->line);
		// printf("<值参数表>\n");
	}
	else
	{
		gram_value_parameter_table(target_func);
		if (strcmp(p->code, "RPARENT") != 0)
		{
			error_l(p->before->line);
		}
	}
	printf("call %s\n", target_func->name);
	printf("void = RET\n");
	p_move();
	// printf("<无返回值函数调用语句>\n");
}

void gram_assign_sentence() // <赋值语句>
{
	char forward_name_1[50];
	char forward_name_2[50];

	if (!gram_identifier(1, b))
	{
		printf("error in gram_assign_sentence!\n");
	}
	else
	{
		char name[50];
		strcpy(name, p->before->name);
		check_const();
		if (strcmp(p->code, "ASSIGN") == 0)
		{
			p_move();
			gram_expression(b, forward_name_1);
			printf("= %s %s\n", forward_name_1, name);
		}
		else if (strcmp(p->code, "LBRACK") == 0)
		{
			p_move();
			int type;
			gram_expression(&type, forward_name_1);
			if (type == 2)
			{
				error_i(p->before->line);
			}
			if (strcmp(p->code, "RBRACK") != 0)
			{
				error_m(p->before->line);
			}
			p_move();
			if (strcmp(p->code, "ASSIGN") != 0)
			{
				printf("error in gram_assign_sentence!\n");
			}
			p_move();
			gram_expression(b, forward_name_2);
			printf("{} %s %s %s\n", forward_name_1, forward_name_2, name); // name[t1] = t2;
		}
		else
		{
			printf("error in gram_assign_sentence!\n");
		}
	}
	// printf("<赋值语句>\n");
}

int gram_identifier(int forward, int *type_addr) // <标识符>
{
	if (strcmp(p->code, "IDENFR") != 0)
	{
		return 0;
	}
	else
	{
		if (iden_check)
		{
			check_symbol_table(type_addr);
		}
		if (forward == 1)
		{
			p_move();
		}
		return 1;
	}
}

void gram_read_sentence() // <读语句>
{
	int type;
	if (strcmp(p->code, "SCANFTK") != 0)
	{
		printf("error in gram_read_sentence!\n");
	}
	p_move();
	if (strcmp(p->code, "LPARENT") != 0)
	{
		printf("error in gram_read_sentence!\n");
	}
	p_move();
	if (!gram_identifier(1, &type))
	{
		printf("error in gram_read_sentence!\n");
	}
	check_const();

	if (type == 1)
	{
		printf("scanf %%d %s\n", p->before->name);
	}
	else if (type == 2)
	{
		printf("scanf %%c %s\n", p->before->name);
	}
	else
	{
		printf("error in gram_read_sentence!\n");
	}

	while (strcmp(p->code, "COMMA") == 0)
	{
		p_move();
		if (!gram_identifier(1, &type))
		{
			printf("error in gram_read_sentence!\n");
		}
		check_const();

		if (type == 1)
		{
			printf("scanf %%d %s\n", p->before->name);
		}
		else if (type == 2)
		{
			printf("scanf %%c %s\n", p->before->name);
		}
		else
		{
			printf("error in gram_read_sentence!\n");
		}
	}
	if (strcmp(p->code, "RPARENT") != 0)
	{
		error_l(p->before->line);
	}
	p_move();
	// printf("<读语句>\n");
}

void gram_write_sentence() // <写语句>
{
	int type;
	char forward_name[50];

	if (strcmp(p->code, "PRINTFTK") != 0)
	{
		printf("error in gram_write_sentence!\n");
	}
	p_move();
	if (strcmp(p->code, "LPARENT") != 0)
	{
		printf("error in gram_write_sentence!\n");
	}
	p_move();
	if (gram_string())
	{	
		if (strcmp(p->code, "COMMA") == 0)
		{
			char str[100];
			strcpy(str, p->before->name);
			p_move();
			gram_expression(&type, forward_name);
			if (type == 1)
			{
				printf("printf %%s%%d %s \"%s\"\n", forward_name, str);
			}
			else if (type == 2)
			{
				printf("printf %%s%%c %s \"%s\"\n", forward_name, str);
			}
			else
			{
				printf("error in gram_write_sentence!\n");
			}

			if (strcmp(p->code, "RPARENT") != 0)
			{
				error_l(p->before->line);
			}
			p_move();
			// printf("<写语句>\n");
		}
		else if (strcmp(p->code, "RPARENT") == 0)
		{
			printf("printf %%s \"%s\"\n", p->before->name);
			p_move();
			// printf("<写语句>\n");
		}
		else
		{
			printf("error in gram_write_sentence!\n");
		}
	}
	else
	{
		gram_expression(&type, forward_name);
		if (type == 1)
		{
			printf("printf %%d %s\n", forward_name);
		}
		else if (type == 2)
		{
			printf("printf %%c %s\n", forward_name);
		}
		else
		{
			printf("error in gram_write_sentence!\n");
		}

		if (strcmp(p->code, "RPARENT") != 0)
		{
			error_l(p->before->line);
		}
		p_move();
		// printf("<写语句>\n");
	}
}

int gram_string() // <字符串>
{
	if (strcmp(p->code, "STRCON") != 0)
	{
		return 0;
	}
	else
	{
		p_move();
		// printf("<字符串>\n");
		return 1;
	}
}

void gram_return_sentence() // <返回语句>
{
	int return_type;
	int line = p->line;
	char forward_name[50];
	have_return_sentence = true;

	if (strcmp(p->code, "RETURNTK") != 0)
	{
		printf("error in gram_return_sentence!\n");
	}
	p_move();
	if (strcmp(p->code, "LPARENT") == 0)
	{
		p_move();
		gram_expression(&return_type, forward_name);
		if (strcmp(p->code, "RPARENT") != 0)
		{
			error_l(p->before->line);
		}
		p_move();
		printf("ret %s\n", forward_name);
	}
	else
	{
		return_type = 3;
		printf("ret void\n");
	}


	if (return_state == 3 && return_type != return_state)
	{
		error_g(line);
	}
	else if ((return_state == 1 || return_state == 2)
		&& return_type != return_state)
	{
		error_h(line);
	}
	// printf("<返回语句>\n");
}

int gram_unsigned_integer(int *value) // <无符号整数>
{
	if (strcmp(p->code, "INTCON") != 0)
	{
		return 0;
	}
	else if (strcmp(p->name, "0") == 0)
	{
		p_move();
		*value = 0;
		// printf("<无符号整数>\n");
		return 1;
	}
	else if (p->name[0] == '0' || !isdigit(p->name[0]))
	{
		return 0;
	}
	int i;
	for (i = 1; i < strlen(p->name); i++)
	{
		if (!isdigit(p->name[i]))
		{
			return 0;
		}
	}
	*value = atoi(p->name);
	p_move();
	// printf("<无符号整数>\n");
	return 1;
}

bool gram_integer(int *value) // 是整数则返回true，value存值
{
	if (strcmp(p->code, "PLUS") == 0  &&
		strcmp(p->next->code, "INTCON") == 0)
	{
		p_move();
		gram_unsigned_integer(value);
		// printf("<整数>\n");
		return true;
	}
	else if (strcmp(p->code, "MINU") == 0 &&
		strcmp(p->next->code, "INTCON") == 0)
	{
		p_move();
		gram_unsigned_integer(value);
		int temp = *value;
		*value = -temp;
		// printf("<整数>\n");
		return true;
	}
	else if (strcmp(p->code, "INTCON") == 0)
	{
		gram_unsigned_integer(value);
		// printf("<整数>\n");
		return true;
	}
	else
	{
		return false;
	}
}

void gram_parameter_table() // <参数表>
{
	f->para_num = 0;
	int type;
	if (strcmp(p->code, "INTTK") == 0)
	{
		type = 1;
	}
	else if (strcmp(p->code, "CHARTK") == 0)
	{
		type = 2;
	}
	else
	{
		printf("error in gram_parameter_table!\n");
	}
	f->para_table[++f->para_num] = type;
	p_move();
	gram_identifier(1, b);
	fill_symbol_table(type, false, false, false);
	while (strcmp(p->code, "COMMA") == 0)
	{
		p_move();
		if (strcmp(p->code, "INTTK") == 0)
		{
			type = 1;
		}
		else if (strcmp(p->code, "CHARTK") == 0)
		{
			type = 2;
		}
		else
		{
			printf("error in gram_parameter_table!\n");
		}
		f->para_table[++f->para_num] = type;
		p_move();
		gram_identifier(1, b);
		fill_symbol_table(type, false, false, false);
	}
	// printf("<参数表>\n");
}

void gram_expression(int *type_addr, char *back_name) // <表达式> 
{
	char forward_name_1[50];
	char forward_name_2[50];
	bool is_minus = false;
	int temp_index = -1;

	if (strcmp(p->code, "PLUS") == 0)
	{
		p_move();
		*type_addr = 1;
	}
	else if (strcmp(p->code, "MINU") == 0)
	{
		p_move();
		*type_addr = 1;
		is_minus = true;
	}
	gram_term(type_addr, forward_name_1);

	if (is_minus && forward_name_1[0] == '@')
	{
		printf("- 0 %s %s\n", forward_name_1, forward_name_1);
	}
	else if (is_minus &&
		     (forward_name_1[0] == '_' || isalpha(forward_name_1[0])))
	{
		temp_index = ++temp_count;
		printf("- 0 %s @temp%d\n", forward_name_1, temp_index);
		memset(forward_name_1, 0, sizeof(forward_name_1));
		sprintf(forward_name_1, "@temp%d", temp_index);
	}
	else if (is_minus) // 是个数字
	{
		int tmp = -atoi(forward_name_1);
		memset(forward_name_1, 0, sizeof(forward_name_1));
		sprintf(forward_name_1, "%d", tmp);
	}

	if (strcmp(p->code, "PLUS") == 0 ||
		strcmp(p->code, "MINU") == 0)
	{
		if (temp_index == -1)
		{
			temp_index = ++temp_count;
		}
		memset(back_name, 0, sizeof(back_name));
		sprintf(back_name, "@temp%d", temp_index);
	}
	else
	{
		memset(back_name, 0, sizeof(back_name));
		strcpy(back_name, forward_name_1);
	}
	while (strcmp(p->code, "PLUS") == 0 ||
		strcmp(p->code, "MINU") == 0)
	{
		is_minus = (strcmp(p->code, "MINU") == 0) ? true : false;
		p_move();
		gram_term(type_addr, forward_name_2);
		*type_addr = 1;
		
		if (is_minus)
		{
			printf("- %s %s %s\n", forward_name_1, forward_name_2, back_name);
		}
		else
		{
			printf("+ %s %s %s\n", forward_name_1, forward_name_2, back_name);
		}
		memset(forward_name_1, 0, sizeof(forward_name_1));
		strcpy(forward_name_1, back_name);
	}
	// printf("<表达式>\n");
}

void gram_term(int *type_addr, char *back_name) // <项>
{
	char forward_name_1[50];
	char forward_name_2[50];
	bool is_mult;

	gram_factor(type_addr, forward_name_1);

	if (strcmp(p->code, "MULT") == 0 ||
		strcmp(p->code, "DIV") == 0)
	{
		memset(back_name, 0, sizeof(back_name));
		sprintf(back_name, "@temp%d", ++temp_count);
	}
	else
	{
		memset(back_name, 0, sizeof(back_name));
		strcpy(back_name, forward_name_1);
	}

	while (strcmp(p->code, "MULT") == 0 ||
		strcmp(p->code, "DIV") == 0)
	{
		is_mult = (strcmp(p->code, "MULT") == 0) ? true : false;
		p_move();
		gram_factor(type_addr, forward_name_2);
		*type_addr = 1;

		if (is_mult)
		{
			printf("* %s %s %s\n", forward_name_1, forward_name_2, back_name);
		}
		else
		{
			printf("/ %s %s %s\n", forward_name_1, forward_name_2, back_name);
		}
		memset(forward_name_1, 0, sizeof(forward_name_1));
		strcpy(forward_name_1, back_name);
	}
	// printf("<项>\n");
}

void gram_factor(int *type_addr, char *back_name) // <因子>
{
	int value;
	char forward_name[50];

	if (gram_identifier(0, type_addr))
	{	
		if (strcmp(p->next->code, "LBRACK") == 0) // 数组
		{		
			char name[30];
			strcpy(name, p->name);
			p_move();
			p_move();
			int type;
			gram_expression(&type, forward_name);
			if (type != 1)
			{
				error_i(p->before->line);
			}
			if (strcmp(p->code, "RBRACK") != 0)
			{
				error_m(p->before->line);
			}
			p_move();

			memset(back_name, 0, sizeof(back_name));
			sprintf(back_name, "@temp%d", ++temp_count);
			printf("[] %s %s %s\n", name, forward_name, back_name);
		}
		else if (strcmp(p->next->code, "LPARENT") == 0) // 函数
		{
			gram_return_call_sentence(type_addr, forward_name);
			memset(back_name, 0, sizeof(back_name));
			strcpy(back_name, forward_name);
		}
		else // 普通标识符变量
		{
			memset(back_name, 0, sizeof(back_name));
			strcpy(back_name, p->name);
			p_move();
		}
		// printf("<因子>\n");
	}
	else if (strcmp(p->code, "LPARENT") == 0) // (表达式)
	{
		p_move();
		gram_expression(b, forward_name);
		memset(back_name, 0, sizeof(back_name));
		strcpy(back_name, forward_name);

		if (strcmp(p->code, "RPARENT") != 0)
		{
			error_l(p->before->line);
		}
		p_move();
		*type_addr = 1;
		// printf("<因子>\n");
	}
	else if (gram_integer(&value))
	{
		*type_addr = 1;
		memset(back_name, 0, sizeof(back_name));
		sprintf(back_name, "%d", value);
		// printf("<因子>\n");
	}
	else if (gram_character(&value))
	{
		*type_addr = 2;
		memset(back_name, 0, sizeof(back_name));
		sprintf(back_name, "%d", value);
		// printf("<因子>\n");
	}
	else
	{
		printf("error in gram_factor\n");
	}
}

bool gram_character(int *value)
{
	if (strcmp(p->code, "CHARCON") == 0)
	{
		*value = (int)(p->name[0]);
		p_move();
		return true;
	}
	else
	{
		return false;
	}
}

void gram_step(int *step_addr) // <步长>
{
	if (!gram_unsigned_integer(step_addr))
	{
		printf("error in gram_step!\n");
	}
	// printf("<步长>\n");
}

void gram_value_parameter_table(struct function *target_func) // <值参数表> 
{
	int line = p->line;
	int type;
	int parameter_table[50]; // 记录参数类型
	int parameter_num = 0; // 已读取参数个数
	char forward_name[50];
	char *push_para[100];

	gram_expression(&type, forward_name);
	parameter_table[++parameter_num] = type;
	push_para[parameter_num] = (char *)malloc(sizeof(forward_name) + 3);
	strcpy(push_para[parameter_num], forward_name);
	while (strcmp(p->code, "COMMA") == 0)
	{
		p_move();
		memset(forward_name, 0, sizeof(forward_name));
		gram_expression(&type, forward_name);
		parameter_table[++parameter_num] = type;
		push_para[parameter_num] = (char *)malloc(sizeof(forward_name) + 3);
		strcpy(push_para[parameter_num], forward_name);
	}
	for (int i = 1; i <= parameter_num; i++)
	{
		printf("push %s\n", push_para[i]);
	}
	check_function_parameter(parameter_num, parameter_table, target_func, line);
	// printf("<值参数表>\n");
}

void p_move()
{
	p = p->next;
}

void initialize_pointer()
{
	s = (struct symbol*)malloc(sizeof(struct symbol));
	s_global = s;
	s_local = s_global;
	f = (struct function*)malloc(sizeof(struct function));
	f_head = f;
	c = (struct constant*)malloc(sizeof(struct constant));
	c_head = c;
	b = (int*)malloc(sizeof(int));
}

void fill_symbol_table(int type, bool isarray, bool isfunction, bool isconst) 
// 将标识符填入符号表
{
	struct symbol *q; // 检查本模块内是否有重复定义
	for (q = s_local; q != s; q = q->next)
	{
		if (strcmp(q->name, p->before->name) == 0)
		{
			break;
		}
	}
	if (q == s) // 没有重复定义
	{
		strcpy(s->name, p->before->name); // 注意这里由于预读，填入的是p之前的单词
		s->type = type;
		s->isarray = isarray;
		s->isfunction = isfunction;
		s->isconst = isconst;
		s->next = (struct symbol*)malloc(sizeof(struct symbol));
		s = s->next;
	}
	else // 重复定义
	{
		error_b(p->before->line);
	}
}

void check_symbol_table(int *type_addr) 
// 检查符号表中是否有定义，如果有返回相应的类型
// 注意全局变量与局部变量重名情况！
{
	bool exist = false;
	struct symbol *q;			
	for (q = s_global; q != s; q = q->next)
	{
		if (strcmp(q->name, p->name) == 0)
		{
			*type_addr = q->type;
			exist = true;
		}
	}
	if (exist)
	{
		return;
	}
	struct constant *r;
	for (r = c_head; r != c; r = r->next)
	{
		if (strcmp(r->name, p->name) == 0)
		{
			*type_addr = r->type;
			exist = true;
		}
	}
	if (exist)
	{
		return;
	}
	error_c(p->line);
}

void check_function_parameter(int parameter_num, int *parameter_table, struct function *target_func, int line) 
// 检查参数类型与个数是否匹配
{
	int i;
	if (parameter_num != target_func->para_num)
	{
		error_d(line);
	}
	for (i = 1; i <= parameter_num; i++)
	{
		if (parameter_table[i] != target_func->para_table[i])
		{
			error_e(line);
		}
	}
}

void check_const() // 检查p上一个单词是否为const变量，如果是则报错
{
	struct symbol *q;
	for (q = s_global; q != s; q = q->next)
	{
		if (strcmp(q->name, p->before->name) == 0)
		{
			break;
		}
	}
	if (q != s && q->isconst)
	{
		error_j(p->before->line);
	}
}
