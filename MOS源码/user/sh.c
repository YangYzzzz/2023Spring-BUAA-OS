#include <args.h>
#include <lib.h>
#define WHITESPACE " \t\r\n"
#define SYMBOLS "<|>&;()"

/* Overview:
 *   Parse the next token from the string at s.
 *
 * Post-Condition:
 *   Set '*p1' to the beginning of the token and '*p2' to just past the token.
 *   Return:
 *     - 0 if the end of string is reached.
 *     - '<' for < (stdin redirection).
 *     - '>' for > (stdout redirection).
 *     - '|' for | (pipe).
 *     - 'w' for a word (command, argument, or file name).
 *
 *   The buffer is modified to turn the spaces after words into zero bytes ('\0'), so that the
 *   returned token is a null-terminated string.
 */
char w[1024][512];
int wcnt;
char cmd[1024][16] = {"cd", "pwd", "mv", "cp", "rm", "rmdir", "touch", "mkdir", "tree", "declare", "unset", "cat", "echo", "sh", "ls"};
int cmd_cnt = 15;
char level[1024][64];
char may[1024][10];
int may_cnt;
int level_cnt;
void search_maybe_string(char *s, int n) {
	//n = 0 代表搜cmd和Level，n = 1代表前面有/ 只搜索level
	may_cnt = 0;
	if (strlen(s) == 0) {
		return;
	}
	char tmp1[1024];
	for (int j = 0; j < level_cnt; j++) {
		strcpy(tmp1, level[j]);
		tmp1[strlen(s)] = 0;
		if (strcmp(tmp1, s) == 0) {
			strcpy(may[may_cnt], level[j]);
			may_cnt++;
		}
	}
	if (n == 0) {
		for (int j = 0; j < 15; j++) {
			//debugf("cmd[j] %s\n", cmd[j]);
			strcpy(tmp1, cmd[j]);
			tmp1[strlen(s)] = 0;
			if (strcmp(tmp1, s) == 0) {
				strcpy(may[may_cnt], cmd[j]);
				may_cnt++;
			}
		}
	}
}

void get_level(char *path) {
	int fd, r, n;
	level_cnt = 0;
	struct Stat st;
	struct File f;
	if ((r = stat(path, &st)) < 0) {
		return;
	}
	if (!st.st_isdir) {
		return;
	} else {
		fd = open(path, O_RDONLY);
		while ((n = readn(fd, &f, sizeof f)) == sizeof f) {
			if (f.f_name[0]) {
				strcpy(level[level_cnt], f.f_name);
				level_cnt++;
			}
		}
	}
	return;
}

int _gettoken(char *s, char **p1, char **p2) {
	int c = 0;
	*p1 = 0;
	*p2 = 0;
	if (s == 0) { //读到0结束
		return 0;
	}

	while (strchr(WHITESPACE, *s)) {
		*s++ = 0;
	} //在这封0
	if (*s == 0) {
		return 0;
	}

	if (strchr(SYMBOLS, *s)) {
		int t = *s;
		*p1 = s;
		*s++ = 0; //（*s）++ 先赋值0，再加一
		*p2 = s;
		return t;
	}
	//p1 p2将头尾找到
	wcnt++;
	while (*s && !strchr(WHITESPACE SYMBOLS, *s)) { //读过整个单词
		while (strchr("\"", *s)) {
			s++;
			while(*s && !strchr("\"", *s)) {
				if (strchr("\\", *s) && strchr("\"\\", *(s + 1))) { // \\ \" 
					s++;
				}
				w[wcnt][c++] = *s;
				s++;
			}
			s++;
		}
		if (strchr("\\", *s) && strchr("\"\\", *(s + 1))) {
			s++;
		}
		//debugf(" %c ", *s);
		w[wcnt][c++] = *s;
		s++; //不用封0么？？
	}
	*p2 = s;
	w[wcnt][c] = 0;
	*p1 = w[wcnt];
	return 'w';
}

int gettoken(char *s, char **p1) { //p1是得到的词 s是下次继续的位置
	static int c, nc;
	static char *np1, *np2;
	if (s) {
		nc = _gettoken(s, &np1, &np2); //得到第一个词
		return 0;
	}
	c = nc; //本轮读上一轮读到的
	*p1 = np1;
	nc = _gettoken(np2, &np1, &np2); //np2是下次的起点
	return c;
}

