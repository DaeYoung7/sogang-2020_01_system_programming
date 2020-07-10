#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define MAX_LEN 100
typedef struct hist_struct *hist_ptr;
typedef struct hist_struct {
	char cmd[MAX_LEN];
	hist_ptr link;
}hist_struct;

void print_history(hist_ptr h_start);
void put_history(hist_ptr* h_last, char*str);
void free_history(hist_ptr* h_start);
