#include <stdio.h>
#include "my_structure.h"
#include "my_intermediate_code.h"

extern struct constant *c;
bool is_the_first_function = true;

void code_function_state(int type, char *name, int para_num, struct symbol *para_begin)
{
	if (is_the_first_function)
	{
		is_the_first_function = false;
	}
	else
	{
		printf("ret void\n");
	}
	printf("\n\n########\n");
	switch (type)
	{
		case 0:
			printf("func void %s %d\n", name, para_num);
			break;
		case 1:
			printf("func int %s %d\n", name, para_num);
			break;
		case 2:
			printf("func char %s %d\n", name, para_num);
			break;
	}
	struct symbol *p = para_begin;
	for (int i = 1;i <= para_num;i++, p = p->next)
	{
		if (p->type == 1)
		{
			printf("para int %s\n", p->name);
		}
		else if (p->type == 2)
		{
			printf("para char %s\n", p->name);
		}
		else
		{
			printf("error in code_function_declaration!\n");
		}
	}
}

void code_constant_define()
{
	if (c->type == 1)
	{
		printf("const int %s %d\n", c->name, c->value);
	}
	else if (c->type == 2)
	{
		printf("const char %s %d\n", c->name, c->value);
	}
}

void code_variable_state(int type, bool isarray, char *name, int bound)
{
	if (type == 1 && isarray)
	{
		printf("var int[] %s %d\n", name, bound);
	}
	else if (type == 1 && !isarray)
	{
		printf("var int %s\n", name);
	}
	else if (type == 2 && isarray)
	{
		printf("var char[] %s %d\n", name, bound);
	}
	else if (type == 2 && !isarray)
	{
		printf("var char %s\n", name);
	}
	else
	{
		printf("error in code_variable_state!\n");
	}
}
