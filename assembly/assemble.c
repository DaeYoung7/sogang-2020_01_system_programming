#define _CRT_SECURE_NO_WARNINGS

#include"assemble.h"

// assembly 지시어
char* asssem_direct[8] = { "START", "END", "BASE", "NOBASE", "BYTE", "WORD", "RESB", "RESW" };
//register 종류
char* registers[10] = { "A", "X", "L", "B", "S", "T", "F", "", "PC", "SW"};
//.asm파일의 정보가 저장될 구조체 배열
assem_info assem_arr[2000000];
//symbol linked list의 시작 node의 주소
symbol_ptr head = NULL;

//type filename에 대한 작업 수행
//파일 열고 내용 출력
int print_type(char*filename) {
	FILE*fp = fopen(filename, "r");
	if (!fp) return 0;
	char buffer[300];

	while (fgets(buffer, sizeof(buffer), fp) != NULL) fputs(buffer, stdout);
	fclose(fp);
	return 1;
}

/*
	line(.asm의 한 줄)을 symbol, opcode, value혹은move로 구분해서 해당 line이 차지하는 메모리 크기를 계산하는 함수
	line이 없는 경우 -3, 주석인 경우('.' 으로 시작) -2, 에러가 있는 경우 -1 return
	정상인 경우 format return
	assembly 지시어는 0 return
	변수, 상수인 경우 10을 더해서 opcode와 구분해준다.
*/
int split_line(char*line, char**word, opcode_ptr *op_ptr) {
	//3번 자른 뒤 문자가 더 남아있는지 체크 (BUFFER, X인 경우를 위한 변수)
	char *tmp, temp[5] = " ";
	if (!line) return -3;
	if (line[0] == '.') return -2;
	//line을 symbol, opcode, v_m으로 분리
	if (line[0] == ' ') {
		word[0] = NULL;
		word[1] = strtok(line, " ");
		word[2] = strtok(NULL, " ");
	}
	else {
		word[0] = strtok(line, " ");
		word[1] = strtok(NULL, " ");
		word[2] = strtok(NULL, " ");
	}
	tmp = strtok(NULL, " ");
	if (tmp) {
		tmp = strcat(temp, tmp);
		word[2] = strcat(word[2], tmp);
	}
	if (word[1][0] == '+') return 4;
	int format_num = get_opcode(word[1], op_ptr, 1);
	if (format_num >= 0) return format_num;
	if (word[1] && word[2]) {
		for (int i = 0; i < 8; i++) {
			if (!strcmp(word[1], asssem_direct[i])) {
				if (i < 4) return 0;
				else if (i == 4) {
					if (word[2][0] == 'C') return strlen(word[2]) - 3 + 10;
					if (word[2][0] == 'X') {
						int temp = strlen(word[2]) - 3;
						return temp / 2 + temp % 2 + 10;
					}
				}
				else if (i == 5) return 3 + 10;
				else if (i == 6) return str_to_int(word[2]) + 10;
				else return str_to_int(word[2]) * 3 + 10;
			}
		}
	}
	//어느 것에도 해당하지 않으면 잘못된 명령어이므로 에러를 return한다.
	return -1;
}

//숫자(10진수)로 이뤄진 문자열을 10진수로 변환해주는 함수
int str_to_int(char* s) {
	int i, len = strlen(s), ret = 0;
	if (s) {
		for (i = 0; i < len; i++) {
			if(s[i]>='0' && s[i]<='9') ret = ret * 10 + (int)s[i] - '0';
			else return -1;
		}
	}
	else return -1;
	return ret;
}

