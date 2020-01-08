#ifndef _MY_STRUCTURE_H_
#define _MY_STRUCTURE_H_

struct word // 单词
{
	char name[100];
	char code[10];
	int line; // 出现行数
	struct word *next;
	struct word *before;
};

struct function // 函数
{
	char name[100];
	int returntype; // 0void,1int,2char
	int para_num; // 参数个数
	int para_table[50]; // 1表示int,2表示char
	struct function *next;
};

struct symbol // 符号表
{
	char name[100];
	int type; // 1int,2表示char,3表示void
	bool isarray; // true表示是数组
	bool isfunction; // true表示是函数名
	bool isconst; // true表示是常量
	struct symbol *next;
};

struct constant // 常量表
{
	char name[100];
	int type; // 1int,2表示char
	int value; // int则就是值，char则是ascii值
	struct constant *next;
};

struct object // mips
{
	char name[100];
	int value;
	int offset; // 如果是局部变量，这里是栈内偏移（0, 4, 8...)
	bool is_in_reg; // 是否在寄存器中
	int reg_num; // 寄存器编号（如果有）
	bool is_valid;
	struct object *next;
};

struct dag_point
{
	char name[100];
	int number; // dag图中结点序号
	struct dag_point *next;
};

struct dag_vertex // dag图
{
	char name[100]; // 变量名，操作符或者scanf
	bool is_mid; // 是否是中间节点
	int number; // dag图中结点序号,也用作scanf的顺序！
	int father[200]; // 父节点（可能多个！）的序号，0表示空
	int father_num; //父节点个数（从1开始计）
	int left_son;
	int right_son;
};

struct dag_print_vertex
{
	int type; // 五种可能：1:%s%d 2:%s%c 3:%d 4:%c 5:%s
	int var; // 这里存的是对应的dag图节点号
	char str[500]; // (如果有)字符串
	struct dag_print_vertex *next;
};

struct dag_scan_vertex
{
	int type; // 两种可能：1:%d 2:%c
	char var[100]; // 要读取的变量名
	int number; // 对应的dag图节点号
	struct dag_scan_vertex *next;
};

#endif 