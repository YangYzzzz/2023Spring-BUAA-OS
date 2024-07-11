#include <stdlib.h>
#include <unistd.h>

int fildes[2];
char buf[100];
int status;

int main(){
	status = pipe(fildes);
	if (status == -1 ) {
	printf("error\n");
	}

	
	switch (fork()) {
		case -1:
		break;


		case 0: /* 子进程 - 作为管道的读者 */
		close(fildes[1]); /* 关闭不用的写端 */
		read(fildes[0], buf, 100); /* 从管道中读数据 */
		printf("child-process read:%s",buf); /* 打印读到的数据 */
		close(fildes[0]); /* 读取结束，关闭读端 */
		exit(EXIT_SUCCESS);


		default: /* 父进程 - 作为管道的写者 */
		close(fildes[0]); /* 关闭不用的读端 */
		write(fildes[1], "Hello world\n", 12); /* 向管道中写数据 */
		close(fildes[1]); /* 写入结束，关闭写端 */
		exit(EXIT_SUCCESS);
	}
}
