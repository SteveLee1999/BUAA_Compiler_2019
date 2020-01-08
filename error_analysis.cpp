#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "my_structure.h"
#include "my_error.h"

extern struct word *p;
extern int total_line;
int first_error_a_line; // 第一个词法错误位置

void error_a(int line) // 词法错误
{
	first_error_a_line = line;
	printf("%d a\n", line);
	exit(0);
}

void error_b(int line) // 重名
{
	printf("%d b\n", line);
	print_end();
	// 不再填表即可，无需特殊处理
}

void error_c(int line) // 未定义
{
	printf("%d c\n", line);	
	print_end();
}

void error_d(int line)
{
	printf("%d d\n", line);
	print_end();
}

void error_e(int line)
{
	printf("%d e\n", line);
	print_end();
}

void error_f(int line)
{
	printf("%d f\n", line);
	print_end();
}

void error_g(int line)
{
	printf("%d g\n", line);
	print_end();
}

void error_h(int line)
{
	printf("%d h\n", line);
	print_end();
}

void error_i(int line)
{
	printf("%d i\n", line);
	print_end();
}

void error_j(int line)
{
	printf("%d j\n", line);
	print_end();
}

void error_k(int line)
{
	printf("%d k\n", line);
	print_end();
}

void error_l(int line)
{
	printf("%d l\n", line);
	print_end();
}

void error_m(int line)
{
	printf("%d m\n", line);
	print_end();
}

void error_n(int line)
{
	printf("%d n\n", line);
	print_end();
}

void error_o(int line)
{
	printf("%d o\n", line);
	while (strcmp(p->code, "COMMA") != 0 && strcmp(p->code, "SEMICN") != 0)
	{
		p = p->next;
	}
	print_end();
}

void print_end()
{/*
	int i;
	char j;
	for (i = p->line + 1;i <= total_line;i++)
	{
		for (j = 'a'; j <= 'o'; j++)
		{
			printf("%d %c\n", i, j);
		}
	}
	exit(0);*/
}