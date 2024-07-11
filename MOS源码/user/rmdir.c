#include<lib.h>
void usage() {
	debugf("rmdir [name]\n");
	exit();
}
int main(int argc, char **argv) {
	int p = 0;
	ARGBEGIN {
	default:
		usage();
		break;
	case 'p':
		p = 1;
		break;
	}
	ARGEND

	if (argc == 0) {
		debugf("no para!\n");
		exit();
	} else {
		if (p == 0) {
			for (int i = 0; i < argc; i++) {
				int r = myremove(parsepath(path_switch(argv[i])), 0, 0, 1);
				if (r < 0) {
					debugf("rmdir fail!\n");
					exit();
				}
			}
		} else {
			for (int i = 0; i < argc; i++) { // /a/b/c   b/c
				while (strlen(argv[i]) > 0) {
					int r = myremove(parsepath(path_switch(argv[i])), 0, 0, 1);
					//debugf("argv : %s\n", argv[i]);
					if (r < 0) {
						debugf("rmdir fail!\n");
						exit();
					}
					if (argv[i][strlen(argv[i]) - 1] == '/') {
						argv[i][strlen(argv[i]) - 1] = 0;
					}
					int k;
					for (k = strlen(argv[i]); k >= 0; k--) {
						if (argv[i][k] == '/') {
							break;
						}
					}
					//debugf("k : %d\n", k);
					argv[i][k + 1] = 0;
				}
			}
		}
	}
	return 0;
}
