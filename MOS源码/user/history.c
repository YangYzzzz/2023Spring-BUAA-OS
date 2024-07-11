#include<lib.h>
char buf[8192];
int main(int argc, char **argv) {
	int history = open("cmd.history", O_RDWR);
	int n, r;
	while((n = read(history, buf, sizeof buf)) > 0) {
		//for (int i = 0; i < n; i++) { //n是filesize
		//	debugf(" %d ", buf[i]);
		//}
		if ((r = write(1, buf, n)) != n) {
			user_panic("write error copying!\n");
		}
	}
	close(history);
}

