#include<lib.h>
void usage() {
	debugf("touch [-p] [NAME]\n");
	exit();
}

void touch(char *path, int p) {
	int r = file_touch(path, p);
	if (r != 0) {
		debugf("touch fail!\n");
		exit();
	}
	return;
}

int main(int argc, char **argv) {
	int p = 0;
	int i;
	ARGBEGIN {
	default:
		usage();
	case 'p':
		p = 1;
		break;
	}
	ARGEND
	if (argc == 0) {
		debugf("touch error!\n");
		exit();
	} else {
		for (int i = 0; i < argc; i++) {
			touch((char *)path_switch(argv[i]), p);
		}
	}
	return 0;
}
