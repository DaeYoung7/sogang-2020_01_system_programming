#define _CRT_SECURE_NO_WARNINGS

#include"20150614.h"

//bpCnt: 설정된 bp의 개수, locNum: loc의 개수(object code가 있는 line의 개수)
int bpCnt = 0, locNum = 0;
//start_address: progaddr명령어로 설정될 프로그램의 시작 주소, total_len: 프로그램의 길이
int start_address = 0, total_len = 0;
/*flagBp: 처음 bp 명령어를 실행하거나 bp없이 run하는 경우 code를 loc순서로 정렬하면서 정보를 저장하기 위한 flag
 1 code가 메모리에 load되었지만 loc에 대한 정렬이 이뤄지지 않은 경우, 0 load되지 않은 경우, -1 load와 정렬을 모두 수행한 경우*/
int flagBp = 0;
//start_run: run을 여러번 하는 경우 시작해야할 bp index를 기억
int start_run = 0;

//object code가 저장될 가상 메모리
int memory_arr[LEN_LIMIT] = { 0 };
//bp의 정보(loc)가 저장될 배열
int bpArr[MAX_LEN] = { 0 };
//각 loc마다 register의 정보를 저장할 구조체배열
locInfo locArr[MAX_LEN];
//copy.obj에 사용되는 opcode list
copyopcode codelist[OPCODE_LEN] = {
	{"LDA", 3, 0x00},
	{"LDB", 3, 0x68},
	{"LDT", 3, 0x74},
	{"LDCH", 3, 0x50},
	{"STL", 3,0x14},
	{"STA", 3,0x0C},
	{"STCH", 3,0x54},
	{"STX", 3,0x10},
	{"JSUB", 3,0x48},
	{"J", 3,0x3C},
	{"JEQ", 3,0x30},
	{"JLT", 3,0x38},
	{"RSUB", 3,0x4C},
	{"COMP", 3,0x28},
	{"TD", 3,0xE0},
	{"RD", 3,0xD8},
	{"WD", 3,0xDC},
	{"CLEAR", 2,0xB4},
	{"COMPR", 2,0xA0},
	{"TIXR", 2,0xB8}
};

