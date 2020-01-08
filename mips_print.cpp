#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "my_mips.h"
#include "my_structure.h"

// 扫描单词相关
char str1[100];
char str2[100];
char str3[100];
char read_line[1000];
struct word *word_head, *p_word;

// 变量存储相关
int fp_offset;
struct object *local_var_head, *local_var_tail, *global_var;

// 常量传播相关
struct object *local_con_head, *local_con_tail, *global_con;

// 寄存器分配相关
int unused_reg = 9; // 将被替换的寄存器号

// 其他
bool is_main;

void mips_pass_constant()
{
	p_word = word_head;
	printf("BOF\n");
	while (strcmp(p_word->name, "func") == 0)
	{
		pc_function();
		while (strcmp(p_word->name, "para") == 0)
		{
			pc_parameter();
		}
		if (strcmp(p_word->name, "const") == 0)
		{
			mips_local_con();
		}
		while (strcmp(p_word->name, "var") == 0)
		{
			if (strcmp(p_word->next->name, "int[]") == 0 ||
				strcmp(p_word->next->name, "char[]") == 0)
			{
				p_word = p_word->next->next->next->next;
			}
			else
			{
				p_word = p_word->next->next->next;
			}
		}
		while (strcmp(p_word->name, "EOF") != 0)
		{
			if (strcmp(p_word->name, "func") == 0)
			{
				break;
			}
			else if (strcmp(p_word->name, "########") == 0)
			{
				pc_beginblock();
			}
			else if (strcmp(p_word->name, "+") == 0 ||
					 strcmp(p_word->name, "-") == 0 || 
					 strcmp(p_word->name, "*") == 0 || 
					 strcmp(p_word->name, "/") == 0 || 
					 strcmp(p_word->name, "==") == 0 || 
					 strcmp(p_word->name, "!=") == 0 || 
					 strcmp(p_word->name, ">=") == 0 || 
					 strcmp(p_word->name, ">") == 0 || 
					 strcmp(p_word->name, "<=") == 0 || 
					 strcmp(p_word->name, "<") == 0)
			{
				pc_calculate();
			}			
			else if (strcmp(p_word->name, "=") == 0)
			{
				pc_assign();
			}
			else if (strcmp(p_word->name, "printf") == 0)
			{
				pc_printf();
			}
			else if (strcmp(p_word->name, "scanf") == 0)
			{
				pc_scanf();
			}
			else if (strcmp(p_word->name, "bt") == 0 ||
					 strcmp(p_word->name, "bnt") == 0)
			{
				pc_branch();
			}
			else if (strcmp(p_word->name, "ret") == 0 ||
					 strcmp(p_word->name, "push") == 0)
			{
				pc_return_or_push();
			}
			else if (strcmp(p_word->name, "[]") == 0)
			{
				pc_from_array();
			}
			else if (strcmp(p_word->name, "{}") == 0)
			{
				pc_to_array();
			}
			else if (strcmp(p_word->name, "call") == 0)
			{
				pc_call_function();
			}
			else if (p_word->name[0] == 'l' && 
				p_word->name[1] == 'a' &&
				p_word->name[2] == 'b' &&
				p_word->name[3] == 'e' &&
				p_word->name[4] == 'l')
			{
				pc_label();
			}
			else if (strcmp(p_word->name, "goto") == 0)
			{
				pc_goto();
			}
			else
			{
				//printf("EEEEEEEEE %s\n", p_word->name);
				printf("error in mips_analysis!\n");
			}
		}
	}
}

void pc_function()
{
	local_con_head = NULL;
	local_con_head = (struct object*)malloc(sizeof(struct object));
	local_con_head->next = NULL;
	local_con_tail = local_con_head;
	local_var_head = NULL;
	local_var_head = (struct object*)malloc(sizeof(struct object));
	local_var_head->next = NULL;
	local_var_tail = local_var_head;
	printf("\n%s %s %s %s\n", p_word->name, p_word->next->name, p_word->next->next->name, p_word->next->next->next->name);
	p_word = p_word->next->next->next->next;
}

void pc_parameter()
{
	while (strcmp(p_word->name, "para") == 0)
	{
		if (strcmp(p_word->next->name, "int[]") == 0 ||
			strcmp(p_word->next->name, "char[]") == 0)
		{
			printf("%s %s %s %s\n", p_word->name, p_word->next->name, p_word->next->next->name, p_word->next->next->next->name);
			p_word = p_word->next->next->next->next;
		}
		else
		{
			printf("%s %s %s\n", p_word->name, p_word->next->name, p_word->next->next->name);
			strcpy(local_var_tail->name, p_word->next->next->name);
			local_var_tail->is_valid = false;
			local_var_tail->next = (struct object*)malloc(sizeof(struct object));
			local_var_tail->next->next = NULL;
			local_var_tail = local_var_tail->next;
			p_word = p_word->next->next->next;
		}
	}
}

void pc_beginblock()
{
	struct object *p;
	for (p = local_var_head; p->next != NULL; p = p->next)
	{
		p->is_valid = false;
	}
	for (p = global_var; p->next != NULL; p = p->next)
	{
		p->is_valid = false;
	}
	memset(p_word->name, 0, sizeof(p_word->name));
	strcpy(p_word->name, p_word->next->name);
	p_word->next = p_word->next->next;
}

void pc_calculate()
{
	char op[50];
	strcpy(op, p_word->name);
	memset(str1, 0, sizeof(str1));
	memset(str2, 0, sizeof(str2));
	memset(str3, 0, sizeof(str3));
	strcpy(str1, p_word->next->name);
	strcpy(str2, p_word->next->next->name);
	strcpy(str3, p_word->next->next->next->name);

	/*
	窥孔优化:
	+ a b @temp1, = @temp1 c → + a b c	
	*/
	if (str3[0] == '@' &&
		strcmp(p_word->next->next->next->next->name, "=") == 0 &&
		strcmp(p_word->next->next->next->next->next->name, str3) == 0)
	{
		p_word->next->next->next = p_word->next->next->next->next->next->next;
	}

	int src1, src2;
	bool b1, b2;
	get_src_constant_value(str1, &b1, &src1);
	get_src_constant_value(str2, &b2, &src2);
	if (b1 && b2)
	{
		int result;
		if (strcmp(op, "+") == 0)
		{
			result = src1 + src2;
		}
		else if (strcmp(op, "-") == 0)
		{
			result = src1 - src2;
		}
		else if (strcmp(op, "*") == 0)
		{
			result = src1 * src2;
		}
		else if (strcmp(op, "/") == 0)
		{
			result = src1 / src2;
		}
		else if (strcmp(op, "==") == 0)
		{
			result = (src1 == src2) ? 1 : 0;
		}
		else if (strcmp(op, "!=") == 0)
		{
			result = (src1 != src2) ? 1 : 0;
		}
		else if (strcmp(op, ">=") == 0)
		{
			result = (src1 >= src2) ? 1 : 0;
		}
		else if (strcmp(op, ">") == 0)
		{
			result = (src1 > src2) ? 1 : 0;
		}
		else if (strcmp(op, "<=") == 0)
		{
			result = (src1 <= src2) ? 1 : 0;
		}
		else if (strcmp(op, "<") == 0)
		{
			result = (src1 < src2) ? 1 : 0;
		}
		store_dst_constant_value(str3, result);
		if (str3[0] == '@')
		{
			memset(p_word->name, 0, sizeof(p_word->name));
			strcpy(p_word->name, p_word->next->next->next->next->name);
			p_word->next = p_word->next->next->next->next->next;
		}
		else
		{
			memset(p_word->name, 0, sizeof(p_word->name));
			strcpy(p_word->name, "=");
			memset(p_word->next->name, 0, sizeof(p_word->next->name));
			sprintf(p_word->next->name, "%d", result);
			p_word->next->next = p_word->next->next->next;
			printf("%s %s %s\n", p_word->name, p_word->next->name, p_word->next->next->name);
			p_word = p_word->next->next->next;
		}
	}
	else
	{
		if (b1)
		{
			memset(p_word->next->name, 0, sizeof(p_word->next->name));
			sprintf(p_word->next->name, "%d", src1);
		}
		if (b2)
		{
			memset(p_word->next->next->name, 0, sizeof(p_word->next->next->name));
			sprintf(p_word->next->next->name, "%d", src2);
		}
		change_dst_constant_value(str3);
		printf("%s %s %s %s\n", p_word->name, p_word->next->name, p_word->next->next->name, p_word->next->next->next->name);
		p_word = p_word->next->next->next->next;
	}
}

