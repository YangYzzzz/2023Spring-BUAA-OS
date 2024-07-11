#include<lib.h>
int deepth = 0;
void usage() {
	exit();
}
void tree(char *path, char *name) { // /cat + aaa.txt 中间补/ 输出层次 ls就是一层 要判断一下是否是目录
	int r;
	int fd, n;
	struct File f;
	struct Stat st;
	if ((r = stat(path, &st)) < 0) {
		user_panic("stat fail!\n");
	}
	if ((fd = open(path, O_RDONLY)) < 0) {
		user_panic("open fail!\n");
	}
	if (!st.st_isdir) {
		printf("|");
		for (int i = 0; i < deepth; i++) {
			printf("----");
		}
		printf("%s\n", name);
		deepth--;
		close(fd);
		return;
	}
	while ((n = readn(fd, &f, sizeof f)) == sizeof f) {
		if (f.f_name[0]) { //寻找dir的下层 只有是目录时要输出名字 文件时在上面输出
			deepth++;
			if (f.f_type == FTYPE_DIR) {
				printf("|");
				for (int i = 0; i < deepth; i++) {
					printf("----");
				}
				printf("\033[1;34m%s\033[0m\n", f.f_name);
			}
			int len = strlen(f.f_name) + strlen(path) + 1;
			char newpath[len + 1];
			strcpy(newpath, path);
			strcpy(newpath + strlen(path), "/");
			strcpy(newpath + strlen(path) + 1, f.f_name);
			newpath[len+1] = 0;
			tree(newpath, f.f_name);
		}
	}
	deepth--;
	close(fd);
	return;
}	

int main(int argc, char **argv) { //构建树形图 深度遍历
	ARGBEGIN {
	default:
		usage();
	case 'd':
	case 'F':
	case 'l':
		break;
	}
	ARGEND
	if (argc == 0) {
		char *work = env->env_workingpath;
		printf("\033[1;34m%s\033[0m\n", work);
		tree(work, "");
	} else {
		for (int i = 0; i < argc; i++) { //两种情况 dir or reg
			char *path = parsepath(path_switch(argv[i]));
			struct Stat st;
			int r;
 		        if ((r = stat(path, &st)) < 0) {
 		             user_panic("stat fail!\n");
 		        }
			if (st.st_isdir) {
				printf("\033[1;34m%s\033[0m\n", path);
				tree(path, "");
			} else {
				printf("%s is regular file!\n", path);
			}
		}
	}
	printf("\n");
	return 0;
}
