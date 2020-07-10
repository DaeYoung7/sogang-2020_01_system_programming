#include"loader.h"

//10진수를 16진수 문자열로 변환
char* hexa_to_str(int num, int n) {
	int r, i, j = n - 1;
	char *hexa = (char*)calloc(n, sizeof(char));
	for(i=0;i<n;i++) {
		r = num % 16;
		if (r < 10) r += 48;
		else r += 55;
		hexa[j--] = (char)r;
		num /= 16;
	}
	return hexa;
}

//symbol의 중복 체크, 정상 종료:0
int checkDupli(char***hash, char*s) {
	int i = 0, h = get_hash(s);
	if (hash[h] == NULL) {
		hash[h] = (char**)calloc(20, sizeof(char*));
	}
	else {
		while (hash[h][i] != NULL) {
			if (strcmp(hash[h][i], s) == 0) return 1;
			else i++;
		}
	}
	hash[h][i] = (char*)malloc(sizeof(char) * 10);
	strcpy(hash[h][i], s);
	return 0;
}

//symbol에 대한 주소 찾아서 return
int findAddr(load_map_ptr mapH, load_map_ptr mapOW, char*s, int n) {
	int ret;
	for (int i = 0; i < n - 1; i++) {
		if (strcmp(mapH[i].code, s) == 0) ret = mapH[i].addr;
		if (strcmp(mapOW[i * 2].code, s) == 0) ret = mapOW[i * 2].addr;
		if (strcmp(mapOW[i * 2 + 1].code, s) == 0) ret = mapOW[i * 2 + 1].addr;
	}
	return ret;
}

//file을 link할 때 필요한 정보들을 출력(+길이)
void printMap(load_map_ptr mapH, load_map_ptr mapOW, int n, int total_len) {
	printf("control symbol address length\n");
	printf("section name\n");
	printf("-------------------------------------\n");
	for (int i = 0; i < n - 1; i++) {
		printf("%s\t\t\t%04X\t%04X\n", mapH[i].code, mapH[i].addr, mapH[i].length);
		if (mapOW[i * 2].code[0]) {
			printf("\t\t%s\t%04X\n", mapOW[i * 2].code, mapOW[i * 2].addr);
			printf("\t\t%s\t%04X\n", mapOW[i * 2 + 1].code, mapOW[i * 2 + 1].addr);
		}
	}
	printf("-------------------------------------\n");
	printf("\t\t  total length %04X\n", total_len);
}

