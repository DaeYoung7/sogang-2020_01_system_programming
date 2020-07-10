#define _CRT_SECURE_NO_WARNINGS

#include"opcode.h"

//opcode.txt를 읽어서 hash table만들기
opcode_ptr* make_opcode() {
	opcode_ptr* op_ptr = (opcode_ptr*)calloc(20, sizeof(opcode_ptr));
	int num, idx;
	char str[10]="";
	char format[10];
	FILE*fp = fopen("opcode.txt", "rt");
	while (fscanf(fp, "%X %s %s", &num, str, format) != EOF) {
		idx = get_hash(str);
		push_node(op_ptr, idx, num, str, format);
	}
	return op_ptr;
}

//string의 각 자리 character의 아스키코드 값을 이용해서 hash값 생성
int get_hash(char str[10]) {
	int num = 0;
	while (*str) num += (*str++);
	return num % 20;
}

//hash table에 요소 추가
//hash값을 row index로 사용해서 해당 row의 마지막을 찾은 뒤 요소 추가
void push_node(opcode_ptr* op_ptr, int idx, int num, char str[10], char* format) {
	opcode_ptr walk = op_ptr[idx];
	//row에 아무 것도 없는 경우
	if (!walk) {
		op_ptr[idx] = (opcode_ptr)malloc(sizeof(opcode_struct));
		op_ptr[idx]->link = NULL;
		op_ptr[idx]->num = num;
		op_ptr[idx]->format = (int)format[0]-'0';
		strcpy(op_ptr[idx]->code, str);
	}
	//row에 이전에 생성된 요소가 있는 경우
	else {
		while (walk->link) {
			walk = walk->link;
		}
		opcode_ptr temp = (opcode_ptr)malloc(sizeof(opcode_struct));
		temp->num = num;
		temp->link = NULL;
		temp->format = (int)format[0]-'0';
		strcpy(temp->code, str);
		walk->link = temp;
	}
}
//hash table 전체를 출력
void print_opcode(opcode_ptr* op_ptr) {
	opcode_ptr walk = NULL;
	//모든 row에 대해(20개)
	for (int i = 0; i < 20; i++) {
		walk = op_ptr[i];
		printf("%-2d : ", i);
		//각 row의 요소들을 끝까지 출력
		while (walk) {
			printf("[%s, %X]",walk->code, walk->num);
			walk = walk->link;
			if (walk) printf(" -> ");
		}
		printf("\n");
	}
}

//입력받은 opcode의 값 출력
int find_opcode(opcode_ptr* op_ptr, char *str) {
	int idx = get_hash(str);
	opcode_ptr walk = op_ptr[idx];
	while (walk) {
		if (strcmp(walk->code, str) == 0) {
			printf("opcode is %X\n", walk->num);
			return 1;
		}
		walk = walk->link;
	}
	printf("wrong opcode\ncheck opcode using opcodelist command\n");
	return 0;
}

//opcode hash table 할당 해제
void free_opcode(opcode_ptr*op_ptr) {
	opcode_ptr walk = NULL;
	opcode_ptr temp = NULL;
	for (int i = 0; i < 20; i++) {
		walk = op_ptr[i];
		while (walk) {
			temp = walk;
			walk = walk->link;
			free(temp);
		}
	}
}
