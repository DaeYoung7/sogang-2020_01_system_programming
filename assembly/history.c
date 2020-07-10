#include"history.h"

//history linked list 출력
void print_history(hist_ptr h_start) {
	int cnt = 1;
	hist_ptr temp = h_start;
	while (temp) {
		printf("\t%-4d %s\n", cnt++, temp->cmd);
		temp = temp->link;
	}
}

//linked list 마지막 노드(h_last)에 새로 만든 노드를 연결
void put_history(hist_ptr *h_last, char*str) {
	hist_ptr temp = (hist_ptr)malloc(sizeof(hist_struct));
	temp->link = NULL;
	strncpy(temp->cmd, str, MAX_LEN);
	if (*h_last) {
		h_last[0]->link = temp;
		h_last[0] = temp;
	}
	else h_last[0] = temp;
}

//linked list 할당 해제
void free_history(hist_ptr* h_start) {
	hist_ptr temp = NULL;
	while (h_start[0]) {
		temp = h_start[0]->link;
		free(h_start[0]);
		h_start[0] = temp;
	}
}