void pc_assign()
{
	memset(str1, 0, sizeof(str1));
	memset(str2, 0, sizeof(str2));
	strcpy(str1, p_word->next->name);
	strcpy(str2, p_word->next->next->name);

	int src;
	bool b;
	get_src_constant_value(str1, &b, &src);
	if (b)
	{
		store_dst_constant_value(str2, src);
		if (str2[0] == '@')
		{
			memset(p_word->name, 0, sizeof(p_word->name));
			strcpy(p_word->name, p_word->next->next->next->name);
			p_word->next = p_word->next->next->next->next;
		}
		else
		{
			memset(p_word->next->name, 0, sizeof(p_word->next->name));
			sprintf(p_word->next->name, "%d", src);
			printf("%s %s %s\n", p_word->name, p_word->next->name, p_word->next->next->name);
			p_word = p_word->next->next->next;
		}
	}
	else
	{
		change_dst_constant_value(str2);
		printf("%s %s %s\n", p_word->name, p_word->next->name, p_word->next->next->name);
		p_word = p_word->next->next->next;
	}
}

void pc_printf()
{
	p_word = p_word->next;
	if (strcmp(p_word->name, "%s%d") == 0 || strcmp(p_word->name, "%s%c") == 0)
	{
		bool b;
		int src;
		get_src_constant_value(p_word->next->name, &b, &src);
		if (b)
		{
			memset(p_word->next->name, 0, sizeof(p_word->next->name));
			sprintf(p_word->next->name, "%d", src);
		}
		printf("printf %s %s %s\n", p_word->name, p_word->next->name, p_word->next->next->name);
		p_word = p_word->next->next->next;
	}
	else if (strcmp(p_word->name, "%d") == 0 || strcmp(p_word->name, "%c") == 0)
	{
		bool b;
		int src;
		get_src_constant_value(p_word->next->name, &b, &src);
		if (b)
		{
			memset(p_word->next->name, 0, sizeof(p_word->next->name));
			sprintf(p_word->next->name, "%d", src);
		}
		printf("printf %s %s\n", p_word->name, p_word->next->name);
		p_word = p_word->next->next;
	}
	else if (strcmp(p_word->name, "%s") == 0)
	{
		printf("printf %s %s\n", p_word->name, p_word->next->name);
		p_word = p_word->next->next;
	}
	else
	{
		printf("error in pc_printf!\n");
	}
}

void pc_scanf()
{
	memset(str1, 0, sizeof(str1));
	strcpy(str1, p_word->next->next->name);
	change_dst_constant_value(str1);
	printf("%s %s %s\n", p_word->name, p_word->next->name, p_word->next->next->name);
	p_word = p_word->next->next->next;
}

void pc_branch()
{
	bool b;
	int src;
	get_src_constant_value(p_word->next->name, &b, &src);
	if (b)
	{
		memset(p_word->next->name, 0, sizeof(p_word->next->name));
		sprintf(p_word->next->name, "%d", src);
	}
	printf("%s %s %s\n", p_word->name, p_word->next->name, p_word->next->next->name);
	p_word = p_word->next->next->next;
}

void pc_return_or_push()
{
	bool b;
	int src;
	get_src_constant_value(p_word->next->name, &b, &src);
	if (b)
	{
		memset(p_word->next->name, 0, sizeof(p_word->next->name));
		sprintf(p_word->next->name, "%d", src);
	}
	printf("%s %s\n", p_word->name, p_word->next->name);
	p_word = p_word->next->next;
}

void pc_from_array() // []
{
	memset(str1, 0, sizeof(str1));
	memset(str2, 0, sizeof(str2));
	strcpy(str1, p_word->next->next->name);
	strcpy(str2, p_word->next->next->next->name);
	int src1;
	bool b;
	get_src_constant_value(str1, &b, &src1);
	if (b)
	{
		memset(p_word->next->next->name, 0, sizeof(p_word->next->next->name));
		sprintf(p_word->next->next->name, "%d", src1);
	}
	change_dst_constant_value(str2);
	printf("%s %s %s %s\n", p_word->name, p_word->next->name, p_word->next->next->name, p_word->next->next->next->name);
	p_word = p_word->next->next->next->next;
}

void pc_to_array() // {}
{
	memset(str1, 0, sizeof(str1));
	memset(str2, 0, sizeof(str2));
	strcpy(str1, p_word->next->name);
	strcpy(str2, p_word->next->next->name);
	int src1, src2;
	bool b1, b2;
	get_src_constant_value(str1, &b1, &src1);
	get_src_constant_value(str2, &b2, &src2);
	if (b1)
	{
		memset(p_word->next->name, 0, sizeof(p_word->next->name));
		sprintf(p_word->next->name, "%d", src1);
	}
	if (b2)
	{
		memset(p_word->next->next->name, 0, sizeof(p_word->next->next->name));
		sprintf(p_word->next->next->name, "%d", src2);
	}
	printf("%s %s %s %s\n", p_word->name, p_word->next->name, p_word->next->next->name, p_word->next->next->next->name);
	p_word = p_word->next->next->next->next;
}

void pc_call_function()
{
	change_dst_constant_value(p_word->next->next->name);
	printf("%s %s\n", p_word->name, p_word->next->name);
	p_word = p_word->next->next;

	/*
	窥孔优化：
	@temp1 = RET, = @temp1 a → a = RET
	*/
	if (p_word->name[0] == '@' &&
		strcmp(p_word->next->next->next->name, "=") == 0 &&
		strcmp(p_word->name, p_word->next->next->next->next->name) == 0)
	{
		memset(p_word->name, 0, sizeof(p_word->name));
		strcpy(p_word->name, p_word->next->next->next->next->next->name);
		p_word->next->next->next = p_word->next->next->next->next->next->next;
	}

	printf("%s %s %s\n", p_word->name, p_word->next->name, p_word->next->next->name);
	p_word = p_word->next->next->next;
}

void pc_label()
{
	printf("%s\n", p_word->name);
	p_word = p_word->next;
}

void pc_goto()
{
	printf("%s %s\n", p_word->name, p_word->next->name);
	p_word = p_word->next->next;
}

void get_src_constant_value(char *str, bool *type, int *ret)
{
	struct object *p;
	if (is_num(str, ret))
	{
		*type = true;
	}
	else if (is_local_con(str, &p) || (is_local_var(str, &p) && p->is_valid))
	{
		*type = true;
		*ret = p->value;
	}
	else if (is_local_var(str, &p) && !p->is_valid)
	{
		*type = false;
	}
	else if (is_global_con(str, &p) || (is_global_var(str, &p) && p->is_valid))
	{
		*type = true;
		*ret = p->value;
	}
	else if (is_global_var(str, &p) && !p->is_valid)
	{
		*type = false;
	}
	else
	{
		*type = false;
	}
}

void store_dst_constant_value(char *str, int value)
{
	struct object *p;
	if (is_local_var(str, &p) || is_global_var(str, &p))
	{
		p->is_valid = true;
		p->value = value;
	}
	else
	{
		local_var_tail->is_valid = true;
		local_var_tail->value = value;
		strcpy(local_var_tail->name, str);
		local_var_tail->next = (struct object*)malloc(sizeof(struct object));
		local_var_tail = local_var_tail->next;
		local_var_tail->next = NULL;
	}
}

void change_dst_constant_value(char *str)
{
	struct object *p;
	if (is_local_var(str, &p) || is_global_var(str, &p))
	{
		p->is_valid = false;
	}
}












































void mips_print_head()
{
	while (1)
	{
		memset(read_line, sizeof(read_line), 0);
		fgets(read_line, 900, stdin);
		if (read_line[0] == 'B' && read_line[1] == 'O' && read_line[2] == 'F')
		{
			break;
		}
		else
		{
			printf("%s", read_line);
		}
	}
	printf(".text\n");
	printf("move $fp, $sp\n");
	printf("j func_main\n");
}

