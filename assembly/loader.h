#include"assemble.h"

#define MAX_LEN 100
#define MAX_BUFFER_LEN 200

//명령어에 대한 정보를 저장할 구조체
typedef struct cmd_stuct {
	char input[MAX_LEN];
	char* token[MAX_LEN];
	int cnt;
}cmd_struct;

//file을 link하기위해 필요한 정보를 저장하는 구조체
typedef struct load_map* load_map_ptr;
typedef struct load_map {
	char code[MAX_LEN];
	int addr;
	int length;
}load_map;

char* hexa_to_str(int num, int n);
int checkDupli(char***hash, char*s);
int findAddr(load_map_ptr mapH, load_map_ptr mapOW, char*s, int n);
void printMap(load_map_ptr mapH, load_map_ptr mapOW, int n, int total_len);
int link_load(cmd_struct*cmd, int*memory_arr, int*total_len, int saddr);
