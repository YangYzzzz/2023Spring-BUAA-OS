#include<lib.h>

void usage() {
	debugf("rm [-xr] [path]\n");
	exit();
}

int main(int argc, char **argv) {
	int force = 0;
	int recursive = 0;
	ARGBEGIN {
	default:
		usage();
		break;
	case 'r':
		recursive = 1;
		break;
	case 'f':
		force = 1;
		break;
	}
	ARGEND
	if (argc == 0) {
		debugf("no para!\n");
		exit();
	} else {
		int r;
		for (int i = 0; i < argc; i++) {  //force 若文件中有内容, 则需force置位才能删; 若recursive置位 则和rmdir -r 一致移除下面的全部； 正常 移除指定位置的文件
			if ((r = myremove(parsepath((path_switch(argv[i]))), force, recursive, 0)) < 0) {
				debugf("rm %s wrong!\n", argv[i]);
			}
		}
	}
	return 0;
}