void mips_assign_register()
{
	mips_print_head();
	p_word = word_head;
	while (strcmp(p_word->name, "func") == 0)
	{
		int para_num;
		mips_function(&para_num);
		if (para_num != 0)
		{
			mips_func_para(para_num);
		}
		while (strcmp(p_word->name, "const") == 0)
		{
			p_word = p_word->next->next->next->next;
		}
		if (strcmp(p_word->name, "var") == 0)
		{
			mips_local_var();
		}
		while (strcmp(p_word->name, "EOF") != 0 && strcmp(p_word->name, "func") != 0)
		{
			if (strcmp(p_word->name, "+") == 0)
			{
				mips_add();
			}
			else if (strcmp(p_word->name, "-") == 0)
			{
				mips_sub();
			}
			else if (strcmp(p_word->name, "*") == 0)
			{
				mips_mul();
			}
			else if (strcmp(p_word->name, "/") == 0)
			{
				mips_div();
			}
			else if (strcmp(p_word->name, "=") == 0)
			{
				mips_assign();
			}
			else if (strcmp(p_word->name, "==") == 0)
			{
				mips_eq();
			}
			else if (strcmp(p_word->name, "!=") == 0)
			{
				mips_neq();
			}
			else if (strcmp(p_word->name, ">=") == 0)
			{
				mips_ge();
			}
			else if (strcmp(p_word->name, ">") == 0)
			{
				mips_gt();
			}
			else if (strcmp(p_word->name, "<=") == 0)
			{
				mips_le();
			}
			else if (strcmp(p_word->name, "<") == 0)
			{
				mips_lt();
			}
			else if (strcmp(p_word->name, "[]") == 0)
			{
				mips_load_array();
			}
			else if (strcmp(p_word->name, "{}") == 0)
			{
				mips_store_array();
			}
			else if (strcmp(p_word->name, "printf") == 0)
			{
				mips_printf();
			}
			else if (strcmp(p_word->name, "scanf") == 0)
			{
				mips_scanf();
			}
			else if (strcmp(p_word->name, "bt") == 0)
			{
				mips_bt();
			}
			else if (strcmp(p_word->name, "bnt") == 0)
			{
				mips_bnt();
			}
			else if (strcmp(p_word->name, "goto") == 0)
			{
				mips_goto();
			}
			else if (strcmp(p_word->name, "ret") == 0)
			{
				mips_return();
			}
			else if (strcmp(p_word->name, "push") == 0 ||
				strcmp(p_word->name, "call") == 0)
			{
				mips_function_call();
			}
			else if (p_word->name[0] == 'l')
			{
				mips_label();
			}
			else
			{
				printf("EEEEEEEE:%s\n", p_word->name);
				printf("error in mips_analysis!\n");
			}
		}
	}
}

void mips_global_con()
{
	struct object *p = global_con;
	while (strcmp(str1, "const") == 0)
	{
		scanf("%s%s%s", str1, str2, str3);
		if (strcmp(str1, "int") == 0 ||
			strcmp(str1, "char") == 0)
		{
			strcpy(p->name, str2);
			p->is_valid = true;
			p->value = atoi(str3);
		}
		else
		{
			printf("error in mips_global_const()!\n");
		}
		p->next = (struct object*)malloc(sizeof(struct object));
		p = p->next;
		p->next = NULL;
		scanf("%s", str1); // 意味着结束时会多读一个
	}
}

void mips_global_var()
{
	struct object *p = global_var;
	while (strcmp(str1, "var") == 0)
	{
		scanf("%s", str1);
		if (strcmp(str1, "int") == 0 ||
			strcmp(str1, "char") == 0)
		{
			scanf("%s", str2);
			printf("%s: .space 4\n", str2);
		}
		else if (strcmp(str1, "int[]") == 0 ||
			strcmp(str1, "char[]") == 0)
		{
			scanf("%s%s", str2, str3);
			printf("%s: .space %d\n", str2, 4 * atoi(str3));
		}
		else
		{
			printf("error in mips_global_var!\n");
		}

		strcpy(p->name, str2);
		p->is_valid = false;
		p->next = (struct object*)malloc(sizeof(struct object));
		p = p->next;
		p->next = NULL;
		scanf("%s", str1);
	}
}

void mips_function(int *para_num)
{
	p_word = p_word->next->next;
	printf("\nfunc_%s:\n", p_word->name);
	if (strcmp(p_word->name, "main") == 0)
	{
		is_main = true;
	}
	else
	{
		printf("sw $ra, ($fp)\n");
	}
	p_word = p_word->next;
	*para_num = atoi(p_word->name);
	p_word = p_word->next;
	fp_offset = 0;
	unused_reg = 9;
	
	local_var_head = NULL;
	local_var_head = (struct object*)malloc(sizeof(struct object));
	local_var_head->next = NULL;
	local_var_tail = local_var_head;
}

void mips_local_con()
{
	while (strcmp(p_word->name, "const") == 0)
	{
		memset(str1, 0, sizeof(str1));
		memset(str2, 0, sizeof(str2));
		memset(str3, 0, sizeof(str3));

		p_word = p_word->next;
		strcpy(str1, p_word->name);
		p_word = p_word->next;
		strcpy(str2, p_word->name);
		p_word = p_word->next;
		strcpy(str3, p_word->name);
		p_word = p_word->next;

		strcpy(local_con_tail->name, str2);
		if (strcmp(str1, "int") == 0 ||
			strcmp(str1, "char") == 0)
		{
			local_con_tail->value = atoi(str3);
		}
		else
		{
			printf("error in pc_local_con!\n");
		}
		local_con_tail->next = (struct object*)malloc(sizeof(struct object));
		local_con_tail->next->next = NULL;
		local_con_tail = local_con_tail->next;
	}
}

void mips_local_var() // 只处理数组
{
	while (strcmp(p_word->name, "var") == 0)
	{
		p_word = p_word->next;
		if (strcmp(p_word->name, "int[]") == 0 ||
			strcmp(p_word->name, "char[]") == 0)
		{
			p_word = p_word->next;
			strcpy(local_var_tail->name, p_word->name);
			p_word = p_word->next;
			int bound = atoi(p_word->name);
			fp_offset -= 4 * bound;
			local_var_tail->offset = fp_offset;
			local_var_tail->is_in_reg = false;
			local_var_tail->next = (struct object*)malloc(sizeof(struct object));
			local_var_tail = local_var_tail->next;
			local_var_tail->next = NULL;
			p_word = p_word->next;
		}
		else
		{
			p_word = p_word->next->next;
		}
	}
}

void mips_func_para(int para_num)
{
	fp_offset -= 4 * para_num;
	for (int i = 1; i <= para_num; i++)
	{
		p_word = p_word->next->next;
		unused_reg++;
		if (unused_reg <= 25)
		{
			local_var_tail->is_in_reg = true;
			local_var_tail->reg_num = unused_reg;
			printf("lw $%d, %d($fp)\n", unused_reg, -4 * i);
			//printf("变量%s存在寄存器%d\n", p_word->name, unused_reg);
		}
		else
		{
			local_var_tail->is_in_reg = false;
			//printf("变量%s存在内存%d\n", p_word->name, -4*i);
		}
		local_var_tail->offset = -4 * i;
		strcpy(local_var_tail->name, p_word->name);
		local_var_tail->next = (struct object*)malloc(sizeof(struct object));
		local_var_tail = local_var_tail->next;
		local_var_tail->next = NULL;
		p_word = p_word->next;
	}
}

