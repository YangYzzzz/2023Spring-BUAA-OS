#include<lib.h>

void usage() {
	debugf("mv [NAME] [PATH]\n");
	exit();
}

int main(int argc, char **argv) {
	ARGBEGIN {
	default:
		usage();
		break;
	}
	ARGEND
	if (argc != 2) {
		debugf("mv para num wrong!\n");
	} else {
		char name[1024];
		strcpy(name, parsepath(path_switch(argv[0])));
		char path[1024];
		strcpy(path, parsepath(path_switch(argv[1])));
		struct Stat stname, stpath;
		int k, new = 0;
		if ((k = stat(name, &stname)) < 0) {
			debugf("path wrong!\n");
			exit();
		}
		if (stname.st_isdir) { //name为目录
			if ((k = stat(path, &stpath)) < 0) { //此刻没有path
				new = 1;
				if ((k = file_mkdir(path, 0)) < 0) {
					debugf("can't find dir!\n");
					exit();
				}
			} else if (!stpath.st_isdir) {
				debugf("dir -> reg fail!\n");
				exit();
			}
		} else { //name为文件
			if ((k = stat(path, &stpath)) < 0) {
				if ((k = file_touch(path, 0)) < 0) {
					debugf("can't find reg file!\n");
					exit();
				}
					
			}
		}
		if ((k = file_move(name, path, new)) < 0) {
			debugf("mv fail!\n");
		}
	}
	return 0;
}
