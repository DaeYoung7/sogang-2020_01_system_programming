#include<stdio.h>
#include<string.h>
#define LEN_LIMIT 0x100000

int min_(int x, int y);
void dump(int*memory_arr, int s, int f);
void edit(int*memory_arr, int a, int v);
void fill(int*memory_arr, int s, int f, int v);
void reset(int*memory_arr);
