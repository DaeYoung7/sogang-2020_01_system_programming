#include"loader.h"

//각 loc마다 register의 정보를 저장할 구조체
typedef struct locInfo {
	int loc;
	int A;
	int X;
	int L;
	int PC;
	int B;
	int S;
	int T;
}locInfo;

//copy.obj에 사용된 opcode의 이름, format, code number를 저장할 구조체
typedef struct copyopcode {
	char code[10];
	int format;
	int num;
}copyopcode;


void setInfo(copyopcode*codelist, locInfo*locArr, int*memArr, int*pc, int*l, int idx, int m, int c, int ni, int tlen);
int sortLoc(copyopcode*codelist, locInfo*locArr, int*memArr, int saddr, int tlen);
void sortBp(locInfo*locArr, int*bpArr, int bnCnt, int locNum);
void printBp(int*bpArr, int bpCnt);
void setBp(int*bpArr, int*bpCnt, char*s);
void clearBp(int*bpArr, int*bpCnt);
void run(locInfo*locArr, int*bpArr, int*start, int bpCnt, int locNum);
#define OPCODE_LEN 22
