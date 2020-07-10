#include"20150614.h"

int main() {
	//가장 바깥 루프 빠져나갈지 체크
	int quit = 0;
	//제대로 된 입력을 받았는지 체크
	int first = 1;
	//history를 출력할 때 사용될 linked list의 처음과 끝
	hist_ptr h_start = NULL;
	hist_ptr h_last = NULL;
	//opcode.txt를 읽어서 hash table만듦
	opcode_ptr* op_ptr = make_opcode();

	while (!quit) {
		//명령어에 대한 정보를 담고 있는 구조체
		cmd_struct cmd;
		printf("sicsim> ");
		fgets(cmd.input, MAX_LEN, stdin);
		quit = make_command(&cmd, &h_start, &h_last, op_ptr, &first);
	}
	return 0;
}