void mips_add()
{
	memset(str1, 0, sizeof(str1));
	memset(str2, 0, sizeof(str2));
	memset(str3, 0, sizeof(str3));

	p_word = p_word->next;
	strcpy(str1, p_word->name);
	p_word = p_word->next;
	strcpy(str2, p_word->name);
	p_word = p_word->next;
	strcpy(str3, p_word->name);
	p_word = p_word->next;

	int src1, src2, dst;
	bool b1, b2;
	int b3;
	get_src_value_or_reg(8, str1, &b1, &src1);
	get_src_value_or_reg(9, str2, &b2, &src2);
	get_dst_reg_or_addr(str3, &b3, &dst);
	if (b3 == 1)
	{
		if (b1 && b2)
		{
			printf("add $%d, $%d, $%d\n", dst, src1, src2);
		}
		else if (!b1 && b2)
		{
			printf("add $%d, $%d, %d\n", dst, src2, src1);
		}
		else if (b1 && !b2)
		{
			printf("add $%d, $%d, %d\n", dst, src1, src2);
		}
		else // 都是常数，表明常量传播有问题！
		{
			printf("error in mips_add!\n");
		}
	}
	else if (b3 == 2)
	{
		if (b1 && b2)
		{
			printf("add $8, $%d, $%d\n", src1, src2);
			printf("sw $8, %d($fp)\n", dst);
		}
		else if (!b1 && b2)
		{
			printf("add $8, $%d, %d\n", src2, src1);
			printf("sw $8, %d($fp)\n", dst);
		}
		else if (b1 && !b2)
		{
			printf("add $8, $%d, %d\n", src1, src2);
			printf("sw $8, %d($fp)\n", dst);
		}
		else
		{
			printf("error in mips_add!\n");
		}
	}
	else if (b3 == 3)
	{
		if (b1 && b2)
		{
			printf("add $8, $%d, $%d\n", src1, src2);
			printf("sw $8, %s\n", str3);
		}
		else if (!b1 && b2)
		{
			printf("add $8, $%d, %d\n", src2, src1);
			printf("sw $8, %s\n", str3);
		}
		else if (b1 && !b2)
		{
			printf("add $8, $%d, %d\n", src1, src2);
			printf("sw $8, %s\n", str3);
		}
		else
		{
			printf("error in mips_add!\n");
		}
	}
}

void mips_sub()
{
	memset(str1, 0, sizeof(str1));
	memset(str2, 0, sizeof(str2));
	memset(str3, 0, sizeof(str3));

	p_word = p_word->next;
	strcpy(str1, p_word->name);
	p_word = p_word->next;
	strcpy(str2, p_word->name);
	p_word = p_word->next;
	strcpy(str3, p_word->name);
	p_word = p_word->next;

	int src1, src2, dst;
	bool b1, b2;
	int b3;
	get_src_value_or_reg(8, str1, &b1, &src1);
	get_src_value_or_reg(9, str2, &b2, &src2);
	get_dst_reg_or_addr(str3, &b3, &dst);
	if (b3 == 1)
	{
		if (b1 && b2)
		{
			printf("sub $%d, $%d, $%d\n", dst, src1, src2);
		}
		else if (!b1 && b2)
		{
			printf("sub $%d, $%d, %d\n", dst, src2, src1);
			printf("sub $%d, $0, $%d\n", dst, dst);
		}
		else if (b1 && !b2)
		{
			printf("sub $%d, $%d, %d\n", dst, src1, src2);
		}
		else
		{
			printf("error in mips_sub!\n");
		}
	}
	else if (b3 == 2)
	{
		if (b1 && b2)
		{
			printf("sub $8, $%d, $%d\n", src1, src2);
			printf("sw $8, %d($fp)\n", dst);
		}
		else if (!b1 && b2)
		{
			printf("sub $8, $%d, %d\n", src2, src1);
			printf("sub $8, $0, $8\n");
			printf("sw $8, %d($fp)\n", dst);
		}
		else if (b1 && !b2)
		{
			printf("sub $8, $%d, %d\n", src1, src2);
			printf("sw $8, %d($fp)\n", dst);
		}
		else
		{
			printf("error in mips_sub!\n");
		}
	}
	else if (b3 == 3)
	{
		if (b1 && b2)
		{
			printf("sub $8, $%d, $%d\n", src1, src2);
			printf("sw $8, %s\n", str3);
		}
		else if (!b1 && b2)
		{
			printf("sub $8, $%d, %d\n", src2, src1);
			printf("sub $8, $0, $8\n");
			printf("sw $8, %s\n", str3);
		}
		else if (b1 && !b2)
		{
			printf("sub $8, $%d, %d\n", src1, src2);
			printf("sw $8, %s\n", str3);
		}
		else
		{
			printf("error in mips_sub!\n");
		}
	}
}

void mips_mul()
{
	memset(str1, 0, sizeof(str1));
	memset(str2, 0, sizeof(str2));
	memset(str3, 0, sizeof(str3));

	p_word = p_word->next;
	strcpy(str1, p_word->name);
	p_word = p_word->next;
	strcpy(str2, p_word->name);
	p_word = p_word->next;
	strcpy(str3, p_word->name);
	p_word = p_word->next;

	int src1, src2, dst;
	bool b1, b2;
	int b3;
	get_src_value_or_reg(8, str1, &b1, &src1);
	get_src_value_or_reg(9, str2, &b2, &src2);
	get_dst_reg_or_addr(str3, &b3, &dst);
	if (b1 && b2)
	{
		printf("mult $%d, $%d\n", src1, src2);
	}
	else if (!b1 && b2)
	{
		printf("li $8, %d\n", src1);
		printf("mult $%d, $8\n", src2);
	}
	else if (b1 && !b2)
	{
		printf("li $9, %d\n", src2);
		printf("mult $%d, $9\n", src1);
	}
	else
	{
		printf("error in mips_mult!\n");
	}

	if (b3 == 1)
	{
		printf("mflo $%d\n", dst);
	}
	else if (b3 == 2)
	{
		printf("mflo $8\n");
		printf("sw $8, %d($fp)\n", dst);
	}
	else if (b3 == 3)
	{
		printf("mflo $8\n");
		printf("sw $8, %s\n", str3);
	}
}

void mips_div()
{
	memset(str1, 0, sizeof(str1));
	memset(str2, 0, sizeof(str2));
	memset(str3, 0, sizeof(str3));

	p_word = p_word->next;
	strcpy(str1, p_word->name);
	p_word = p_word->next;
	strcpy(str2, p_word->name);
	p_word = p_word->next;
	strcpy(str3, p_word->name);
	p_word = p_word->next;

	int src1, src2, dst;
	bool b1, b2;
	int b3;
	get_src_value_or_reg(8, str1, &b1, &src1);
	get_src_value_or_reg(9, str2, &b2, &src2);
	get_dst_reg_or_addr(str3, &b3, &dst);
	if (b1 && b2)
	{
		printf("div $%d, $%d\n", src1, src2);
	}
	else if (!b1 && b2)
	{
		printf("li $8, %d\n", src1);
		printf("div $%d, $8\n", src2);
	}
	else if (b1 && !b2)
	{
		printf("li $9, %d\n", src2);
		printf("div $%d, $9\n", src1);
	}
	else
	{
		printf("error in mips_mult!\n");
	}

	if (b3 == 1)
	{
		printf("mflo $%d\n", dst);
	}
	else if (b3 == 2)
	{
		printf("mflo $8\n");
		printf("sw $8, %d($fp)\n", dst);
	}
	else if (b3 == 3)
	{
		printf("mflo $8\n");
		printf("sw $8, %s\n", str3);
	}
}

void mips_assign()
{
	memset(str1, 0, sizeof(str1));
	memset(str2, 0, sizeof(str2));

	p_word = p_word->next;
	strcpy(str1, p_word->name);
	p_word = p_word->next;
	strcpy(str2, p_word->name);
	p_word = p_word->next;

	int src, dst;
	bool b1;
	int b2;
	get_src_value_or_reg(8, str1, &b1, &src);
	get_dst_reg_or_addr(str2, &b2, &dst);
	if (b2 == 1)
	{
		if (b1)
		{
			printf("move $%d, $%d\n", dst, src);
		}
		else
		{
			printf("li $%d, %d\n", dst, src);
		}
	}
	else if (b2 == 2)
	{
		if (b1)
		{
			printf("sw $%d, %d($fp)\n", src, dst);
		}
		else
		{
			printf("li $8, %d\n", src);
			printf("sw $8, %d($fp)\n", dst);
		}
	}
	else if (b2 == 3)
	{
		if (b1)
		{
			printf("sw $%d, %s\n", src, str2);
		}
		else
		{
			printf("li $8, %d\n", src);
			printf("sw $8, %s\n", str2);
		}
	}
}

