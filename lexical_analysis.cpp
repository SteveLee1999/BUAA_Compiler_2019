#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "my_structure.h"
#include "my_lexical.h"
#include "my_error.h"

extern struct word *p;
extern int line;

int getsym(char cin) // 读取一个单词，并返回多读的字符
{
	char c = cin;
	while (isspace(c) || c == '\0')
	{
		if (c == '\n')
		{
			line++;
		}
		c = getchar();
	}
	int cnt = -1;

	if (c == EOF)
	{
		return -2;
	}
	else if (isalpha(c) || c == '_')
	{
		p->line = line;
		while (isalnum(c) || c == '_')
		{
			p->name[++cnt] = c;
			c = getchar();
		}
		p->name[++cnt] = '\0';
		if (!word_isRESERVE())
		{
			// 已经检查过每个字符，因此如果不是保留字就一定是标识符
			strcpy(p->code, "IDENFR");
		}
		return (int)c; // retract();
	}
	else if (isdigit(c))
	{
		p->line = line;
		while (isdigit(c))
		{
			p->name[++cnt] = c;
			c = getchar();
		}
		p->name[++cnt] = '\0';
		word_isINT();
		return (int)c; // retract();
	}
	else if (c == '\'')
	{
		p->line = line;
		c = getchar();
		if (getchar() != '\'')
		{
			error_a(line);
		}
		if (c != '+' && c != '-' && c != '*' && c != '/' &&
			c != '_' && !isalnum(c))
		{
			error_a(line);
		}
		strcpy(p->code, "CHARCON");
		p->name[0] = c;
		p->name[1] = '\0';
	}
	else if (c == '\"')
	{
		p->line = line;
		while ((c = getchar()) != '\"' && c != EOF)
		{
			p->name[++cnt] = c;
		}
		if (c == EOF) // 缺失"
		{
			error_a(line);
		}
		p->name[++cnt] = '\0';
		word_isSTR();
	}
	else if (c == '=')
	{
		p->line = line;
		c = getchar();
		if (c == '=')
		{
			strcpy(p->code, "EQL");
			strcpy(p->name, "==");
		}
		else
		{
			strcpy(p->code, "ASSIGN");
			strcpy(p->name, "=");
			return (int)c; // retract();
		}
	}
	else if (c == '<')
	{
		p->line = line;
		c = getchar();
		if (c == '=')
		{
			strcpy(p->code, "LEQ");
			strcpy(p->name, "<=");
		}
		else
		{
			strcpy(p->code, "LSS");
			strcpy(p->name, "<");
			return (int)c; // retract();
		}
	}
	else if (c == '>')
	{
		p->line = line;
		c = getchar();
		if (c == '=')
		{
			strcpy(p->code, "GEQ");
			strcpy(p->name, ">=");
		}
		else
		{
			strcpy(p->code, "GRE");
			strcpy(p->name, ">");
			return (int)c; // retract();
		}
	}
	else if (c == '!')
	{
		p->line = line;
		if (getchar() == '=')
		{
			strcpy(p->code, "NEQ");
			strcpy(p->name, "!=");
		}
		else
		{
			error_a(line);
		}
	}
	else if (c == '+')
	{
		p->line = line;
		strcpy(p->code, "PLUS");
		strcpy(p->name, "+");
	}
	else if (c == '-')
	{
		p->line = line;
		strcpy(p->code, "MINU");
		strcpy(p->name, "-");
	}
	else if (c == '*')
	{
		p->line = line;
		strcpy(p->code, "MULT");
		strcpy(p->name, "*");
	}
	else if (c == '/')
	{
		p->line = line;
		strcpy(p->code, "DIV");
		strcpy(p->name, "/");
	}
	else if (c == ';')
	{
		p->line = line;
		strcpy(p->code, "SEMICN");
		strcpy(p->name, ";");
	}
	else if (c == ',')
	{
		p->line = line;
		strcpy(p->code, "COMMA");
		strcpy(p->name, ",");
	}
	else if (c == '(')
	{
		p->line = line;
		strcpy(p->code, "LPARENT");
		strcpy(p->name, "(");
	}
	else if (c == ')')
	{
		p->line = line;
		strcpy(p->code, "RPARENT");
		strcpy(p->name, ")");
	}
	else if (c == '[')
	{
		p->line = line;
		strcpy(p->code, "LBRACK");
		strcpy(p->name, "[");
	}
	else if (c == ']')
	{
		p->line = line;
		strcpy(p->code, "RBRACK");
		strcpy(p->name, "]");
	}
	else if (c == '{')
	{
		p->line = line;
		strcpy(p->code, "LBRACE");
		strcpy(p->name, "{");
	}
	else if (c == '}')
	{
		p->line = line;
		strcpy(p->code, "RBRACE");
		strcpy(p->name, "}");
	}
	else // 此时有非法字符
	{
		error_a(line);
	}
	return 32;
}