//string type의 hexadecimal index나 value를 decimal로 바꿔주는 함수
int str_to_hexa(char*s) {
	//temp: hexadecimal 범위의 문자인지 체크, d: 값 저장
	int temp, d = 0;
	int len = strlen(s);

	//자릿값이 큰 자릿수부터 계산하며 16을 곱해주는 방식
	for (int i = 0; i < len; i++) {
		if ('0' <= s[i] && s[i] <= '9') temp = (int)(s[i] - '0');
		else if ('a' <= s[i] && s[i] <= 'f') temp = (int)(s[i] - 'a' + 10);
		else if ('A' <= s[i] && s[i] <= 'F') temp = (int)(s[i] - 'A' + 10);
		else temp = -1;

		if (temp != -1) {
			d *= 16;
			d += temp;
		}
		else return -1;
	}
	return d;
}

//opcode의 값을 반환하는 함수
//is_format으로 원하는 값이 format인지 code값인지 체크
int get_opcode(char*s, opcode_ptr* op_ptr, int is_format) {
	int op_idx = get_hash(s);
	opcode_ptr walk = op_ptr[op_idx];
	//문자열이 opcode일 때 is_format을 체크한 뒤 알맞은 값을 return한다.
	while (walk) {
		if (strcmp(walk->code, s) == 0) {
			if(is_format) return walk->format;
			else return walk->num;
		}
		walk = walk->link;
	}
	return -1;
}

//register의 주소 반환
int register_mem(char*c) {
	int i, l = strlen(c);
	if (*c) for (i = 0; i < 10; i++) if (registers[i] && registers[i][0] == *c) if (l == strlen(registers[i])) return i;
	return -1;
}

//2의 보수를 취하는 함수
//숫자를 2진수로 변환해서 한 글자씩 배열에 저장한 뒤 거꾸로 읽으며 보수를 취해준다.
int twos_complement(int n) {
	n *= -1;
	int idx = 0, arr[20] = { 0 };
	long int ret = 0;
	while (n / 2) {
		arr[idx] = n % 2;
		n /= 2;
		idx++;
	}
	arr[idx] = 1;
	for (int i = 11; i >= 0; i--) {
		ret *= 2;
		if(!arr[i]) ret += 1;
	}
	return ++ret;
}

//숫자 n을 16진수로 나타냈을 때 숫자의 길이를 구하는 함수
int code_len(int n) {
	int ret = 0;
	while (n / 16) {
		n /= 16;
		ret += 1;
	}
	return ++ret;
}

//opcode나 v_m(값이나 이동할 symbol)에 특수문자가 섞인 경우 symbol만을 추출하는 함수
//nixbpe 중 ni에 대한 값을 code_num에 더하는 역할도 수행
int get_symbol(int code_num, char**tmp,char*s) {
	char temp[100];
	strcpy(temp, s);
	if (temp[0] == '#') {
		code_num += 1;
		*tmp = strtok(temp, "#");
	}
	else if (temp[0] == '@') {
		code_num += 2;
		*tmp = strtok(temp, "@");
	}
	else {
		code_num += 3;
		*tmp = temp;
	}
	if (tmp[0][strlen(*tmp) - 3] == ',')  *tmp = strtok(*tmp, ",");
	if (tmp[0][0] == '+') *tmp = strtok(*tmp, "+");
	return code_num;
}