void mips_eq() // ==
{
	memset(str1, 0, sizeof(str1));
	memset(str2, 0, sizeof(str2));
	memset(str3, 0, sizeof(str3));

	p_word = p_word->next;
	strcpy(str1, p_word->name);
	p_word = p_word->next;
	strcpy(str2, p_word->name);
	p_word = p_word->next;
	strcpy(str3, p_word->name);
	p_word = p_word->next;

	int src1, src2, dst;
	bool b1, b2;
	int b3;
	get_src_value_or_reg(8, str1, &b1, &src1);
	get_src_value_or_reg(9, str2, &b2, &src2);
	get_dst_reg_or_addr(str3, &b3, &dst);
	if (b3 == 1)
	{
		if (b1 && b2)
		{
			printf("seq $%d, $%d, $%d\n", dst, src1, src2);
		}
		else if (!b1 && b2)
		{
			printf("seq $%d, $%d, %d\n", dst, src2, src1);
		}
		else if (b1 && !b2)
		{
			printf("seq $%d, $%d, %d\n", dst, src1, src2);
		}
		else
		{
			printf("error in mips_eq!\n");
		}
	}
	else if (b3 == 2)
	{
		if (b1 && b2)
		{
			printf("seq $8, $%d, $%d\n", src1, src2);
			printf("sw $8, %d($fp)\n", dst);
		}
		else if (!b1 && b2)
		{
			printf("seq $8, $%d, %d\n", src2, src1);
			printf("sw $8, %d($fp)\n", dst);
		}
		else if (b1 && !b2)
		{
			printf("seq $8, $%d, %d\n", src1, src2);
			printf("sw $8, %d($fp)\n", dst);
		}
		else
		{
			printf("error in mips_eq!\n");
		}
	}
	else if (b3 == 3)
	{
		if (b1 && b2)
		{
			printf("seq $8, $%d, $%d\n", src1, src2);
			printf("sw $8, %s\n", str3);
		}
		else if (!b1 && b2)
		{
			printf("seq $8, $%d, %d\n", src2, src1);
			printf("sw $8, %s\n", str3);
		}
		else if (b1 && !b2)
		{
			printf("seq $8, $%d, %d\n", src1, src2);
			printf("sw $8, %s\n", str3);
		}
		else
		{
			printf("error in mips_eq!\n");
		}
	}
}

void mips_neq() // !=
{
	memset(str1, 0, sizeof(str1));
	memset(str2, 0, sizeof(str2));
	memset(str3, 0, sizeof(str3));

	p_word = p_word->next;
	strcpy(str1, p_word->name);
	p_word = p_word->next;
	strcpy(str2, p_word->name);
	p_word = p_word->next;
	strcpy(str3, p_word->name);
	p_word = p_word->next;

	int src1, src2, dst;
	bool b1, b2;
	int b3;
	get_src_value_or_reg(8, str1, &b1, &src1);
	get_src_value_or_reg(9, str2, &b2, &src2);
	get_dst_reg_or_addr(str3, &b3, &dst);
	if (b3 == 1)
	{
		if (b1 && b2)
		{
			printf("sne $%d, $%d, $%d\n", dst, src1, src2);
		}
		else if (!b1 && b2)
		{
			printf("sne $%d, $%d, %d\n", dst, src2, src1);
		}
		else if (b1 && !b2)
		{
			printf("sne $%d, $%d, %d\n", dst, src1, src2);
		}
		else
		{
			printf("error in mips_eq!\n");
		}
	}
	else if (b3 == 2)
	{
		if (b1 && b2)
		{
			printf("sne $8, $%d, $%d\n", src1, src2);
			printf("sw $8, %d($fp)\n", dst);
		}
		else if (!b1 && b2)
		{
			printf("sne $8, $%d, %d\n", src2, src1);
			printf("sw $8, %d($fp)\n", dst);
		}
		else if (b1 && !b2)
		{
			printf("sne $8, $%d, %d\n", src1, src2);
			printf("sw $8, %d($fp)\n", dst);
		}
		else
		{
			printf("error in mips_eq!\n");
		}
	}
	else if (b3 == 3)
	{
		if (b1 && b2)
		{
			printf("sne $8, $%d, $%d\n", src1, src2);
			printf("sw $8, %s\n", str3);
		}
		else if (!b1 && b2)
		{
			printf("sne $8, $%d, %d\n", src2, src1);
			printf("sw $8, %s\n", str3);
		}
		else if (b1 && !b2)
		{
			printf("sne $8, $%d, %d\n", src1, src2);
			printf("sw $8, %s\n", str3);
		}
		else
		{
			printf("error in mips_eq!\n");
		}
	}
}

void mips_ge() // >=
{
	memset(str1, 0, sizeof(str1));
	memset(str2, 0, sizeof(str2));
	memset(str3, 0, sizeof(str3));

	p_word = p_word->next;
	strcpy(str1, p_word->name);
	p_word = p_word->next;
	strcpy(str2, p_word->name);
	p_word = p_word->next;
	strcpy(str3, p_word->name);
	p_word = p_word->next;

	int src1, src2, dst;
	bool b1, b2;
	int b3;
	get_src_value_or_reg(8, str1, &b1, &src1);
	get_src_value_or_reg(9, str2, &b2, &src2);
	get_dst_reg_or_addr(str3, &b3, &dst);
	if (b3 == 1)
	{
		if (b1 && b2)
		{
			printf("sge $%d, $%d, $%d\n", dst, src1, src2);
		}
		else if (!b1 && b2)
		{
			printf("sle $%d, $%d, %d\n", dst, src2, src1);
		}
		else if (b1 && !b2)
		{
			printf("sge $%d, $%d, %d\n", dst, src1, src2);
		}
		else
		{
			printf("error in mips_eq!\n");
		}
	}
	else if (b3 == 2)
	{
		if (b1 && b2)
		{
			printf("sge $8, $%d, $%d\n", src1, src2);
			printf("sw $8, %d($fp)\n", dst);
		}
		else if (!b1 && b2)
		{
			printf("sle $8, $%d, %d\n", src2, src1);
			printf("sw $8, %d($fp)\n", dst);
		}
		else if (b1 && !b2)
		{
			printf("sge $8, $%d, %d\n", src1, src2);
			printf("sw $8, %d($fp)\n", dst);
		}
		else
		{
			printf("error in mips_eq!\n");
		}
	}
	else if (b3 == 3)
	{
		if (b1 && b2)
		{
			printf("sge $8, $%d, $%d\n", src1, src2);
			printf("sw $8, %s\n", str3);
		}
		else if (!b1 && b2)
		{
			printf("sle $8, $%d, %d\n", src2, src1);
			printf("sw $8, %s\n", str3);
		}
		else if (b1 && !b2)
		{
			printf("sge $8, $%d, %d\n", src1, src2);
			printf("sw $8, %s\n", str3);
		}
		else
		{
			printf("error in mips_eq!\n");
		}
	}
}

void mips_gt() // >
{
	memset(str1, 0, sizeof(str1));
	memset(str2, 0, sizeof(str2));
	memset(str3, 0, sizeof(str3));

	p_word = p_word->next;
	strcpy(str1, p_word->name);
	p_word = p_word->next;
	strcpy(str2, p_word->name);
	p_word = p_word->next;
	strcpy(str3, p_word->name);
	p_word = p_word->next;

	int src1, src2, dst;
	bool b1, b2;
	int b3;
	get_src_value_or_reg(8, str1, &b1, &src1);
	get_src_value_or_reg(9, str2, &b2, &src2);
	get_dst_reg_or_addr(str3, &b3, &dst);
	if (b3 == 1)
	{
		if (b1 && b2)
		{
			printf("sgt $%d, $%d, $%d\n", dst, src1, src2);
		}
		else if (!b1 && b2)
		{
			printf("slti $%d, $%d, %d\n", dst, src2, src1);
		}
		else if (b1 && !b2)
		{
			printf("sgt $%d, $%d, %d\n", dst, src1, src2);
		}
		else
		{
			printf("error in mips_eq!\n");
		}
	}
	else if (b3 == 2)
	{
		if (b1 && b2)
		{
			printf("sgt $8, $%d, $%d\n", src1, src2);
			printf("sw $8, %d($fp)\n", dst);
		}
		else if (!b1 && b2)
		{
			printf("slti $8, $%d, %d\n", src2, src1);
			printf("sw $8, %d($fp)\n", dst);
		}
		else if (b1 && !b2)
		{
			printf("sgt $8, $%d, %d\n", src1, src2);
			printf("sw $8, %d($fp)\n", dst);
		}
		else
		{
			printf("error in mips_eq!\n");
		}
	}
	else if (b3 == 3)
	{
		if (b1 && b2)
		{
			printf("sgt $8, $%d, $%d\n", src1, src2);
			printf("sw $8, %s\n", str3);
		}
		else if (!b1 && b2)
		{
			printf("slti $8, $%d, %d\n", src2, src1);
			printf("sw $8, %s\n", str3);
		}
		else if (b1 && !b2)
		{
			printf("sgt $8, $%d, %d\n", src1, src2);
			printf("sw $8, %s\n", str3);
		}
		else
		{
			printf("error in mips_eq!\n");
		}
	}
}

