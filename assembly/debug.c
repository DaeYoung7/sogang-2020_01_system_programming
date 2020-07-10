#include"debug.h"

//X, T, L: register의 정보를 임시로 저장할 변수
//flagX: EOF에 대한 출력으로 X값을 변경하기 위한 flag
//flagL: 다른 라인으로 jump했을 때 L값을 변경하기 위한 flag
//flagC: COMP 명령어가 True일 때 1
int X = 0, T = 0, L = 0, flagX = 0, flagL = 0, flagC = 0;

//line의 정보(loc, registers)를 저장하는 함수
void setInfo(copyopcode*codelist, locInfo*locArr, int*memArr, int*pc, int*l, int idx, int m, int c, int ni, int tlen) {
	//JSUB으로 이동해야할 때: L 이동한 뒤 돌아와야하는 line의 loc, *l 현재 돌아가야하는 line의 loc
	//이동한 뒤 *l에 L값 저장
	if (flagL) {
		flagL = 0;
		*l = L;
	}
	//i,j,k, temp: 연산에서 값 저장을 위해 사용할 변수, loc: 현재 loc의 정보
	int i, j, k, loc = *pc, temp;

	//기본적으로 이전 line의 정보를 받아옴
	if (idx) {
		locArr[idx].A = locArr[idx - 1].A;
		locArr[idx].X = locArr[idx - 1].X;
		locArr[idx].B = locArr[idx - 1].B;
		locArr[idx].S = locArr[idx - 1].S;
		locArr[idx].T = locArr[idx - 1].T;
	}
	//이전 line이 없는 경우 0으로 초기화
	else {
		locArr[idx].A = 0;
		locArr[idx].X = 0;
		locArr[idx].B = 0;
		locArr[idx].S = 0;
		locArr[idx].T = 0;
	}
	//format2
	if (codelist[c].format == 2) {
		//CLEAR
		//해당 register 0으로 clear
		if(codelist[c].num == 0xB4){
			if (memArr[m + 1] == 0x10) locArr[idx].X = 0;
			else if (memArr[m + 1] == 0x00) locArr[idx].A = 0;
			else if (memArr[m + 1] == 0x40) locArr[idx].S = 0;
		}
		//TIXR
		//flagX==1일 때 loop을 돌았다는 가정하에 T값을 X에 저장
		else if (codelist[c].num == 0xB8 && flagX) {
			locArr[idx].X = T;
			flagX = 0;
		}
		//이전 line과 달라질 정보에 대한 저장
		locArr[idx].loc = *pc;
		locArr[idx].L = *l;
		locArr[idx].PC = (*pc + 2);
		*pc += 2;
	}
	else {
		//format4
		if (memArr[m + 1] == 0x10) {
			*pc = loc + 4;
			//JSUB
			if (ni == 3) {
				flagL = 1;
				L = *pc;
				*pc = memArr[m + 2] * 16 * 16 + memArr[m + 3];
			}
			//LDT
			else if (ni == 1) locArr[idx].T = memArr[m + 2] * 16 * 16 + memArr[m + 3];
		}
		//format3
		else {
			*pc += 3;
			//접근해야할 주소 혹은 다뤄야할 값(address부분) 미리 계산
			temp = memArr[m + 1] % 16 * 16 * 16 + memArr[m + 2];
			if (locArr[idx].B && (memArr[m + 1] == 4 * 16 || memArr[m + 1] == 12 * 16)) temp += locArr[idx].B;
			else temp += *pc;

			switch (codelist[c].num) {
			case 0x00:
				//LDA
				//immediate address인 경우 pc, base사용되지 않았다.
				if (ni == 1) {
					if (locArr[idx].B && (memArr[m + 1] == 4 * 16 || memArr[m + 1] == 12 * 16)) temp -= locArr[idx].B;
					else temp -= *pc;
					locArr[idx].A = temp;
				}
				//EOF를 저장하는 경우는 예외적으로 처리
				else {
					locArr[idx].A = memArr[temp];
					if (memArr[temp + 3] == tlen) locArr[idx].A = 0x454F46;
				}
				break;
			case 0x68:
				//LDB
				if (ni == 1) locArr[idx].B = temp;
				else locArr[idx].B = memArr[temp];
				break;
			case 0x74:
				//LDT
				T = memArr[temp];
				locArr[idx].T = T;
				break;
			case 0x50:
				//LDCH
				//길이(T)만큼 차례대로 값을 A에 가져옴
				j = 0;
				while (X < T - 1) {
					flagX = 1;
					locArr[idx].A = memArr[temp + X + j];
					j++;
					X++;
				}
				break;
			case 0x14:
				//STL
				memArr[temp] = *l;
				break;
			case 0x0C:
				//STA
				//A에 저장된 값이 255(memArr 한 칸에 저장될 수 있는 크기)보다 크다면 memArr의 다음 index로 넘어가서 저장
				i = locArr[idx].A;
				j = 0;
				while (i / 256) {
					i /= 256;
					j++;
				}
				if (j) {
					i = locArr[idx].A;
					while (j >= 0) {
						k = i % 256;
						memArr[temp + j--] = k;
						i /= 256;
					}
				}
				else memArr[temp] = locArr[idx].A;
				break;
			case 0x3C:
				//J
				if (ni == 2) *pc = memArr[temp];
				break;
			case 0x30:
				//JEQ
				//flagC=1일 때 jump
				if (flagC) {
					*pc = temp;
					flagC = 0;
				}
				break;
			case 0x4C:
				//RSUB
				*pc = *l;
				break;
			case 0x28:
				//COMP
				//비교해서 값이 같으면 flagC=1
				if (locArr[idx].A == memArr[m + 2]) flagC = 1;
				break;
			}
		}
		locArr[idx].loc = loc;
		locArr[idx].L = *l;
		locArr[idx].PC = *pc;
	}
}