//symbol linked list를 만드는 함수
int make_symbol(int idx) {
	/*    변수
		c: error check, flag_l: head가 NULL이면 0, 처음 생성 이후는 1
		store: strtok에서 문자열이 훼손되는 것을 방지하기 위해 문자열을 임시로 저장할 배열
		walk: 마지막 node를 가리킬 포인터, temp: 현재 저장할 정보를 갖는 node
		walk_t: linked list를 앞으로 이동하며 temp와 값을 비교할 포인터
	*/
	int i, c, len, cmp, flag_l = 0;
	char *t, store[10];
	symbol_ptr walk, walk_t, temp;
	for (i = 1; i < idx; i++) {
		if (assem_arr[i].is_symbol) {
			//node 생성 후 node내 변수 초기화
			if (!flag_l) {
				head = (symbol_ptr)malloc(sizeof(symbol_info));
				temp = head;
			}
			else {
				temp = (symbol_ptr)malloc(sizeof(symbol_info));
				cmp = strcmp(assem_arr[i].symbol, walk->symbol);
			}
			temp->bwd = temp->fwd = NULL;
			temp->byte_arr = NULL;
			temp->word_arr = NULL;
			temp->line = assem_arr[i].line;
			temp->loc = assem_arr[i].loc;
			temp->stdio = 0;
			strcpy(temp->symbol, assem_arr[i].symbol);

			//assembly 지시어인 경우 추가작업 실행
			//여기서 변수와 상수 구분해준다.
			if (!strcmp(assem_arr[i].opcode, "BYTE")) {
				assem_arr[i].is_const = 1;
				assem_arr[i].is_variable = 0;
				len = strlen(assem_arr[i].v_m);
				strcpy(store, assem_arr[i].v_m);
				t = strtok(store, "'");
				t = strtok(NULL, "'");
				temp->byte_arr = (char*)malloc(sizeof(char)*(len-3));
				//BYTE인 경우 v_m자체가 아닌 값만 저장
				strcpy(temp->byte_arr, t);
				if (assem_arr[i].v_m[0] == 'X') temp->stdio = 1;
			}
			else if (!strcmp(assem_arr[i].opcode, "RESB")) temp->byte_arr = (char*)malloc(sizeof(char)*assem_arr[i].format);
			else if(!strcmp(assem_arr[i].opcode, "WORD")) {
				assem_arr[i].is_const = 1;
				assem_arr[i].is_variable = 0;
				temp->word_arr = (int*)malloc(sizeof(int));
				c = str_to_int(assem_arr[i].v_m);
				if (c < 0) return i + 1;
				temp->word_arr[0] = c;
			}
			else if (!strcmp(assem_arr[i].opcode, "RESW")) temp->word_arr = (int*)malloc(sizeof(int)*(assem_arr[i].format / 3));
			//symbol을 알맞은 위치에 연결
			if (flag_l) {
				//temp가 walk보다 앞
				if (cmp < 0) {
					walk_t = walk;
					while ((cmp < 0) && walk_t->fwd != NULL) {
						walk_t = walk_t->fwd;
						cmp = strcmp(temp->symbol, walk_t->symbol);
					}
					if (cmp == 0) return i + 1;
					else if (cmp < 0) {
						temp->bwd = head;
						head->fwd = temp;
						head = temp;
					}
					else {
						temp->bwd = walk_t->bwd;
						walk_t->bwd->fwd = temp;
						temp->fwd = walk_t;
						walk_t->bwd = temp;
					}
				}
				//중복된 symbol, error
				else if (cmp == 0) return i + 1;
				//temp가 walk보다 뒤(walk가 마지막 node이므로 바로 연결)
				else {
					temp->fwd = walk;
					walk->bwd = temp;
					walk = temp;
				}
			}
			//처음 생성하는 경우
			else {
				walk = head;
				flag_l = 1;
			}
		}
	}
	return 0;
}

//symbol linked list의 처음부터 마지막 node까지 출력하는 함수
int print_symbol() {
	symbol_ptr walk = head;
	if (head) {
		while (walk) {
			printf("\t%-6s  %04X\n", walk->symbol, walk->loc);
			walk = walk->bwd;
		}
		return 1;
	}
	else return 0;
}

