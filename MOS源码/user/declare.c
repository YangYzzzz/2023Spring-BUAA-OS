#include<lib.h>
//执行新一条命令时不会保留这个全局变量了 卧槽！
void usage() {
	debugf("usage: declare [-xr] [NAME [=VALUE]]\n");
	exit();
}
/*
void printall() {
	//局部
	int i;
	for (i = 0; i < 100; i++) {
		if (part[i].NAME[0] != 0) {
			printf("NAME : %s, VALUE : %s\n", part[i].NAME, part[i].VALUE);
		}
	}
}
struct Environment* alloc_envir() {
	int i;
	for (i = 0; i < 100; i++) {
		if (part[i].NAME[0] == 0) {
			return part + i;
		}
	}
	return NULL;
}
struct Environment* isexist(char *name) {
	int i;
	for (i = 0; i < 100; i++) {
		if (strcmp(part[i].NAME, name) == 0) {
			return part + i;
		}
	}
	return NULL;
}
int addpart(int readonly, char *name, char *value) {
	struct Environment *envir = isexist(name);
	if (envir == NULL) {
		envir = alloc_envir();
		if (envir == NULL) {
			debugf("no more part space!\n");
			exit();
		}
		//维护这个变量好像没意义
		strcpy(envir->NAME, name);
		strcpy(envir->VALUE, value);
		envir->isread = readonly;
		debugf("%s", part[0].NAME);
	} else {
		if (envir->isread == 0) { //非只可读
			strcpy(envir->VALUE, value);
		} else {
			debugf("can't change rdonly envir!\n");
			exit();
		}
	}
	return 0;
} */
/*
syscall_env_var
0 printall
1 set global readonly
2 set global not readonly
3 set local readonly
4 set local not readonly
*/
int main(int argc, char **argv) {
	int global = 0;
	int readonly = 0;
	ARGBEGIN {
	default:
		usage();
	case 'x':
		global = 1; //全局
		break;
	case 'r':
		readonly = 1;
		break;
	}
	ARGEND
	int r;
	if (argc == 0) {
		if ((r = syscall_set_env_var(0, 0, 0, 0)) < 0) { //cmd name value
			exit();
		}
	} else {  //应该
	for (int i = 0; i < argc; i++) {
		char name[128];
		char value[128];
		char *poi = strchr(argv[i], '=');
		if (poi == NULL) {
			strcpy(name, argv[i]);
			strcpy(value, "");
		} else {
			memcpy(name, argv[i], poi - argv[i]); //读name
			strcpy(value, poi + 1);
		}
		if (global == 0 && readonly == 0) {
			//debugf("name : %s, value : %s\n", name, value);
			if ((r = syscall_set_env_var(0, 4, name, value)) < 0) {
				exit();
			}
		} else if (global == 1 && readonly == 0) {
			if ((r = syscall_set_env_var(0, 2, name, value)) < 0) {
				exit();
			}
		} else if (global == 0 && readonly == 1) {
			if ((r = syscall_set_env_var(0, 3, name, value)) < 0) {
				exit();
			}
		} else if (global == 1 && readonly == 1) {
			if ((r = syscall_set_env_var(0, 1, name, value)) < 0) {
				exit();
			}
		}
	}
	}
	return 0;
}
		
