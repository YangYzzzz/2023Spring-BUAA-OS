#include<lib.h>
void usage() {
	debugf("mkdir [-p] [NAME]\n");
	exit();
}

void mkdir(char *path, int p) {
	//debugf(" 1 ");
	int r = file_mkdir(path, p);
	if (r != 0) {
		debugf("mkdir fail!\n");
		exit();
	}
	return;
}
int main(int argc, char **argv) {
	int p = 0;
	ARGBEGIN {
	default:
		usage();
	case 'p':
		p = 1;
		break;
	}
	ARGEND
	if (argc == 0) {
		debugf("para wrong!\n");
		exit();
	} else {
		for (int i = 0; i < argc; i++) {
			mkdir(path_switch(argv[i]), p);
		}
	}
	return 0;
}