//.asm파일의 각 line정보를 이용해서 .lst파일을 만드는 함수
int make_lstfile(int idx, char*filename, opcode_ptr* op_ptr) {
	/*    변수
		i,j : for문의 index로 사용, c : 함수의 return값 받고 error체크할 때 사용
		store_tmp: strtok으로 인해 문자열이 훼손될 수 있어서 store_tmp에 저장한 뒤 store_tmp로 작업
	*/
	int i, j, c, address;
	long int code_num = 0;
	symbol_ptr walk, walk_t;
	char*tmp = NULL, *fn = strcat(strtok(filename, "."), ".lst"), store_tmp[10];
	FILE*lstp = fopen(fn, "w");

	fprintf(lstp, "%-3d  \t%04X\t%s\t%s\t%s\n", assem_arr[0].line, assem_arr[0].loc, assem_arr[0].symbol, assem_arr[0].opcode, assem_arr[0].v_m);
	for (i = 1; i < idx; i++) {
		assem_arr[i].code = 0;
		//assembly 지시어 중 BASE인 경우 v_m 정보를 BASE에 저장
		if (assem_arr[i].is_direct) {
			if (!strcmp(assem_arr[i].opcode, "BASE")) registers[7] = assem_arr[i].v_m;
			fprintf(lstp, "%-3d  \t\t\t%s\t%s\n", assem_arr[i].line, assem_arr[i].opcode, assem_arr[i].v_m);
		}
		else if (assem_arr[i].is_comment) fprintf(lstp, "%-3d  \t\t\t%s", assem_arr[i].line, assem_arr[i].comment);
		else if (assem_arr[i].is_const) {
			// 상수(BYTE, WORD)인 경우 v_m을 값으로 변환하는 과정 필요
			if (!strcmp(assem_arr[i].opcode, "BYTE")) {
				walk = head;
				while (strcmp(walk->symbol, assem_arr[i].symbol) != 0) walk = walk->bwd;
				//hexa인 경우
				if (walk->stdio) {
					assem_arr[i].code += str_to_hexa(walk->byte_arr);
					fprintf(lstp, "%-3d  \t%04X\t%s\t%s\t%-20s%02X\n", assem_arr[i].line, assem_arr[i].loc, assem_arr[i].symbol, assem_arr[i].opcode, assem_arr[i].v_m, assem_arr[i].code);
				}
				else {
					for (j = 0; j < (signed)(strlen(walk->byte_arr)); j++) {
						assem_arr[i].code *= 256;
						assem_arr[i].code += (int)walk->byte_arr[j];
					}
					fprintf(lstp, "%-3d  \t%04X\t%s\t%s\t%-20s%0X\n", assem_arr[i].line, assem_arr[i].loc, assem_arr[i].symbol, assem_arr[i].opcode, assem_arr[i].v_m, assem_arr[i].code);
				}

			}
			else if (!strcmp(assem_arr[i].opcode, "WORD")) {
				strcpy(store_tmp, assem_arr[i].v_m);
				c = str_to_int(store_tmp);
				if (c < 0) return i + 1;
				assem_arr[i].code = c;
				fprintf(lstp, "%-3d  \t%04X\t%s\t%s\t%-20s%0X\n", assem_arr[i].line, assem_arr[i].loc, assem_arr[i].symbol, assem_arr[i].opcode, assem_arr[i].v_m, assem_arr[i].code);
			}
		}
		else if(assem_arr[i].is_variable) fprintf(lstp, "%-3d  \t%04X\t%s\t%s\t%-20s\n", assem_arr[i].line, assem_arr[i].loc, assem_arr[i].symbol, assem_arr[i].opcode, assem_arr[i].v_m);
		//opcode인 경우
		else {
			code_num = get_symbol(code_num, &tmp, assem_arr[i].opcode);
			
			strcpy(store_tmp, tmp);
			code_num = get_opcode(store_tmp, op_ptr, 0);
			switch (assem_arr[i].format) {
			case 2:
				strcpy(store_tmp, assem_arr[i].v_m);
				tmp = strtok(store_tmp, ", ");
				code_num *= 16;
				c = register_mem(tmp);
				if (c < 0) return i + 1;
				code_num += c;
				code_num *= 16;
				tmp = strtok(NULL, " ");
				if (tmp) {
					c = register_mem(tmp);
					if (c < 0) return i;
					code_num += c;
				}
				assem_arr[i].code = code_num;
				if (assem_arr[i].is_symbol) fprintf(lstp, "%-3d  \t%04X\t%s\t%s\t%-20s%04X\n", assem_arr[i].line, assem_arr[i].loc, assem_arr[i].symbol, assem_arr[i].opcode, assem_arr[i].v_m, assem_arr[i].code);
				else fprintf(lstp, "%-3d  \t%04X\t\t%s\t%-20s%04X\n", assem_arr[i].line, assem_arr[i].loc, assem_arr[i].opcode, assem_arr[i].v_m, assem_arr[i].code);
				break;
			case 3:
				code_num = get_symbol(code_num, &tmp, assem_arr[i].v_m);
				strcpy(store_tmp, tmp);
				code_num *= 2;
				//nixbpe 중 x에 대한 조사
				if (assem_arr[i].v_m[strlen(assem_arr[i].v_m) - 1] == 'X') code_num += 1;
				walk = head;
				while (walk && strcmp(walk->symbol, store_tmp)) walk = walk->bwd;
				//symbol이 있는 경우
				if (walk) {
					//format3이면서 접근하려는 주소와의 차이가 12bit이상일 때 BASE에 값이 존재해야 한다.
					if (abs(walk->loc - assem_arr[i + 1].loc) > 4095) {
						if (!strcmp(registers[7], "B")) {
							fclose(lstp);
							return i + 1;
						}
						//BASE의 symbol node 탐색
						walk_t = head;
						while (walk && strcmp(walk_t->symbol, registers[7]) != 0) walk_t = walk_t->bwd;
						address = walk->loc - walk_t->loc;
						if (address < 0) return i + 1;
						//b=1, p=0, e=0
						code_num = code_num * 8 + 4;
					}
					else {
						//b=0, p=1, e=0
						code_num = code_num * 8 + 2;
						address = walk->loc - assem_arr[i + 1].loc;
						if (address < 0) address = twos_complement(address);
					}
					code_num = code_num * (int)pow(16,3) + address;
				}
				//symbol이 없는 경우
				else {
					if (assem_arr[i].v_m) {
						c = str_to_int(store_tmp);
						if (c < 0) return i + 1;
						code_num = (code_num * 8) * (int)pow(16, 3) + c;
					}
					else code_num = (code_num * 8) * (int)pow(16, 3);
				}
				assem_arr[i].code = code_num;
				if (assem_arr[i].is_symbol) fprintf(lstp, "%-3d  \t%04X\t%s\t%s\t%-20s%06X\n", assem_arr[i].line, assem_arr[i].loc, assem_arr[i].symbol, assem_arr[i].opcode, assem_arr[i].v_m, assem_arr[i].code);
				else fprintf(lstp, "%-3d  \t%04X\t\t%s\t%-20s%06X\n", assem_arr[i].line, assem_arr[i].loc, assem_arr[i].opcode, assem_arr[i].v_m, assem_arr[i].code);
				break;
			case 4:
				code_num = get_symbol(code_num, &tmp, assem_arr[i].v_m);
				strcpy(store_tmp, tmp);
				code_num *= 2;
				if (assem_arr[i].v_m[strlen(assem_arr[i].v_m) - 1] == 'X') code_num += 1;
				code_num = code_num * 8 + 1;
				walk = head;
				while (walk && strcmp(walk->symbol, store_tmp)) walk = walk->bwd;
				if (walk) code_num = code_num * (int)pow(16, 5) + walk->loc;
				else {
					if (assem_arr[i].v_m) {
						c = str_to_int(store_tmp);
						if (c < 0) return i + 1;
						code_num = code_num * (int)pow(16, 5) + c;
					}
					else return i + 1;
				}
				assem_arr[i].code = code_num;
				if (assem_arr[i].is_symbol) fprintf(lstp, "%-3d  \t%04X\t%s\t%s\t%-20s%0X\n", assem_arr[i].line, assem_arr[i].loc, assem_arr[i].symbol, assem_arr[i].opcode, assem_arr[i].v_m, assem_arr[i].code);
				else fprintf(lstp, "%-3d  \t%04X\t\t%s\t%-20s%0X\n", assem_arr[i].line, assem_arr[i].loc, assem_arr[i].opcode, assem_arr[i].v_m, assem_arr[i].code);
			}
		}
	}
	tmp = NULL;
	fclose(lstp);
	return 0;
}