#define MAXARGS 128

int parsecmd(char **argv, int *rightpipe) {
	int argc = 0;
	while (1) {
		char *t;
		int fd, r, a;
		int c = gettoken(0, &t);
		//debugf(" token: %s\n", t);
		switch (c) {
		case 0:
			return argc;

		case ';': //依次执行左右两边的指令
			if ((r = fork()) < 0) {
				debugf("fork error!\n");
				exit();
			}
			if (r > 0) { //父进程
				wait(r);
				return parsecmd(argv, rightpipe);
			} else {
				//debugf(" par : %d, child : %d\n", env->env_id, r);
				return argc;
			}
			
		case 'w':
			if (argc >= MAXARGS) {
				debugf("too many arguments\n");
				exit();
			}
			argv[argc++] = t;
			break;
		case '<':
			if (gettoken(0, &t) != 'w') {
				debugf("syntax error: < not followed by word\n");
				exit();
			}
			// Open 't' for reading, dup it onto fd 0, and then close the original fd.
			/* Exercise 6.5: Your code here. (1/3) */
			char *tmp = parsepath(path_switch(t));
			file_touch(tmp, 0);
			fd = open(tmp, O_RDWR);
			dup(fd, 0);
			close(fd);
			break;
			user_panic("< redirection not implemented");

			break;
		case '>':
			a = gettoken(0, &t);
			if (a == '>') {
				if (gettoken(0, &t) != 'w') {
					debugf("syntax error: > not followed by word\n");
					exit();
				}
				char *tmp = parsepath(path_switch(t));
				file_touch(tmp, 0);
				fd = open(tmp, O_RDWR);
				seek(fd, get_file_size(fd));
			} else {
				if (a != 'w') {
					debugf("syntax error: > not followed by word\n");
					exit();
				}
				// Open 't' for writing, dup it onto fd 1, and then close the original fd
				char *tmp = parsepath(path_switch(t));
				file_touch(tmp, 0);
				fd = open(tmp, O_RDWR);
				ftruncate(fd, 0);
			}
			dup(fd, 1);
			close(fd);
			break;
			user_panic("> redirection not implemented");

			break;
		case '|':;
			/*
			 * First, allocate a pipe.
			 * Then fork, set '*rightpipe' to the returned child envid or zero.
			 * The child runs the right side of the pipe:
			 * - dup the read end of the pipe onto 0
			 * - close the read end of the pipe
			 * - close the write end of the pipe
			 * - and 'return parsecmd(argv, rightpipe)' again, to parse the rest of the
			 *   command line.
			 * The parent runs the left side of the pipe:
			 * - dup the write end of the pipe onto 1
			 * - close the write end of the pipe
			 * - close the read end of the pipe
			 * - and 'return argc', to execute the left of the pipeline.
			 */
			int p[2];
			/* Exercise 6.5: Your code here. (3/3) */
			if ((r = pipe(p)) < 0) {
				debugf("pipe fail\n");
				exit();
			}
			if ((*rightpipe = fork()) < 0) {
				debugf("fork fail!!\n");
				exit();
			}
			if (*rightpipe == 0) { //子进程 
				dup(p[0], 0); //将读端映射到标准输入
				close(p[0]);
				close(p[1]);
				//debugf(" p[0]:%d\np[1]:%d\n", p[0], p[1]);
				return parsecmd(argv, rightpipe); //子进程执行右面的操作 将输入映射到标准输入
			} else {
				dup(p[1], 1);
				close(p[1]);
				close(p[0]);
				//debugf(" p[0]:%d\np[1]:%d\n", p[0], p[1]);
				return argc; //父进程执行左面的操作 将输出映射到标准输出 输入输出是一个地方
				//标准输入与输出相连，即实现了父进程左侧的输出恰好是右侧子进程的输入 完成管道机制
			}
			user_panic("| not implemented");

			break;
		case '&':
			if ((r = fork()) < 0) {
				debugf(" & fork error!!\n");
				exit();
			}
			if (r > 0) { //父进程执行右面，当右面执行完毕，即可开始下一次轮询获取指令 
				return parsecmd(argv, rightpipe);
			} else { //子进程执行左面
				return argc;
			}
			break;
		}
	}

	return argc;
}

