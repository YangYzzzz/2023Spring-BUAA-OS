#include<lib.h>
void usage() {
	exit();
}
int main(int argc, char **argv) {
	ARGBEGIN {
	default:
		usage();
	case 'd':
		break;
	}
	ARGEND
	char *pwd = getcwd();
	char tmp[1024];
	int i;
	for (i = 0;*pwd != 0; pwd++, i++) {
		tmp[i] = *pwd;
	}
	printf("%s\n", tmp);
	return 0;
}