//.asm파일 각 line의 정보를 이용해서 obj파일을 생성하는 함수
void make_objfile(int idx, char*filename) {
	/*	변수
		i,j,k: index변수, start,finish: 현재 line에 출력할 node의 시작과 끝 index, len,len_c: 출력 길이
		print_idx: 출력할 index정보를 담는 배열, jsub_idx: M에 대한 정보를 출력할 배열(jsub)일 때만 사용
		tmp,store_tmp: 문자열 훼손 방지용
	*/
	int i, j, k, start, finish = 0, len = 0, len_c;
	int*print_idx = (int*)calloc(idx, sizeof(int));
	int*modi_idx = (int*)calloc(idx, sizeof(int));
	char*tmp, *fn = strcat(strtok(filename, "."), ".obj"), store_tmp[10];
	FILE*objp = fopen(fn, "w");

	//H, 프로그램명과 시작, 끝loc에 대한 정보 출력
	fprintf(objp, "H%s  %06X%06X\n",assem_arr[0].symbol, assem_arr[0].loc, assem_arr[idx-1].loc);
	//T, 첫줄과 마지막줄을 제외한 줄에 대해 처리
	//object code의 숫자길이를 구한 뒤 출력해야할 자리수보다 작은 경우 남은 자리에 0을 출력하고 code출력
	for (i = 1; i < (idx - 1); i++) {
		//지시어나 주석의 경우 pass
		if (!(assem_arr[i].is_direct || assem_arr[i].is_comment)) {
			print_idx[i] = 1;
			get_symbol(0, &tmp, assem_arr[i].opcode);
			strcpy(store_tmp, tmp);
			//format4의 경우 immediate addressing이 아니면 symbol에 대한 접근으로 봐도 무방하다.
			//pc와 BASE를 사용한 접근은 format3이기 때문
			if (assem_arr[i].opcode[0] == '+' && assem_arr[i].v_m[0] != '#') modi_idx[i] = 1;
			if (i > finish) {
				start = i;
				finish = idx;
				len = 0;
			}
			//변수의 경우 메모리가 할당되지만 object code는 갖지 않으므로 전체 길이가 30을 넘는지 체크할 때만 사용
			//변수는 전체 길이에 포함시키지 않고 출력하지도 않는다.
			if (!assem_arr[i].is_variable) len += assem_arr[i].format;
			if ((len + assem_arr[i + 1].format) > 29 || i == idx - 2) {
				finish = i;
				fprintf(objp, "T%06X%02X",assem_arr[start].loc, len);
				for (j = start; j <= finish; j++) {
					if (print_idx[j] && !assem_arr[j].is_variable) {
						len_c = code_len(assem_arr[j].code);
						if (len_c % 2) {
							fprintf(objp, "0");
							len_c++;
						}
						for (k = 0; k < (assem_arr[j].format - len_c / 2); k++) fprintf(objp, "00");
						fprintf(objp, "%0X", assem_arr[j].code);
					}
				}
				fprintf(objp, "\n");
			}
		}
	}
	//M(JSUB)
	for (i = 0; i < idx; i++) {
		if (modi_idx[i]) {
			j = assem_arr[i].loc + 1;
			k = assem_arr[i].format * 2 - 3;
			fprintf(objp, "M%06X%02X\n", j, k);
		}
	}
	//E
	fprintf(objp, "E%06X\n", assem_arr[0].loc);

	fclose(objp);
	free(print_idx);
	free(modi_idx);
	return;
}