void mips_le() // <=
{
	memset(str1, 0, sizeof(str1));
	memset(str2, 0, sizeof(str2));
	memset(str3, 0, sizeof(str3));

	p_word = p_word->next;
	strcpy(str1, p_word->name);
	p_word = p_word->next;
	strcpy(str2, p_word->name);
	p_word = p_word->next;
	strcpy(str3, p_word->name);
	p_word = p_word->next;

	int src1, src2, dst;
	bool b1, b2;
	int b3;
	get_src_value_or_reg(8, str1, &b1, &src1);
	get_src_value_or_reg(9, str2, &b2, &src2);
	get_dst_reg_or_addr(str3, &b3, &dst);
	if (b3 == 1)
	{
		if (b1 && b2)
		{
			printf("sle $%d, $%d, $%d\n", dst, src1, src2);
		}
		else if (!b1 && b2)
		{
			printf("sge $%d, $%d, %d\n", dst, src2, src1);
		}
		else if (b1 && !b2)
		{
			printf("sle $%d, $%d, %d\n", dst, src1, src2);
		}
		else
		{
			printf("error in mips_eq!\n");
		}
	}
	else if (b3 == 2)
	{
		if (b1 && b2)
		{
			printf("sle $8, $%d, $%d\n", src1, src2);
			printf("sw $8, %d($fp)\n", dst);
		}
		else if (!b1 && b2)
		{
			printf("sge $8, $%d, %d\n", src2, src1);
			printf("sw $8, %d($fp)\n", dst);
		}
		else if (b1 && !b2)
		{
			printf("sle $8, $%d, %d\n", src1, src2);
			printf("sw $8, %d($fp)\n", dst);
		}
		else
		{
			printf("error in mips_eq!\n");
		}
	}
	else if (b3 == 3)
	{
		if (b1 && b2)
		{
			printf("sle $8, $%d, $%d\n", src1, src2);
			printf("sw $8, %s\n", str3);
		}
		else if (!b1 && b2)
		{
			printf("sge $8, $%d, %d\n", src2, src1);
			printf("sw $8, %s\n", str3);
		}
		else if (b1 && !b2)
		{
			printf("sle $8, $%d, %d\n", src1, src2);
			printf("sw $8, %s\n", str3);
		}
		else
		{
			printf("error in mips_eq!\n");
		}
	}
}

void mips_lt() // <
{
	memset(str1, 0, sizeof(str1));
	memset(str2, 0, sizeof(str2));
	memset(str3, 0, sizeof(str3));

	p_word = p_word->next;
	strcpy(str1, p_word->name);
	p_word = p_word->next;
	strcpy(str2, p_word->name);
	p_word = p_word->next;
	strcpy(str3, p_word->name);
	p_word = p_word->next;

	int src1, src2, dst;
	bool b1, b2;
	int b3;
	get_src_value_or_reg(8, str1, &b1, &src1);
	get_src_value_or_reg(9, str2, &b2, &src2);
	get_dst_reg_or_addr(str3, &b3, &dst);
	if (b3 == 1)
	{
		if (b1 && b2)
		{
			printf("slt $%d, $%d, $%d\n", dst, src1, src2);
		}
		else if (!b1 && b2)
		{
			printf("sgt $%d, $%d, %d\n", dst, src2, src1);
		}
		else if (b1 && !b2)
		{
			printf("slti $%d, $%d, %d\n", dst, src1, src2);
		}
		else
		{
			printf("error in mips_eq!\n");
		}
	}
	else if (b3 == 2)
	{
		if (b1 && b2)
		{
			printf("slt $8, $%d, $%d\n", src1, src2);
			printf("sw $8, %d($fp)\n", dst);
		}
		else if (!b1 && b2)
		{
			printf("sgt $8, $%d, %d\n", src2, src1);
			printf("sw $8, %d($fp)\n", dst);
		}
		else if (b1 && !b2)
		{
			printf("slti $8, $%d, %d\n", src1, src2);
			printf("sw $8, %d($fp)\n", dst);
		}
		else
		{
			printf("error in mips_eq!\n");
		}
	}
	else if (b3 == 3)
	{
		if (b1 && b2)
		{
			printf("slt $8, $%d, $%d\n", src1, src2);
			printf("sw $8, %s\n", str3);
		}
		else if (!b1 && b2)
		{
			printf("sgt $8, $%d, %d\n", src2, src1);
			printf("sw $8, %s\n", str3);
		}
		else if (b1 && !b2)
		{
			printf("slti $8, $%d, %d\n", src1, src2);
			printf("sw $8, %s\n", str3);
		}
		else
		{
			printf("error in mips_eq!\n");
		}
	}
}

void mips_load_array() // []
{
	memset(str1, 0, sizeof(str1));
	memset(str2, 0, sizeof(str2));
	memset(str3, 0, sizeof(str3));

	p_word = p_word->next;
	strcpy(str1, p_word->name);
	p_word = p_word->next;
	strcpy(str2, p_word->name);
	p_word = p_word->next;
	strcpy(str3, p_word->name);
	p_word = p_word->next;
	// [] str1 str2 str3 → str3 = str1[str2];
	
	int src1, src2, dst;
	bool b1, b2;
	int b3;
	get_array_offset(str1, &b1, &src1);
	get_src_value_or_reg(8, str2, &b2, &src2);
	get_dst_reg_or_addr(str3, &b3, &dst);
	if (b3 == 1)
	{
		if (b2)
		{
			if (b1)
			{
				printf("sll $8, $%d, 2\n", src2);
				printf("add $8, $8, $fp\n");
				printf("lw $%d, %d($8)\n", dst, src1);
			}
			else
			{
				printf("sll $8, $%d, 2\n", src2);
				printf("lw $%d, %s($8)\n", dst, str1);
			}
		}
		else
		{
			if (b1)
			{
				printf("lw $%d, %d($fp)\n", dst, src1 + src2 * 4);
			}
			else
			{
				printf("lw $%d, %s + %d\n", dst, str1, src2 * 4);
			}
		}
	}
	else if (b3 == 2)
	{
		if (b2)
		{
			if (b1)
			{
				printf("sll $8, $%d, 2\n", src2);
				printf("add $8, $8, $fp\n");
				printf("lw $8, %d($8)\n", src1);
				printf("sw $8, %d($fp)\n", dst);
			}
			else
			{
				printf("sll $8, $%d, 2\n", src2);
				printf("lw $8, %s($8)\n", str1);
				printf("sw $8, %d($fp)\n", dst);
			}
		}
		else
		{
			if (b1)
			{
				printf("lw $8, %d($fp)\n", src1 + src2 * 4);
				printf("sw $8, %d($fp)\n", dst);
			}
			else
			{
				printf("lw $8, %s + %d\n", str1, src2 * 4);
				printf("sw $8, %d($fp)\n", dst);
			}
		}
	}
	else if (b3 == 3)
	{
		if (b2)
		{
			if (b1)
			{
				printf("sll $8, $%d, 2\n", src2);
				printf("add $8, $8, $fp\n");
				printf("lw $8, %d($8)\n", src1);
				printf("sw $8, %s\n", str3);
			}
			else
			{
				printf("sll $8, $%d, 2\n", src2);
				printf("lw $8, %s($8)\n", str1);
				printf("sw $8, %s\n", str3);
			}
		}
		else
		{
			if (b1)
			{
				printf("lw $8, %d($fp)\n", src1 + src2 * 4);
				printf("sw $8, %s\n", str3);
			}
			else
			{
				printf("lw $8, %s + %d\n", str1, src2 * 4);
				printf("sw $8, %s\n", str3);
			}
		}
	}	
}