//loader 명령어의 main역할을 수행하는 함수.
//입력 받은 파일들을 link한 뒤 memory에 load한다.
int link_load(cmd_struct*cmd, int*memory_arr, int*total_len, int saddr) {
	//i,j,k,l: 값을 임시로 저장할 변수, csaddr_t: link함에 따라 바뀌는 start address 저장, csaddr: 전체 프로그램의 시작주소 저장
	int i, j, k, l, csaddr_t, csaddr = saddr;
	//file의 한 줄에 대한 정보를 저장할 배열
	char buffer[MAX_BUFFER_LEN + 1];
	//값을 임시로 저장할 배열
	char temp[7] = { 0 };
	//mapH: H에 대한 정보 저장, mapOW: D에 대한 정보 저장
	load_map_ptr mapH = (load_map_ptr)malloc(sizeof(load_map)*(cmd->cnt - 1));
	load_map_ptr mapOW = (load_map_ptr)malloc(sizeof(load_map)*(cmd->cnt - 1)*2);
	//symbol에 대한 중복 체크하기 위한 포인터
	char ***hashTable = (char***)calloc(20, sizeof(char**));
	//pass1
	for (i = 0; i < (cmd->cnt - 1); i++) {
		FILE*fp = NULL;
		fp = fopen(cmd->token[i + 1], "rt");
		while (fgets(buffer, MAX_BUFFER_LEN, fp) != NULL) {
			j = 1;
			//H
			if (buffer[0] == 'H') {
				while (buffer[j] != ' ') j++;
				//symbol저장
				for (k = 0; k < j - 1; k++) mapH[i].code[k] = buffer[k + 1];
				mapH[i].code[k] = '\0';
				if (checkDupli(hashTable, mapH[i].code)) return 0;
				//시작주소 저장
				mapH[i].addr = csaddr;
				while (buffer[j] == ' ') j++;
				//길이 저장
				for (k = j + 6; k < j + 12; k++) temp[k - j - 6] = buffer[k];
				mapH[i].length = str_to_hexa(temp);
				csaddr_t = csaddr;
				csaddr += mapH[i].length;
				// M에 관한 처리 있는지 확인
				mapOW[i * 2].code[0] = '\0';
			}
			//D
			else if (buffer[0] == 'D') {
				while (buffer[j] != ' ') j++;
				//symbol 저장
				for (k = 0; k < j - 1; k++) mapOW[i * 2].code[k] = buffer[k + 1];
				mapOW[i * 2].code[k] = '\0';
				if (checkDupli(hashTable, mapOW[i * 2].code)) return 0;
				//주소 저장
				for (k = j + 1; k < j + 7; k++) temp[k - j - 1] = buffer[k];
				mapOW[i*2].addr = str_to_hexa(temp) + csaddr_t;
				j += 7;
				l = j;
				while (buffer[j] != ' ') j++;
				//symbol 저장
				for (k = l; k < j; k++) mapOW[i * 2 + 1].code[k - l] = buffer[k];
				mapOW[i * 2 + 1].code[k - l] = '\0';
				if (checkDupli(hashTable, mapOW[i * 2 + 1].code)) return 0;
				while (buffer[j] == ' ') j++;
				//주소 저장
				for (k = j; k < j + 6; k++) temp[k - j] = buffer[k];
				mapOW[i * 2 + 1].addr = str_to_hexa(temp) + csaddr_t;
			}
		}
		fclose(fp);
	}
	//pass2
	//mapR: R로 시작하는 line의 정보 저장, forward_addr: 이전 주소 저장, p:임시 저장 변수
	int mapR[3][10], forward_addr, start, len, p;
	//plus_minus: M에서 +혹은 -인지 저장할 배열, regi: 수정에 사용될 register번호 저장
	int plus_minus[20], regi[20];
	//flagM: modify인 경우 E에서 마지막 modify에 대한 작업 수행
	int flagM;
	int cnt, sum;
	//format3: 6, format4: 5
	//5인 경우 남은 자리에 값을 넣기 위해 해당 값을 저장하는 변수
	char five_six;
	//수정할 주소의 값을 문자열형태로 저장
	char forward_value[7];
	//문자열을 임시로 저장할 배열
	char temp_c[10] = { 0 }, temp_n[3] = { 0 };
	char*temp_p;
	for (i = 0; i < (cmd->cnt - 1); i++) {
		cnt = 0;
		flagM = 0;
		forward_addr = -1;
		forward_value[6] = '\0';

		FILE*fp = NULL;
		fp = fopen(cmd->token[i + 1], "rt");
		while (fgets(buffer, MAX_BUFFER_LEN, fp) != NULL) {
			j = 1;
			//R
			//symbol의 번호를 mapR[i]의 index로 사용해서 symbol에 대한 주소를 저장
			if (buffer[0] == 'R') {
				mapR[i][1] = mapH[i].addr;
				for (p = 0; p < 4; p++) {
					while (buffer[j] >= '0' && buffer[j] <= '9') j++;
					l = j - 1;
					while (buffer[j] != ' ' && buffer[j] != '\n') j++;
					//symbol에 대한 주소 찾아서 저장
					for (k = l + 1; k < j; k++) temp_c[k - l - 1] = buffer[k];
					for (k = j - l - 1; k < 10; k++) temp_c[k] = '\0';
					mapR[i][buffer[l] - '0'] = findAddr(mapH, mapOW, temp_c, cmd->cnt);
					while (buffer[j] == ' ') j++;
				}
			}
			//T
			else if (buffer[0] == 'T') {
				//시작 주소 계산
				for (j = 1; j < 7; j++) temp[j - 1] = buffer[j];
				start = str_to_hexa(temp) + mapH[i].addr;
				//길이 계산
				for (k = j; k < j + 2; k++) temp_n[k - j] = buffer[k];
				temp_n[2] = '\0';
				j = k;
				len = str_to_hexa(temp_n);
				//문자 2개씩 10진수로 변환해서 memory_arr에 저장
				for (k = start; k < start + len; k++) {
					for (p = j; p < j + 2; p++) temp_n[p - j] = buffer[p];
					j = p;
					memory_arr[k] = str_to_hexa(temp_n);
				}
			}
			//M
			//동일한 주소에 대한 modify는 한꺼번에 수행
			else if (buffer[0] == 'M') {
				flagM = 1;
				//수정할 주소 계산
				for (j = 1; j < 7; j++) temp[j - 1] = buffer[j];
				start = str_to_hexa(temp) + mapH[i].addr;
				//이전에 저장된 주소와 다르다면 수정 작업 수행
				if (start != forward_addr) {
					//첫번째 줄이 아닌 경우
					if (forward_addr != -1) {
						sum = 0;
						//regi와 plus_minus를 사용해서 수정할 값 계산
						for (k = 0; k < cnt; k++) sum += plus_minus[k] * mapR[i][regi[k]];
						temp_p = hexa_to_str(str_to_hexa(forward_value) + sum, 6);
						//5인 경우 첫번째 숫자 변경
						if (len == 5) temp_p[0] = five_six;
						//값을 옮긴 뒤 10진수로 바꾸고 메모리에 저장
						for (k = 0; k < 3; k++) {
							temp_n[0] = temp_p[k * 2];
							temp_n[1] = temp_p[k * 2 + 1];
							memory_arr[forward_addr + k] = str_to_hexa(temp_n);
						}
						free(temp_p);
						cnt = 0;
					}
					forward_addr = start;
					temp_p = hexa_to_str(memory_arr[start], 2);
					//길이5인 경우를 대비해 남을 자리의 값 저장해둔다.
					five_six = temp_p[0];
					//forward_value에 대한 계산
					if (len == 5) forward_value[0] = '0';
					else forward_value[0] = five_six;
					forward_value[1] = temp_p[1];
					free(temp_p);
					for (k = 1; k < 3; k++) {
						temp_p = hexa_to_str(memory_arr[start + k], 2);
						forward_value[k * 2] = temp_p[0];
						forward_value[k * 2 + 1] = temp_p[1];
						free(temp_p);
					}
				}
				//길이 계산
				for (k = j; k < j + 2; k++) temp_n[k - j] = buffer[k];
				len = str_to_hexa(temp_n);
				//plus, minus 계산
				if(buffer[k]=='+') plus_minus[cnt] = 1;
				else plus_minus[cnt] = -1;
				j = k + 1;
				//사용하는 register 저장
				for (k = j; k < j + 2; k++) temp_n[k - j] = buffer[k];
				regi[cnt++] = str_to_hexa(temp_n);
			}
			//E
			else if (buffer[0] == 'E' && flagM) {
				sum = 0;
				for (k = 0; k < cnt; k++) sum += plus_minus[k] * mapR[i][regi[k]];
				temp_p = hexa_to_str(str_to_hexa(forward_value) + sum, 6);
				if (len == 5) temp_p[0] = five_six;
				for (k = 0; k < 3; k++) {
					temp_n[0] = temp_p[k * 2];
					temp_n[1] = temp_p[k * 2 + 1];
					memory_arr[forward_addr + k] = str_to_hexa(temp_n);
				}
				free(temp_p);
			}
		}
		fclose(fp);
	}
	//전체 길이 계산
	*total_len = csaddr - saddr;
	printMap(mapH, mapOW, cmd->cnt, *total_len);
	//free
	free(mapH);
	free(mapOW);
	for (i = 0; i < 20; i++) {
		if (hashTable[i]) {
			for (j = 0; j < 20; j++) {
				if (hashTable[i][j]) free(hashTable[i][j]);
				else break;
			}
			free(hashTable[i]);
		}
	}
	free(hashTable);
	return 1;
}