//assemble filename 명령어에 대한 main 함수 (ret_arr[0]: 에러체크, ret_arr[1]: 에러인 경우 line정보 저장
int* make_assemble(int *ret_arr, char*filename, opcode_ptr *op_ptr) {
	FILE*fp = fopen(filename, "r");
	if (!fp) return ret_arr;
	/*    변수 설명
	buffer: 파일을 읽어서 잠시 저장할 버퍼, line: buffer와 비슷한 역할
	word: symbol, opcode, v_m에 대한 정보를 저장할 변수
	add: line의 메모리 할당 정도를 계산해서 loc에 더해나갈 변수
	c: error check, mem: loc값, num: line number, idx: assem_arr의 index
	*/
	char buffer[100];
	char*line, **word;
	word = (char**)malloc(sizeof(char*) * 4);
	int c, add, mem = 0, num = 0, idx = 0;
	
	ret_arr[0] = -1;
	//이전에 assemble명령어를 수행한 경우 symbol linked list를 할당해제한다.
	if (head) {
		strcpy(registers[7], "B");
		free_symbol();
		head = NULL;
	}
	//object파일과 symbol의 정보를 저장한다.
	while (fgets(buffer, sizeof(buffer), fp) != NULL) {
		num += 5;
		ret_arr[1] = num/5;
		line = strtok(buffer, "\n");
		assem_arr[idx].line = num;
		assem_arr[idx].format = 0;
		assem_arr[idx].is_direct = 0;
		assem_arr[idx].is_symbol = 0;
		assem_arr[idx].is_variable = 0;
		assem_arr[idx].is_comment = 0;
		assem_arr[idx].is_const = 0;

		add = split_line(line, word, op_ptr);
		//첫 줄이 START인 경우 설정된 주소부터 시작한다.
		if (num == 5) {
			if (!strcmp("START", word[1])) {
				c = str_to_hexa(word[2]);
				if (c < 0) return ret_arr;
				mem += c;
			}
		}
		if (add == -2) {
			assem_arr[idx].is_comment = 1;
			strcat(line, "\n");
			strcpy(assem_arr[idx++].comment, line);
			continue;
		}
		//NULL값인 경우 다음 줄에 읽을 것이 있다면 에러, 없다면 정상 종료한다.
		else if (add == -3) {
			fgets(buffer, sizeof(buffer), fp);
			line = strtok(buffer, " \n");
			if (line && (line[0] > 32 && line[0] < 128)) return ret_arr;
			break;
		}
		else if (add == -1) return ret_arr;
		else {
			assem_arr[idx].loc = mem;
			if (word[0]) {
				strcpy(assem_arr[idx].symbol, word[0]);
				assem_arr[idx].is_symbol = 1;
			}
			strcpy(assem_arr[idx].opcode, word[1]);
			if (word[2]) strcpy(assem_arr[idx].v_m, word[2]);
		}
		//변수나 상수인 경우 10보다 크게 만듦
		if (add > 9) {
			add -= 10;
			assem_arr[idx].is_variable = 1;
		}
		else if (add == 0) assem_arr[idx].is_direct = 1;
		mem += add;
		assem_arr[idx].format = add;
		idx += 1;
	}
	ret_arr[1] = make_symbol(idx);
	if (ret_arr[1]) {
		printf("Duplicated symbol\n");
		return ret_arr;
	}
	c = make_lstfile(idx, filename, op_ptr);
	if(c) {
		ret_arr[1] = c + 1;
		return ret_arr;
	}
	make_objfile(idx, filename);
	fclose(fp);
	free(word);
	ret_arr[0] = 1;
	return ret_arr;
}

//symbol linked list 동적 할당해제
void free_symbol() {
	symbol_ptr walk = head;
	while (walk) {
		head = head->bwd;
		free(walk);
		walk = head;
	}
}