int make_command(cmd_struct*cmd, hist_ptr *h_start, hist_ptr* h_last, opcode_ptr* op_ptr, int* first) {
	//start_address: 프로그램의 시작주소, memory_arr: 가상 메모리 배열
	//어떤 입력도 받지 않았을 때
	if (strcmp(cmd->input, "\n") == 0) {
		printf("enter command\n");
		return 0;
	}
	
	//new line을 제거한 입력을 input에 따로 저장
	char* temp = strtok(cmd->input, "\n");
	char input[MAX_LEN];
	strcpy(input, temp);

	//cmd구조체에 저장된 input을 ','와 new line단위로 분리한 정보와 그 개수를 cmd구조체에 저장
	cmd->cnt = 0;
	cmd->token[cmd->cnt] = strtok(cmd->input, " ,\n");
	while (cmd->token[cmd->cnt++]) {
		cmd->token[cmd->cnt] = strtok(NULL, " ,\n");
	}
	cmd->cnt--;

	//제대로 된 입력을 처음 받은 경우 history linked list의 start 설정
	if (*first&&*h_last) {
		*first = 0;
		*h_start = *h_last;
	}

	/**********
		각 명령어에 대한 처리
		q or quit을 제외한 모든 명령어는 제대로 된 입력일 때 history에 저장
		제대로 된 명령인지는 개수(cmd->cnt)로 체크
	***********/
	//q or quit인 경우 history와 opcode의 포인터들을 할당 해제하고 프로그램 종료를 위해 1 return
	if (strcmp(cmd->token[0], "q") == 0 || strcmp(cmd->token[0], "quit") == 0) {
		if (cmd->cnt != 1) {
			printf("\ttoo many tokens\n");
			return 0;
		}
		free_history(h_start);
		free_opcode(op_ptr);
		free_symbol();
		return 1;
	}
	//h or help인 경우 명령어들 출력
	else if (strcmp(cmd->token[0], "h") == 0 || strcmp(cmd->token[0], "help") == 0) {
		if (cmd->cnt != 1) {
			printf("\ttoo many tokens\n");
			return 0;
		}
		put_history(h_last, input);
		printf("h[elp]\n"
			"d[ir]\n"
			"q[uit]\n"
			"hi[story]\n"
			"du[mp] [start, end]\n"
			"e[dit] address, value\n"
			"f[ill] start, end, value\n"
			"reset\n"
			"opcode mnemonic\n"
			"opcodelist\n"
			"assemble filename\n"
			"type filename\n"
			"symbol\n"
			"bp [address]\n"
			"run\n");			
	}
	//d or dir 인경우 현재 디렉토리에 있는 요소들 출력
	else if (strcmp(cmd->token[0], "d") == 0 || strcmp(cmd->token[0], "dir") == 0) {
		if (cmd->cnt != 1) {
			printf("\ttoo many tokens\n"); 
			return 0;
		}
		put_history(h_last, input);
		check_dir();
	}
	//hi or history인 경우 명령어의 hustory 출력
	else if (strcmp(cmd->token[0], "hi") == 0 || strcmp(cmd->token[0], "history") == 0) {
		if (cmd->cnt != 1) {
			printf("\ttoo many tokens\n");
			return 0;
		}
		put_history(h_last, input);
		//처음 제대로 된 명령어를 받은 경우 start가 null이기 때문에 last로 생성
		if (*first) print_history(*h_last);
		else print_history(*h_start);
	}
	//du or dump인 경우 메모리에 대한 출력
	else if (strcmp(cmd->token[0], "du") == 0 || strcmp(cmd->token[0], "dump") == 0) {
		if (cmd->cnt > 3) {
			printf("\ttoo many tokens\n");
			return 0;
		}
		//s:start f:finish check:제대로 된 인자를 받았는지 체크
		int s, f, check = 0;
		switch (cmd->cnt) {
			//인자를 du or dump 하나만 받은 경우
		case 1:
			dump(memory_arr, -1, -1);
			check = 1;
			break;
		case 2:
			//인자를 du or dump, start index 2개를 받은 경우
			s = str_to_hexa(cmd->token[1]);
			if (s >= 0 && s < LEN_LIMIT) {
				dump(memory_arr, s, -1);
				check = 1;
			}
			else printf("start memory is out of range\n");
			break;
		case 3:
			//인자를 du or dump, start,finish index 3개를 받은 경우
			s = str_to_hexa(cmd->token[1]);
			f = str_to_hexa(cmd->token[2]);
			if (s >= 0 && s <= f && f < LEN_LIMIT) {
				dump(memory_arr, s, f);
				check = 1;
			}
			else printf("there is problem in number\n");
		}
		//제대로 된 명령어인 경우 history에 저장
		if(check) put_history(h_last, input);
	}
	//e or edit인 경우 메모리에 접근해서 값 수정
	else if (strcmp(cmd->token[0], "e") == 0 || strcmp(cmd->token[0], "edit") == 0) {
		if (cmd->cnt != 3) {
			printf("\ttoo many tokens\n");
			return 0;
		}
		//a: address, v: value
		int a = str_to_hexa(cmd->token[1]);
		int v = str_to_hexa(cmd->token[2]);
		//address가 메모리 범위 안에 있고 value가 알맞은 범위에 있는 경우만 실행
		if (a >= 0 && a < LEN_LIMIT && v >= 0 && v <= 255) {
			edit(memory_arr, a, v);
			put_history(h_last, input);
		}
		else printf("there is problem in number\n");
	}
	//f or fill인 경우 메모리의 일정 범위의 값을 공통 값으로 변경
	else if (strcmp(cmd->token[0], "f") == 0 || strcmp(cmd->token[0], "fill") == 0) {
		if (cmd->cnt != 4) {
			printf("\ttoo many tokens\n");
			return 0;
		}
		//s: start, f: finish, v: value
		int s = str_to_hexa(cmd->token[1]);
		int f = str_to_hexa(cmd->token[2]);
		int v = str_to_hexa(cmd->token[3]);
		if (s >= 0 && s <= f && f < LEN_LIMIT && v >= 0 && v <= 255) {
			fill(memory_arr, s, f, v);
			put_history(h_last, input);
		}
		else printf("there is problem in number\n");
	}
	//reset인 경우 메모리의 값을 0으로 초기화
	else if (strcmp(cmd->token[0], "reset") == 0) {
		if (cmd->cnt != 1) {
			printf("\ttoo many tokens\n");
			return 0;
		}
		reset(memory_arr);
		put_history(h_last, input);
	}
	//opcode인 경우 opcode에 대한 값 출력
	else if (strcmp(cmd->token[0], "opcode") == 0) {
		if (cmd->cnt != 2) {
			printf("\ttoo many tokens\n");
			return 0;
		}
		if(find_opcode(op_ptr, cmd->token[1])) put_history(h_last, input);
	}
	//opcodelist인 경우 모든 opcode출력
	else if (strcmp(cmd->token[0], "opcodelist") == 0) {
		if (cmd->cnt != 1) {
			printf("\ttoo many tokens\n");
			return 0;
		}
		put_history(h_last, input);
		print_opcode(op_ptr);
	}
	//assemble filename인 경우 lst, obj파일 생성
	else if (strcmp(cmd->token[0], "assemble") == 0) {
		if (cmd->cnt != 2) {
			printf("\ttoo many tokens\n");
			return 0;
		}
		int check[2] = { 0 };
		make_assemble(check, cmd->token[1], op_ptr);
		if (check[0] == 1) {
			printf("Successfully assemble %s.\n", cmd->token[1]);
			put_history(h_last, input);
		}
		else if (check[0] == 0) printf("Wrong filename\n");
		else if (check[0] == -1) printf("Error in line number %d\n", check[1]*5);
	}
	//type filename인 경우 파일 내용 출력
	else if (strcmp(cmd->token[0], "type") == 0) {
		if (cmd->cnt != 2) {
			printf("\ttoo many tokens\n");
			return 0;
		}
		if (print_type(cmd->token[1])) put_history(h_last, input);
		else printf("Wrong filename\n");
	}
	//symbol인 경우 .asm에 있는 symbol 출력
	else if (strcmp(cmd->token[0], "symbol") == 0) {
		if (cmd->cnt != 1) {
			printf("\ttoo many tokens\n");
			return 0;
		}
		if (print_symbol()) put_history(h_last, input);
		else printf("Assemble first\n");
	}
	//progaddr인 경우 start_address 설정
	else if (strcmp(cmd->token[0], "progaddr") == 0) {
		if (cmd->cnt != 2) {
			printf("\tWrong input, Check help\n");
			return 0;
		}
		start_address = str_to_hexa(cmd->token[1]);
		if (start_address != -1) put_history(h_last, input);
		else {
			start_address = 0;
			printf("Wrong address!");
		}
	}
	//loader인 경우 file을 link한 뒤 메모리에 load하는 작업 수행
	else if (strcmp(cmd->token[0], "loader") == 0) {
		if (cmd->cnt < 2) {
			printf("\tWrong input, Check help\n");
			return 0;
		}
		//load하지 않고 프로그램을 실행하는 경우를 방지하기 위해 flagBp를 0에서 1로 변경
		else {
			if (link_load(cmd, memory_arr, &total_len, start_address)) {
				flagBp = 1;
				bpCnt = 0;
				locNum = 0;
				put_history(h_last, input);
			}
			else printf("Duplicated symbol error");
		}
	}
	//bp인 경우 bp, bp clear, bp address 에 대한 작업 수행
	else if (strcmp(cmd->token[0], "bp") == 0) {
		if (cmd->cnt > 2) {
			printf("\tWrong input, Check help\n");
			return 0;
		}
		else if (flagBp == 0) {
			printf("loading first!!\n");
			return 0;
		}
		else {
			//bp, bp목록 출력
			if (cmd->cnt == 1) {
				if (bpCnt) sortBp(locArr, bpArr, bpCnt, locNum);
				printBp(bpArr, bpCnt);
			}
			//bp clear, bp관한 정보 초기화
			else if (strcmp(cmd->token[1], "clear") == 0) {
				start_run = 0;
				clearBp(bpArr, &bpCnt);
			}
			//bp address, bp 설정
			else {
				//loader 명령어를 실행했고 sortLoc함수를 실행하지 않았을 때 실행
				if (flagBp == 1) {
					locNum = sortLoc(codelist, locArr, memory_arr, start_address, total_len);
					flagBp = -1;
				}
				setBp(bpArr, &bpCnt, cmd->token[1]);
			}
			put_history(h_last, input);
			return 0;
		}
	}
	//ruc 명령어에 대한 처리
	else if (strcmp(cmd->token[0], "run") == 0) {
		if (cmd->cnt != 1) {
			printf("\ttoo many tokens\n");
			return 0;
		}
		else if (flagBp == 0) {
			printf("loading first!!\n");
			return 0;
		}
		else {
			if (flagBp == 1) {
				locNum = sortLoc(codelist, locArr, memory_arr, start_address, total_len);
				flagBp = -1;
			}
			run(locArr, bpArr, &start_run, bpCnt, locNum);
			put_history(h_last, input);
		}
	}
	else {
		printf("\twrong command\n\tcheck command using help\n");
	}
	return 0;
}

