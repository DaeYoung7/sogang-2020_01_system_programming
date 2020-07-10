#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef struct opcode_struct* opcode_ptr;
typedef struct opcode_struct {
	char code[10];
	int num;
	int format;
	opcode_ptr link;
}opcode_struct;

opcode_ptr* make_opcode();
int get_hash(char str[10]);
void push_node(opcode_ptr* op_ptr, int idx, int num, char str[10], char*format);
void print_opcode(opcode_ptr* op_ptr);
int find_opcode(opcode_ptr* op_ptr, char*str);
void free_opcode(opcode_ptr*op_ptr);