bool word_isRESERVE() // 返回false说明不是保留字
{
	switch (strlen(p->name))
	{
	case 2:
		if (p->name[0] == 'd' && p->name[1] == 'o')
		{
			strcpy(p->code, "DOTK");
			return true;
		}
		else if (p->name[0] == 'i' && p->name[1] == 'f')
		{
			strcpy(p->code, "IFTK");
			return true;
		}
		else
			return false;
	case 3:
		if (p->name[0] == 'f' && p->name[1] == 'o' && p->name[2] == 'r')
		{
			strcpy(p->code, "FORTK");
			return true;
		}
		else if (p->name[0] == 'i' && p->name[1] == 'n' && p->name[2] == 't')
		{
			strcpy(p->code, "INTTK");
			return true;
		}
		else
			return false;
	case 4:
		if (p->name[0] == 'c' && p->name[1] == 'h' &&
			p->name[2] == 'a' && p->name[3] == 'r')
		{
			strcpy(p->code, "CHARTK");
			return true;
		}
		else if (p->name[0] == 'e' && p->name[1] == 'l' &&
			p->name[2] == 's' && p->name[3] == 'e')
		{
			strcpy(p->code, "ELSETK");
			return true;
		}
		else if (p->name[0] == 'm' && p->name[1] == 'a' &&
			p->name[2] == 'i' && p->name[3] == 'n')
		{
			strcpy(p->code, "MAINTK");
			return true;
		}
		else if (p->name[0] == 'v' && p->name[1] == 'o' &&
			p->name[2] == 'i' && p->name[3] == 'd')
		{
			strcpy(p->code, "VOIDTK");
			return true;
		}
		else
			return false;
	case 5:
		if (p->name[0] == 'c' && p->name[1] == 'o' &&
			p->name[2] == 'n' && p->name[3] == 's' &&
			p->name[4] == 't')
		{
			strcpy(p->code, "CONSTTK");
			return true;
		}
		else if (p->name[0] == 's' && p->name[1] == 'c' &&
			p->name[2] == 'a' && p->name[3] == 'n' &&
			p->name[4] == 'f')
		{
			strcpy(p->code, "SCANFTK");
			return true;
		}
		else if (p->name[0] == 'w' && p->name[1] == 'h' &&
			p->name[2] == 'i' && p->name[3] == 'l' &&
			p->name[4] == 'e')
		{
			strcpy(p->code, "WHILETK");
			return true;
		}
		else
			return false;
	case 6:
		if (p->name[0] == 'p' && p->name[1] == 'r' &&
			p->name[2] == 'i' && p->name[3] == 'n' &&
			p->name[4] == 't' && p->name[5] == 'f')
		{
			strcpy(p->code, "PRINTFTK");
			return true;
		}
		else if (p->name[0] == 'r' && p->name[1] == 'e' &&
			p->name[2] == 't' && p->name[3] == 'u' &&
			p->name[4] == 'r' && p->name[5] == 'n')
		{
			strcpy(p->code, "RETURNTK");
			return true;
		}
		else
			return false;
	default:
		return false;
	}
	return false;
}

void word_isSTR()
{
	for (int i = 0; i < strlen(p->name); i++)
	{
		if (p->name[0] != 32 && p->name[0] != 33 &&
			(p->name[0] < 35 || p->name[0] > 126))
		{
			error_a(line);
		}
	}
	strcpy(p->code, "STRCON");
}

void word_isINT()
{
	if (strcmp(p->name, "0") != 0 && p->name[0] == '0')
	{
		error_a(line);
	}
	else
	{
		strcpy(p->code, "INTCON");
	}
}