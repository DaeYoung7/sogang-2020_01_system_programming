#include<math.h>
#include"opcode.h"

int print_type(char*filename);
int split_line(char*line, char**word, opcode_ptr *op_ptr);
int str_to_int(char* s);
int str_to_hexa(char*s);
int get_opcode(char*s, opcode_ptr* op_ptr, int is_format);
int register_mem(char*c);
int twos_complement(int n);
int code_len(int n);
int get_symbol(int code_num, char**tmp, char*s);
int make_symbol(int idx);
int print_symbol();
int make_lstfile(int idx, char*filename, opcode_ptr* op_ptr);
void make_objfile(int idx, char*filename);
int* make_assemble(int *ret_arr, char*filename, opcode_ptr *op_ptr);
void free_symbol();

//.asm 각 line의 정보를 저장할 구조체
typedef struct assem_info {
	int line;
	int loc;
	int code;
	int format;
	//지시어, symbol, 변수, 상수, 주석인지 체크
	int is_direct;
	int is_symbol;
	int is_variable;
	int is_comment;
	int is_const;

	char symbol[10];
	char opcode[10];
	char v_m[10]; //value, move
	char comment[100];
}assem_info;

//symbol의 정보를 저장할 구조체
typedef struct symbol_info* symbol_ptr;
typedef struct symbol_info{
	char symbol[10];
	int loc;
	int line;
	int stdio;
	char*byte_arr;//RESB, BYTE
	int*word_arr;//RESW, WORD
	symbol_ptr fwd;
	symbol_ptr bwd;
}symbol_info;