void runcmd(char *s) {
	wcnt = -1;
	gettoken(s, 0); //空读一个

	char *argv[MAXARGS];
	int rightpipe = 0;
	int argc = parsecmd(argv, &rightpipe);
	if (argc == 0) {
		return;
	}
	argv[argc] = 0;
	if (!strstr(argv[0], ".b")) {
		char newargv[20];
		//memset(newargv, 0, 10);
		strcpy(newargv, argv[0]);
		int len = strlen(newargv);
		newargv[len] = '.';
		newargv[len+1] = 'b';
		newargv[len+2] = 0;
		argv[0] = newargv;
	}
	for (int i = 0; i < argc; i++) {
		if (argv[i][0] == '$') {
			char *tmp = syscall_get_env_var(0, argv[i] + 1);
			if (tmp == NULL) {
				debugf("can't parse env var!\n");
				return;
			}
			strcpy(argv[i], tmp);
		}
	}
	//debugf("argc : %d, argv[0] : %s, argvlen : %d\n", argc, argv[0], strlen(argv[0]));
	int child = spawn(argv[0], argv); //argv[0]是对应指令路径 argv是要为指令main函数设置的参数
	close_all();
	if (child >= 0) {
		wait(child);
	} else {
		debugf("spawn %s: %d\n", argv[0], child);
	}
	if (rightpipe) {
		wait(rightpipe);
	}
	//debugf(" envid exit: %d\n", env->env_id);
	exit();
}

void uphistory(char *buf1, int *hispointer) {
	memset(buf1, 0, 1024);
	if (*hispointer == 0) {
		//debugf("no up!");
		return;
	}
	int history = open("/cmd.history", O_RDWR); //从0开始
	char file[*hispointer];
	int last = 0, i;
	int n = read(history, file, *hispointer); //file找临近末尾的\n
	//debugf(" %d ", file[*hispointer - 1]); 换行
	for (int i = *hispointer - 2; i >= 0; i--) {
		if (file[i] == '\n') {
			last = i + 1;
			break;
		} else if (i == 0) {
			last = 0;
			break;
		}
	}
	//从Last 到 hispointer
	for (i = last; i < *hispointer - 1; i++) {
		debugf("%c", file[i]);
		buf1[i - last] = file[i];
	}
	buf1[i - last] = '\0'; //把\n读走
	*hispointer = last;
	close(history);
	return;
}

void downhistory(char *buf1, int *hispointer) { //也可以选择把history读入到数组中 避免每次打开文件的开销
	memset(buf1, 0, 1024);
	int history = open("/cmd.history", O_RDWR);
	seek(history, *hispointer);
	int r;
	int i, tmp;
	for (i = 0; i < 1024; i++) {
		r = read(history, buf1 + i, 1);
		//debugf(" %d ", buf1[i]);
		if (buf1[i] == 0) { //适用于一开始便在文件末尾处
			//debugf("no down!");
			if (i != 0) {
				debugf("down wrong!");
			}
			return;
		}
		if (buf1[i] == '\n') {
			//debugf(" nextline ");
			tmp = *hispointer + i + 1; //给pointer赋上值
			break;
		}
	}
	memset(buf1, 0, 1024); //要回显的是再下一条
	for (i = 0; i < 1024; i++) {
		r = read(history, buf1 + i, 1);
		if (buf1[i] == 0) {
			//debugf("no down!");
			if (i != 0) {
				debugf("down wrong!\n");
			}
			*hispointer = tmp;
			return;
		}
		if (buf1[i] == '\n') {
			*hispointer = tmp;
			break;
		}
		debugf("%c", buf1[i]);
	}
	buf1[i] = 0;
	close(history);
	return;
}