void mips_store_array() // {}
{
	memset(str1, 0, sizeof(str1));
	memset(str2, 0, sizeof(str2));
	memset(str3, 0, sizeof(str3));

	p_word = p_word->next;
	strcpy(str1, p_word->name);
	p_word = p_word->next;
	strcpy(str2, p_word->name);
	p_word = p_word->next;
	strcpy(str3, p_word->name);
	p_word = p_word->next;	
	// {} str1 str2 str3 → str3[str1] = str2

	int src1, src2, dst;
	bool b1, b2, b3;
	get_array_offset(str3, &b3, &dst);
	get_src_value_or_reg(8, str1, &b1, &src1);
	get_src_value_or_reg(9, str2, &b2, &src2);
	if (b3)
	{
		if (b2)
		{
			if (b1)
			{
				printf("sll $8, $%d, 2\n", src1);
				printf("add $8, $8, $fp\n");
				printf("sw $%d, %d($8)\n", src2, dst);
			}
			else
			{
				printf("sw $%d, %d($fp)\n", src2, dst + src1 * 4);
			}
		}
		else
		{
			if (b1)
			{
				printf("li $8, %d\n", src2);
				printf("sll $9, $%d, 2\n", src1);
				printf("add $9, $9, $fp\n");
				printf("sw $8, %d($9)\n", dst);
			}
			else
			{
				printf("li $8, %d\n", src2);
				printf("sw $8, %d($fp)\n", dst + src1 * 4);
			}
		}
	}
	else
	{
		if (b2)
		{
			if (b1)
			{
				printf("sll $8, $%d, 2\n", src1);
				printf("sw $%d, %s($8)\n", src2, str3);
			}
			else
			{
				printf("sw $%d, %s + %d\n", src2, str3, src1 * 4);
			}
		}
		else
		{
			if (b1)
			{
				printf("li $8, %d\n", src2);
				printf("sll $9, $%d, 2\n", src1);
				printf("sw $8, %s($9)\n", str3);
			}
			else
			{
				printf("li $8, %d\n", src2);
				printf("sw $8, %s + %d\n", str3, src1 * 4);
			}
		}
	}
}

void mips_printf()
{
	p_word = p_word->next;
	if (strcmp(p_word->name, "%s%d") == 0)
	{
		memset(str1, 0, sizeof(str1));
		memset(str2, 0, sizeof(str2));
		strcpy(str1, p_word->next->name);
		strcpy(str2, p_word->next->next->name);
		p_word = p_word->next->next->next;

		printf("li $v0, 4\n");
		printf("la $a0, %s\n", str2);
		printf("syscall\n");
		bool b;
		int src;
		get_src_value_or_reg(8, str1, &b, &src);
		if (b)
		{
			printf("move $a0, $%d\n", src);		
		}		
		else
		{
			printf("li $a0, %d\n", src);
		}
		printf("li $v0, 1\n");
		printf("syscall\n");
	}
	else if (strcmp(p_word->name, "%s%c") == 0)
	{
		memset(str1, 0, sizeof(str1));
		memset(str2, 0, sizeof(str2));
		strcpy(str1, p_word->next->name);
		strcpy(str2, p_word->next->next->name);
		p_word = p_word->next->next->next;

		printf("li $v0, 4\n");
		printf("la $a0, %s\n", str2);
		printf("syscall\n");
		bool b;
		int src;
		get_src_value_or_reg(8, str1, &b, &src);
		if (b)
		{
			printf("move $a0, $%d\n", src);
		}
		else
		{
			printf("li $a0, %d\n", src);
		}
		printf("li $v0, 11\n");
		printf("syscall\n");
	}
	else if (strcmp(p_word->name, "%s") == 0)
	{
		p_word = p_word->next;
		printf("li $v0, 4\n");
		printf("la $a0, %s\n", p_word->name);
		printf("syscall\n");
		p_word = p_word->next;
	}
	else if (strcmp(p_word->name, "%d") == 0)
	{
		memset(str1, 0, sizeof(str1));
		strcpy(str1, p_word->next->name);
		bool b;
		int src;
		get_src_value_or_reg(8, str1, &b, &src);
		if (b)
		{
			printf("move $a0, $%d\n", src);
		}
		else
		{
			printf("li $a0, %d\n", src);
		}
		printf("li $v0, 1\n");
		printf("syscall\n");
		p_word = p_word->next->next;
	}
	else if (strcmp(p_word->name, "%c") == 0)
	{
		memset(str1, 0, sizeof(str1));
		strcpy(str1, p_word->next->name);
		bool b;
		int src;
		get_src_value_or_reg(8, str1, &b, &src);
		if (b)
		{
			printf("move $a0, $%d\n", src);
		}
		else
		{
			printf("li $a0, %d\n", src);
		}
		printf("li $v0, 11\n");
		printf("syscall\n");
		p_word = p_word->next->next;
	}
	else
	{
		printf("error in mips_printf!\n");
	}
	// printf需要自带换行符
	printf("li $a0, 10\n");
	printf("li $v0, 11\n");
	printf("syscall\n");
}

void mips_scanf()
{
	p_word = p_word->next;
	if (strcmp(p_word->name, "%d") == 0)
	{
		printf("li $v0, 5\n");
	}
	else if (strcmp(p_word->name, "%c") == 0)
	{
		printf("li $v0, 12\n");
	}
	printf("syscall\n");
	p_word = p_word->next;
	int dst;
	int b;
	get_dst_reg_or_addr(p_word->name, &b, &dst);
	if (b == 1)
	{
		printf("move $%d, $v0\n", dst);
	}
	else if (b == 2)
	{
		printf("sw $v0, %d($fp)\n", dst);
	}
	else if (b == 3)
	{
		printf("sw $v0, %s\n", p_word->name);
	}
	p_word = p_word->next;
}

void mips_bt()
{
	memset(str1, 0, sizeof(str1));
	memset(str2, 0, sizeof(str2));

	p_word = p_word->next;
	strcpy(str1, p_word->name);
	p_word = p_word->next;
	strcpy(str2, p_word->name);
	p_word = p_word->next;

	bool b;
	int src;
	get_src_value_or_reg(8, str1, &b, &src);
	if (b)
	{
		printf("bne $0, $%d, %s\n", src, str2);
	}
	else
	{
		printf("bne $0, %d, %s\n", src, str2);
	}
}

void mips_bnt()
{
	memset(str1, 0, sizeof(str1));
	memset(str2, 0, sizeof(str2));

	p_word = p_word->next;
	strcpy(str1, p_word->name);
	p_word = p_word->next;
	strcpy(str2, p_word->name);
	p_word = p_word->next;

	bool b;
	int src;
	get_src_value_or_reg(8, str1, &b, &src);
	if (b)
	{
		printf("beq $0, $%d, %s\n", src, str2);
	}
	else
	{
		printf("beq $0, %d, %s\n", src, str2);
	}
}

void mips_goto()
{
	printf("j %s\n", p_word->next->name);
	p_word = p_word->next->next;
}

void mips_return()
{
	if (is_main)
	{
		printf("li $v0, 10\n");
		printf("syscall\n");
		p_word = p_word->next->next;
	}
	else
	{
		p_word = p_word->next;
		if (strcmp(p_word->name, "void") != 0)
		{
			int a;
			bool b;
			get_src_value_or_reg(8, p_word->name, &b, &a);
			if (b)
			{
				printf("move $v0, $%d\n", a);
			}
			else
			{
				printf("li $v0, %d\n", a);
			}
		}
		printf("lw $ra, ($fp)\n");
		printf("jr $ra\n");
		p_word = p_word->next;
	}
}

