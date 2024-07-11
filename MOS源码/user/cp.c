#include<lib.h>
void usage() {
	debugf("cp [-r] [NAME] [PATH]");
	exit();
}

int main(int argc, char **argv) {
	int r = 0;
	ARGBEGIN {
	default:
		usage();
		break;
	case 'r':
		r = 1;
		break;
	}
	ARGEND
	if (argc != 2) {
		debugf("para num wrong!\n");
		return 0;
	} else {
		char name[1024];
		strcpy(name, parsepath(path_switch(argv[0])));
		char path[1024];
		strcpy(path, parsepath(path_switch(argv[1]))); //深克隆 应控制块等复制一份
		struct Stat stname, stpath;
		int k, new = 0;
		/*
		if ((k = stat(name, &stname)) < 0 || (k = stat(path, &stpath)) < 0) {
			debugf("stat fail\n");
			exit();
		}
		if (!stpath.st_isdir) {
			debugf("target path is a reg file!\n");
			exit();
		}
		//debugf("stname %s \n", name);
		if (stname.st_isdir && r == 0) {
			debugf("can't copy dir!\n");
			exit();
		}
		if ((k = file_copy(name, path)) < 0) {
			debugf("copy fail!\n");
			exit();
		} */
		if ((k = stat(name, &stname)) < 0) {
			debugf("path wrong!\n");
			exit();
		}
		if (stname.st_isdir && r == 0) {
			debugf("can't copy dir %s!\n", name);
			exit();
		}
		if (stname.st_isdir && r == 1) { //name为目录
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
		if ((k = file_copy(name, path, new)) < 0) {
			debugf("copy fail!\n");
		}

	}
	return 0;
}

