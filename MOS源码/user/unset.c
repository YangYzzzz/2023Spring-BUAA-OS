#include <lib.h>
void usage() {
	debugf("unset [NAME]\n");
	exit();
}
int main(int argc, char **argv) {
	ARGBEGIN {
	default:
		usage();
		break;
	}
	ARGEND
	for (int i = 0; i < argc; i++) {
		int r = syscall_set_env_var(0, 5, argv[i], 0);
	}
	return 0;
}