void mips_function_call()
{
	printf("add $sp, $fp, %d\n", fp_offset);
	printf("add $sp, $sp, -4\n");
	printf("sw $fp, 0($sp)\n");
	printf("add $sp, $sp, -4\n");
	int cnt = 0;
	while (strcmp(p_word->name, "push") == 0)
	{
		cnt++;
		p_word = p_word->next;
		printf("add $sp, $sp, -4\n");
		int a;
		bool b;
		get_src_value_or_reg(8, p_word->name, &b, &a);
		if (b)
		{
			printf("sw $%d, 0($sp)\n", a);
		}
		else
		{
			printf("li $8, %d\n", a);
			printf("sw $8, 0($sp)\n");
		}
		p_word = p_word->next;
	}
	// 所有寄存器存入内存
	struct object *p, *reg2var[30];
	for (int i = 8; i <= 25; i++)
	{
		reg2var[i] = NULL;
	}
	for (p = local_var_head; p->next != NULL; p = p->next)
	{
		if (p->is_in_reg)
		{
			printf("sw $%d, %d($fp)\n", p->reg_num, p->offset);
			reg2var[p->reg_num] = p;
		}
	}
	printf("add $fp, $sp, %d\n", 4 * cnt);
	p_word = p_word->next;
	printf("jal func_%s\n", p_word->name);
	printf("lw $fp, 4($fp)\n"); // 恢复$fp
	// 恢复寄存器
	p = local_var_head;
	for (int i = 8; i <= 25; i++)
	{
		if (reg2var[i] != NULL)
		{
			printf("lw $%d, %d($fp)\n", reg2var[i]->reg_num, reg2var[i]->offset);
		}
	}
	p_word = p_word->next;
	int dst, b;
	if (strcmp(p_word->name, "void") != 0)
	{
		get_dst_reg_or_addr(p_word->name, &b, &dst);
		if (b == 1)
		{
			printf("move $%d, $v0\n", dst);
		}
		else if (b == 2)
		{
			printf("sw $v0, %d($fp)\n", dst);
		}
		else if (b == 3)
		{
			printf("sw $v0, %s\n", p_word->name);
		}
	}
	p_word = p_word->next->next->next;
}

void mips_label()
{
	printf("%s\n", p_word->name);
	p_word = p_word->next;
}

void mips_initialize_pointer()
{
	global_var = (struct object*)malloc(sizeof(struct object));
	global_var->next = NULL;
	global_con = (struct object*)malloc(sizeof(struct object));
	global_con->next = NULL;
	local_var_head = (struct object*)malloc(sizeof(struct object));
	local_var_head->next = NULL;
	local_var_tail = local_var_head;
	local_con_head = (struct object*)malloc(sizeof(struct object));
	local_con_head->next = NULL;
	local_con_tail = local_con_head;
	word_head = (struct word*)malloc(sizeof(struct word));
	word_head->next = NULL;
	p_word = word_head;
}

void mips_store_data()
{
	struct object *p;
	printf(".data\n");
	scanf("%s", str1);
	if (strcmp(str1, "const") == 0) // 全局常量
	{
		mips_global_con();
	}
	if (strcmp(str1, "var") == 0) // 全局变量
	{
		mips_global_var();
	}
	scanf("%s", str1); // 跳过第一个########
	int str_cnt = 0;
	while (1)
	{
		if (strcmp(str1, "EOF") == 0)
		{
			strcpy(p_word->name, "EOF");
			p_word->next = (struct word*)malloc(sizeof(struct word));
			p_word = p_word->next;
			p_word->next = NULL;
			break;
		}
		if (str1[0] == '\"')
		{
			fgets(str2, 100, stdin);
			strcat(str1, str2);
			// 处理转义
			for (int i = 0, j = 0; i < strlen(str1); i++, j++)
			{
				if (str1[i] == '\\')
				{
					str2[j++] = '\\';
					str2[j] = '\\';
				}
				else
				{
					str2[j] = str1[i];
				}
			}
			printf("string%d: .asciiz %s\n", ++str_cnt, str2); // 首尾已经有双引号		
			char temp1[10];
			sprintf(temp1, "%d", str_cnt);
			strcpy(p_word->name, "string");
			strcat(p_word->name, temp1);
		}
		else
		{
			strcpy(p_word->name, str1);
		}
		p_word->next = (struct word*)malloc(sizeof(struct word));
		p_word = p_word->next;
		p_word->next = NULL;
		memset(str1, 0, sizeof(str1));
		memset(str2, 0, sizeof(str2));
		scanf("%s", str1);
	}
}

bool is_num(char *str, int *value)
{
	if (!isdigit(str[0]) && str[0] != '-')
	{
		return false;
	}
	for (int i = 1; i < strlen(str); i++)
	{
		if (!isdigit(str[i]))
		{
			return false;
		}
	}
	*value = atoi(str);
	return true;
}

bool is_local_con(char *str, struct object **ret)
{
	struct object *p;
	for (p = local_con_head; p->next != NULL; p = p->next)
	{
		if (strcmp(p->name, str) == 0)
		{
			*ret = p;
			return true;
		}
	}
	return false;
}

bool is_local_var(char *str, struct object **ret)
{
	struct object *p;
	for (p = local_var_head; p->next != NULL; p = p->next)
	{

		if (strcmp(p->name, str) == 0)
		{
			*ret = p;
			return true;
		}
	}
	return false;
	// 表示未定义：许多中间变量（比如t1）都没有定义过
}

bool is_global_con(char *str, struct object **ret)
{
	struct object *p;
	for (p = global_con; p->next != NULL; p = p->next)
	{
		if (strcmp(p->name, str) == 0)
		{
			*ret = p;
			return true;
		}
	}
	return false;
}

bool is_global_var(char *str, struct object **ret)
{
	struct object *p;
	for (p = global_var; p->next != NULL; p = p->next)
	{
		if (strcmp(p->name, str) == 0)
		{
			*ret = p;
			return true;
		}
	}
	return false;
	// 表示不是全局变量
}


// type = true:返回寄存器编号
// type = false:常量,返回值
// usable是可以使用的临时寄存器编号
void get_src_value_or_reg(int usable, char *str, bool *type, int *ret)
{
	struct object *p;
	if (is_num(str, ret))
	{
		*type = false;
	}
	else if (is_local_var(str, &p) && p->is_in_reg)
	{
		*type = true;
		*ret = p->reg_num;
	}
	else if (is_local_var(str, &p) && !p->is_in_reg)
	{
		*type = true;
		*ret = usable;
		printf("lw $%d, %d($fp)\n", usable, p->offset);
	}
	else if (is_global_var(str, &p))
	{
		*type = true;
		*ret = usable;
		printf("lw $%d, %s\n", usable, str);
	}
	else
	{
		printf("error in get_src_value_or_reg!\n");
	}
}


// type = 1:返回寄存器编号
// type = 2:返回内存偏移量
// type = 3:全局变量
void get_dst_reg_or_addr(char *str, int *type, int *ret)
{
	struct object *p;
	if (is_local_var(str, &p))
	{
		if (p->is_in_reg)
		{
			*type = 1;
			*ret = p->reg_num;
		}
		else
		{
			*type = 2;
			*ret = p->offset;
		}
	}
	else if (is_global_var(str, &p))
	{
		*type = 3;
	}
	else
	{
		unused_reg++;
		if (unused_reg <= 25)
		{
			*type = 1;
			*ret = unused_reg;
			local_var_tail->is_in_reg = true;
			local_var_tail->reg_num = unused_reg;
			//printf("变量%s存在寄存器%d\n", str, local_var_tail->reg_num);
		}
		else
		{
			*type = 2;
			*ret = fp_offset - 4;
			local_var_tail->is_in_reg = false;
			//printf("变量%s存在内存偏移%d\n", str, fp_offset - 4);
		}
		fp_offset -= 4;
		local_var_tail->offset = fp_offset;
		strcpy(local_var_tail->name, str);		
		local_var_tail->next = (struct object*)malloc(sizeof(struct object));
		local_var_tail = local_var_tail->next;
		local_var_tail->next = NULL;
	}
}

// type = true:局部数组，返回偏移量
// type = false:全局数组
void get_array_offset(char *array, bool *type, int *offset)
{
	struct object *p;
	if (is_local_var(array, &p))
	{
		*type = true;
		*offset = p->offset;
	}
	else if (is_global_var(array, &p))
	{
		*type = false;
	}
	else
	{
		printf("get_array_offset!\n");
	}
}