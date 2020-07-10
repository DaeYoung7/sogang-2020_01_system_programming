#include"dir.h"
void check_dir() {
	//현재 디렉토리 가져오기
	DIR *dir_ptr = opendir(".");
	struct dirent *file = NULL;
	struct stat dir_stat;
	int cnt = 1;

	if (!dir_ptr) {
		printf("cannot open dir");
		return;
	}
	//디렉토리 포인터의 값을 읽고 요소들 출력
	while ((file = readdir(dir_ptr)) != NULL) {
		stat(file->d_name, &dir_stat);
		//디렉토리인지, 실행파일인지 체크
		if (dir_stat.st_mode & S_IFDIR) printf("\t%s/", file->d_name);
		else if (dir_stat.st_mode & S_IXUSR) printf("\t%s.", file->d_name);
		else printf("\t%s", file->d_name);

		if ((cnt++) % 4 == 0) printf("\n");
	}
	cnt--;
	if (cnt % 4 != 0) printf("\n");
	closedir(dir_ptr);
	return;
}
