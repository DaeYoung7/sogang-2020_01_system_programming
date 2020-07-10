#include"memory.h"

//last: 마지막 출력 index+1 값 저장
int last = 0;

//최솟값 찾는 함수
int min_(int x, int y) {
	if (x < y) return x;
	else return y;
}

//dump 명령어 수행
void dump(int*memory_arr, int s, int f) {
	int i, j, start = s, finish = f;
	//start index가 없는 경우 last부터 출력
	if (s == -1) {
		start = last;
		if (start == LEN_LIMIT) start = 0;
	}
	//finish index가 없는 경우 start+160-1과 마지막 index 중 작은 값 저장
	if (f == -1) {
		finish = min_(start + 160 - 1,LEN_LIMIT - 1);
	}
	//줄 단위로 출력(x/16, 한 줄에 16개 메모리 index존재)
	//값 출력 ; 문자출력
	for (i = start / 16; i <= finish / 16; i++) {
		printf("%05X  ", i*16);
		//start와 finish index가 같은 줄에 위치한 경우
		if (start / 16 == finish / 16) {
			for (j = i * 16; j < start; j++) printf("    ");
			for (j = start; j < finish + 1; j++) printf("%02X  ", memory_arr[j]);
			for (j = finish + 1; j < i * 16 + 16; j++) printf("    ");
			printf(";  ");
			for (j = i * 16; j < start; j++) printf(". ");
			for (j = start; j < finish + 1; j++) {
				if (memory_arr[j] > 31 && memory_arr[j] < 127) printf("%c ", memory_arr[j]);
				else printf(". ");
			}
			for (j = finish + 1; j < i * 16 + 16; j++) printf(". ");
		}
		//첫 줄을 출력할 때 start가 해당 줄의 처음이 아닌 경우(값은 빈칸, 문자는 .으로 출력)
		else if (i == start / 16 && start % 16 != 0) {
			for (j = i*16; j < start; j++) printf("    ");
			for (j = start; j < (i*16 + 16); j++) printf("%02X  ", memory_arr[j]);
			printf(";  ");
			for (j = i*16; j < start; j++) printf(". ");
			for (j = start; j < (i * 16 + 16); j++) {
				if(memory_arr[j] > 31 && memory_arr[j] < 127) printf("%c ", memory_arr[j]);
				else printf(". ");
			}
		}
		//마지막 줄을 출력할 때 last가 해당 줄의 마지막이 아닌 경우(값은 빈칸, 문자는 .으로 출력)
		else if (i == finish / 16 && finish % 16 != 15) {
			for (j = i*16; j <= finish; j++) printf("%02X  ", memory_arr[j]);
			for (j = finish+1; j < i*16 + 16; j++) printf("    ");
			printf(";  ");
			for (j = i*16; j <= finish; j++) {
				if (memory_arr[j] > 31 && memory_arr[j] < 127) printf("%c ", memory_arr[j]);
				else printf(". ");
			}
			for (j = finish + 1; j < i*16 + 16; j++) printf(". ");
		}
		else {
			for (j = i*16; j < i*16 + 16; j++) printf("%02X  ", memory_arr[j]);
			printf(";  ");
			for (j = i*16; j < i*16 + 16; j++) {
				if (memory_arr[j] > 31 && memory_arr[j] < 127) printf("%c ", memory_arr[j]);
				else printf(". ");
			}
		}
		printf("\n");
	}
	//마지막 요소가 가상 메모리 배열의 끝인 경우 last를 배열의 처음으로 설정
	if ((finish + 1) == LEN_LIMIT) last = 0;
	else last = finish + 1;
	return;
}

//메모리 배열의 특정 index의 값 변경
void edit(int*memory_arr, int a, int v) {
	memory_arr[a] = v;
}

//메모리 배열의 특정 구간의 값 변경
void fill(int*memory_arr, int s, int f, int v) {
	for (int i = s; i <= f; i++) {
		memory_arr[i] = v;
	}
}

//메모리 배열의 값을 모두 0으로 초기화
void reset(int*memory_arr) {
	memset(memory_arr, 0, LEN_LIMIT);
	last = 0;
}