//code를 읽으면서 실행되는 순서대로 line을 locArr에 저장
int sortLoc(copyopcode*codelist, locInfo*locArr, int*memArr, int saddr, int tlen) {
	int i, j, t3, t2, t1, t0, idx = 0;
	int pc = 0, l = tlen;
	for (i = saddr; i < tlen;) {
		//0인 경우 pass(buffer)
		if (memArr[i] < 1) continue;
		//ni에 대한 처리(0, 1, 2, 3)
		t3 = memArr[i] - 3;
		t2 = memArr[i] - 2;
		t1 = memArr[i] - 1;
		t0 = memArr[i];
		//ni를 제외했을 때 알맞은 opcode를 찾는다.
		for (j = 0; j < OPCODE_LEN; j++) {
			if (t0 == codelist[j].num) {
				setInfo(codelist, locArr, memArr, &pc, &l, idx, i, j, 0, tlen);
				break;
			}
			else if (t1 == codelist[j].num) {
				setInfo(codelist, locArr, memArr, &pc, &l, idx, i, j, 1, tlen);
				break;
			}
			else if (t2 == codelist[j].num) {
				setInfo(codelist, locArr, memArr, &pc, &l, idx, i, j, 2, tlen);
				break;
			}
			else if (t3 == codelist[j].num){
				setInfo(codelist, locArr, memArr, &pc, &l, idx, i, j, 3, tlen);
				break;
			}
		}
		i = pc;
		idx++;
	}
	return idx;
}

//bpArr의 loc값을 실행 순서대로 정렬
void sortBp(locInfo*locArr, int*bpArr, int bpCnt, int locNum) {
	int i, j, min, tmp;
	//bpArr의 loc에 대한 실행순서를 저장할 배열의 포인터
	int*temp = (int*)calloc(bpCnt*2, sizeof(int));
	for (i = 0; i < bpCnt; i++) {
		for (j = 0; j < locNum; j++) {
			if (locArr[j].loc == bpArr[i]) {
				temp[i] = j;
				break;
			}
		}
	}
	//temp를 정렬하며 temp index 변화에 따라 bpArr도 변경
	for (i = 0; i < bpCnt; i++) {
		min = temp[i];
		tmp = i;
		for (j = i; j < bpCnt; j++) {
			if (min > temp[j]) {
				min = temp[j];
				tmp = j;
			}
		}
		min = tmp;
		tmp = bpArr[i];
		bpArr[i] = bpArr[min];
		bpArr[min] = tmp;
		tmp = temp[i];
		temp[i] = temp[min];
		temp[min] = tmp;
	}
	free(temp);
}

//bp: bp가 설정된 경우 bpArr의 내용 출력
void printBp(int*bpArr, int bpCnt) {
	printf("\t\t\tbreakpoint\n");
	printf("\t\t\t----------\n");
	for(int i=0;i<bpCnt;i++) printf("\t\t\t%X\n", bpArr[i]);
}

//bp address: bpArr에 값 추가
void setBp(int*bpArr, int*bpCnt, char*s) {
	int loc = str_to_hexa(s);
	bpArr[(*bpCnt)++] = loc;
	printf("\t\t\t[ok] create breakpoint %X\n", loc);
}

//bp clear: bp관한 정보 초기화
void clearBp(int*bpArr, int*bpCnt) {
	for (int i = 0; i < *bpCnt; i++) bpArr[i] = 0;
	*bpCnt = 0;
	printf("\t\t\t[ok] clear all breakpoints\n");
}

//실행할 loc의 범위를 정한 뒤 해당 line의 정보 출력
void run(locInfo*locArr, int*bpArr, int*start, int bpCnt, int locNum) {
	int i = 0;
	if (bpCnt && bpCnt > *start) {
		sortBp(locArr, bpArr, bpCnt, locNum);
		while (locArr[i].PC != bpArr[*start]) i++;
	}
	else i = locNum - 1;
	printf("\tA : %06X  X : %06X\n", locArr[i].A, locArr[i].X);
	printf("\tL : %06X PC : %06X\n", locArr[i].L, locArr[i].PC);
	printf("\tB : %06X  S : %06X\n", locArr[i].B, locArr[i].S);
	printf("\tT : %06X\n", locArr[i].T);
	if (bpCnt > *start) {
		printf("\t\t    Stop at checkpoint[%X]\n", bpArr[*start]);
		(*start)++;
	}
	else {
		printf("\t\t    End Program\n");
		*start = 0;
	}
}