void readline(char *buf, char *buf1, u_int n) { //研究逻辑
	int r;
	memset(buf, 0, 1024);
	memset(buf1, 0, 1024);
	int pointer = 0; //指向光标所在位置
	int max = 0;
	int history = open("/cmd.history", O_RDWR);
	int hispointer = get_file_size(history); //记录file的屁股
	close(history);
	int i, j, k, flag = 0;
	for (i = 0; i < n; i++) {
		if ((r = read(0, buf + i, 1)) != 1) { //从标准读入中读取数据
			if (r < 0) {
				debugf("read error: %d\n", r);
			}
			exit();
		}
		// 0x25 LEFT ARROW 键 
		//0x26 UP ARROW 键 
		//0x27 RIGHT ARROW 键 
		//0x28 DOWN ARROW 键 
		if (buf[i] != 9) {
			flag = 0;
		}
		if (buf[i] == '\b' || buf[i] == 0x7f) { //三种特殊情况
			//buf1[pointer] = ' ';
			if (pointer != 0) { //删除 max--
				char tmp[128];
				strcpy(tmp, buf1 + pointer); //从pointer到最后暂存起来
				if (buf[i] != '\b') { //输出一个回退字符 回退光标同步
					debugf("\b"); //到后面一格
				}
				for (j = 0; j < max - pointer; j++) {
					debugf("%c", tmp[j]);
				}
				printf(" ");
				for (j = 0; j < max - pointer + 1; j++) {
					debugf("\b");
				} //多回退一个
				pointer--;
				for (j = 0, k = pointer; tmp[j] != 0; j++, k++) {
					buf1[k] = tmp[j]; //修改buf1中内容
				}
				buf1[k] = 0;
				max--;
			}
			continue;
		}
		if (buf[i] == 27) {
			i++;
			r = read(0, buf+i, 1);
			i++;
			r = read(0, buf+i, 1);
			if (buf[i] == 68) {
				if (pointer != 0) {
					pointer--;
				} else {
					debugf(" ");
				}
			} else if (buf[i] == 67) {
				if (pointer < max) {
					pointer++;
				} else {
					debugf("\b");
				}
			} else if (buf[i] == 65) { //up
				//debugf("\n21373037:(%s)$ ", env->env_workingpath); //往上滚了 所以要换行一下到原来的位置
				debugf("%c%c%c", 27, 91, 66); //下移
				for (j = 0; j < pointer; j++) {
					debugf("\b"); //没刷掉
				} 
				for (j = 0; j < max; j++) {
					debugf(" ");
				}
				for (j = 0; j < max; j++) {
					debugf("\b");
				}
				uphistory(buf1, &hispointer);
				pointer = strlen(buf1);
				max = pointer;
				//debugf("poi : %d\n ", pointer);
				continue;
			} else if (buf[i] == 66) {
				//debugf("%c%c%c", 27, 91, 65); //上移
				//debugf("\n21373037:(%s)$ ", env->env_workingpath);
				for (j = 0; j < pointer; j++) {
					debugf("\b");
				}
				for (j = 0; j < max; j++) {
					debugf(" ");
				}
				for (j = 0; j < max; j++) {
					debugf("\b");
				}
				downhistory(buf1, &hispointer);
				pointer = strlen(buf1);
				max = pointer;
				//debugf("poi : %d\n", pointer);
				continue;
			}
			continue;
		} 
		if (buf[i] == 9) {
			if (flag == 1) {
				debugf("\nyou maybe want: ");
				for (int j = 0; j < may_cnt; j++) {
        	                        debugf("%s ", may[j]);
	                        }
				debugf("\n");
				buf1[0] = 0; //封0
				return;
			}
			flag = 1;
			char search_path[1024];
			char lack_str[1024];
			char para1[1024];
			int tmp1 = pointer, firstcut = pointer;
			int i1;
			int mode = 0;
			for (; buf1[firstcut] != ' ' && buf1[firstcut] != '/' && firstcut >= 0; firstcut--) {
			}
			if (buf1[firstcut] == '/') {
				mode = 1;
			}
			firstcut++;
			for (; buf1[tmp1] != ' ' && tmp1 >= 0; tmp1--) {
			}
			for (i1 = 0, tmp1 = tmp1 + 1; tmp1 < pointer; tmp1++, i1++) {
				para1[i1] = buf1[tmp1];
			} 
			para1[i1] = 0;
			char *t = parsepath(path_switch(para1)); // /yang/b
			for (i1 = strlen(t); t[i1] != '/'; i1--) {
			}
			strcpy(lack_str, t + i1 + 1);
			t[i1] = 0;
			strcpy(search_path, t);
			if (search_path[0] == 0) {
				search_path[0] = '/';
				search_path[1] = 0;
			}
			get_level(search_path);
			search_maybe_string(lack_str, mode);
			int minlen = 1024, flagminstr = 0;
			char minstr[1024];
			for (i1 = 0; i1 < may_cnt; i1++) {
				if (strlen(may[i1]) < minlen) {
					minlen = strlen(may[i1]);
				}
			} //找到最小
			for (; minlen >= 0 && minlen != 1024; minlen--) {
				char tmp1[1024];
				strcpy(tmp1, may[0]);
				tmp1[minlen] = 0;
				flagminstr = 0;
				for (i1 = 1; i1 < may_cnt; i1++) {
					char tmp2[1024];
					strcpy(tmp2, may[i1]);
					tmp2[minlen] = 0;
					if (strcmp(tmp1, tmp2) != 0) {
						flagminstr = 1;
						break;
					}
				}
				if (flagminstr == 0) {
					strcpy(minstr, tmp1);
					break;
				}
			}

			if (may_cnt != 0) { //有唯一解
				int diff = strlen(minstr) - strlen(lack_str);
				char tmp[128];
				strcpy(tmp, buf1 + pointer);
				strcpy(buf1 + firstcut, minstr);
				strcpy(buf1 + firstcut + strlen(minstr), tmp);
				for (int k = firstcut; k < pointer; k++) {
					debugf("\b");
				}
				for (int k = 0; k < strlen(minstr); k++) {
					debugf("%c", minstr[k]);
				}
				for (int k = 0; k < strlen(tmp); k++) {
					debugf("%c", tmp[k]);
				}
				for (int k = 0; k < strlen(tmp); k++) {
					debugf("\b");
				}
				pointer = pointer + diff;
                                max = max + diff;
			}
			//debugf("lack : %s\n", lack_str);
			//debugf("search_path : %s\n", search_path);
			continue;
		}
		if (buf[i] == '\r' || buf[i] == '\n') {
			buf[i] = 0;
			return;
		}
		char tmp[128];
		strcpy(tmp, buf1 + pointer); //暂存指针之后所有内容
		//printf("%c", buf[i]); //正常情况
		buf1[pointer] = buf[i];
		pointer++;
		for (j = 0, k = pointer; j < strlen(tmp); j++, k++) {
			debugf("%c", tmp[j]);
			buf1[k] = tmp[j];
		}
		for (j = 0; j < strlen(tmp); j++) {
			debugf("\b");
		}
		max++; //记录最大长度
	}
	debugf("line too long\n");
	while ((r = read(0, buf, 1)) == 1 && buf[0] != '\r' && buf[0] != '\n') {
		;
	}
	buf[0] = 0;
}

