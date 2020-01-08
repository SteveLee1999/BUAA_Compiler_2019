#include <stdio.h>
#include <malloc.h>
#include "my_structure.h"
#include "my_lexical.h"
#include "my_syntax.h"
#include "my_mips.h"

struct word *p;
int line = 1; // 统计行数
int total_line; // 总行数

int main()
{
	freopen("testfile.txt", "r", stdin);
	freopen("intermediate_code(1).txt", "w", stdout);
	char c = ' ';
	p = (struct word*)malloc(sizeof(struct word));
	struct word *p_head = p;
	while (1)
	{
		c = getsym(c);
		if (c == -2)
		{
			break;
		}
		else
		{
			p->next = (struct word*)malloc(sizeof(struct word));
			p->next->before = p;
			p = p->next;
		}
	}
	total_line = p->before->line;
	p = p_head; 
	gram_procedure();
	printf("\n########\n");
	printf("EOF\n");
	fclose(stdin);
	fclose(stdout);
	freopen("intermediate_code(1).txt", "r", stdin);
	freopen("intermediate_code(2).txt", "w", stdout);
	mips_initialize_pointer();
	mips_store_data();
	mips_pass_constant();
	fclose(stdin);
	fclose(stdout);
	freopen("intermediate_code(2).txt", "r", stdin);
	freopen("mips.txt", "w", stdout);
	mips_assign_register();
	fclose(stdin);
	fclose(stdout);
	return 0;
}