char buf[1024];
char buf1[1024];
void usage(void) {
	debugf("usage: sh [-dix] [command-file]\n");
	exit();
}

int main(int argc, char **argv) {
	int r;
	int interactive = iscons(0); //输入是否被重定向
	int echocmds = 0;
	int history;
	syscall_set_shid(0);
	//debugf("%d", env->env_id);
	if ((history = open("/cmd.history", O_CREAT)) < 0) {
		debugf("create error!\n");
		exit();
	}
	close(history);
	debugf("\033[1;93m !~~_________ _      _________~~!       __________.                   __________        _______ __          ______ ________ ______ _                             \n");
	debugf("   /_::______/\\     /_::____/ |        /________ / \\                 /_________/ \\      /_______ /\\        /::____________________/ !                                \n");
	debugf("   \\        \\  \\   /       /  /       /          \\  \\     QAQ       [          \\  \\     |       ]  ]      /                      ! _!                 \n");
	debugf("    \\        \\  \\ /       /  /       /    /==\\    \\  \\              [           \\  \\   |        ]  ]      |       __ ____________!/                              \n");
	debugf("     \\        \\__/       /  /       /    /  ! \\    \\  \\             [            \\  \\ _|        ]  ]      |       |  1                               \n");
	debugf("      \\~              ~~/ .>       /    /  !   \\    \\  \\            [             \\  __|        ]  ]      |       |  !      ____________.                      \n");
	debugf("        |          :|  .>         /    / _!_____\\    \\  \\          [     ...      ...        ]   ]        |       |  !     /__________  !          \n");
	debugf("        |          :|  |         /    /_/________\\    \\  \\         [                         ]   ]        |       |  !    |_____      ]  !                \n");
	debugf("        |          :|  |        /                      \\  \\        [          /!\\            ]   ]        |       |  !          !     ]  !              \n");
	debugf("        |          :|  |       /       __________       \\  \\       [         / ! \\           ]   ]        |       | ~_______ __ !     ]  !           \n");
	debugf("        |          :|  |      /       /  |       \\       \\_-\\     [         / ~ ~ \\         ]   ]         |       |_/___________!     |  !        \n");
	debugf("        |          :|  |     /       /  /         \\       \\  @    [        /  /    \\        ]   ]          |                          |  !              \n");
	debugf("        |____~~___ :|_/      |______/__/           \\ ______|@     [_______/__/      \\_______]_ /            [________~~_________~~_____|./            \n");
	debugf("                                                                                                                 \n");
	debugf("                                                                                                                         \n");
	debugf("\n\033[1;36m::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	debugf("::                                                                                                                                                            ::\n");
	debugf("::                                                             Welcome to Yang's OS!                                                                          ::\n");
	debugf("::                                                                                                                                                            ::\n");
	debugf("::                                            Support Command : cat ls echo halt tree touch mkdir cd pwd                                                      ::\n");
	debugf("::                                                        cp mv rm rmdir history declare unset                                                                ::\n");
	debugf("::                                                  Other Function :  Auto-completion $ > >> < | \\                                                            ::\n");
	debugf("::                                                             Designer : Yangyangyang                                                                        ::\n");
	debugf("::                                                                                                                                                            ::\n");
	debugf("::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	ARGBEGIN {
	case 'i':
		interactive = 1;
		break;
	case 'x':
		echocmds = 1;
		break;
	default:
		usage();
	}
	ARGEND

	if (argc > 1) {
		usage();
	}
	if (argc == 1) {
		close(0);
		if ((r = open(argv[1], O_RDONLY)) < 0) {
			user_panic("open %s: %d", argv[1], r);
		}
		user_assert(r == 0);
	}
	for (;;) {
		if (interactive) {
			printf("\033[1;32mgit@21373037\033[0m:\033[1;34m%s\033[0m \033[31m(lab6-challenge)\033[0m$ ", env->env_workingpath);
		}
		readline(buf, buf1, sizeof buf);
		history = open("/cmd.history", O_RDWR);
                int size = get_file_size(history);
		seek(history, size);
                write(history, buf1, strlen(buf1));
                write(history, "\n", 1);
                close(history);
		if (buf1[0] == 'c' && buf1[1] == 'd') {
			char *argv[10];
			gettoken(buf1, 0);
			int argc = parsecmd(argv, 0);
			if (argc != 2) {
				debugf("cd para wrong!\n");
			}
			if (argv[1][0] == '$') {
	                        argv[1] = syscall_get_env_var(0, argv[1] + 1);
		                if (argv[1] == NULL) {
	                               debugf("can't parse env var!\n");
	                        }
	                }
			int r = chdir(argv[1]); //要改变的路径
			if (r < 0) {
				debugf("cd fail!\n");
			}
			continue;
		}
		if (buf[0] == '#') {
			continue;
		}
		if (echocmds) {
			printf("# %s\n", buf);
		}
		if ((r = fork()) < 0) {
			user_panic("fork: %d", r);
		}
		if (r == 0) {
			runcmd(buf1);
			exit();
		} else {
			wait(r);
			//debugf(" wait over!\n");
		}
	}
	return 0;
}

//char total[32][16] = {"cd", "pwd", "mv", "cp", "rm", "rmdir", "touch", "mkdir", "tree", "declare", "unset", "cat", "echo", "sh", "ls"}; 
