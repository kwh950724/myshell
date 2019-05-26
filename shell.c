// 1. exit 입력 시 shell 종료
// 2. a; b; c 와 같이 입력시 a, b, c 세개의 프로그램 차례대로 실행
// 3. username@hostname$ 형식의 shell prompt. username과 hostname 얻는 방법은 아래 코드 참조
// https://github.com/jongchank/syssw/blob/master/13_process_management/gethostusername.c

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>

#define MAX_LEN_LINE    50
#define LEN_HOSTNAME	30
#define CRT_SECURE_NO_WARNINGS

int main(void)
{
    char command[MAX_LEN_LINE];
    char *args[] = {command, NULL};
    int ret, status;
    pid_t pid, cpid;
    char *pos;
    char *argv[] = {NULL};

    while (true) {
        char *s;
        int len;
        char *prog[10] = {NULL, };
	int prog_count = 0;
	int exec_prog_count = 0;
	uid_t user_id;
	struct passwd *pw_name;


	
	char hostname[LEN_HOSTNAME + 1];
	// Initializing array of hostname.
	memset(hostname, 0x00, sizeof(hostname));
	gethostname(hostname, LEN_HOSTNAME);
	if(pos == NULL) printf("%s@%s:~$ ", getpwuid(getuid())->pw_name, hostname);
	else printf("%s@%s:~/%s$ ", getpwuid(getuid())->pw_name, hostname, pos);
        //printf("MyShell $ ");
	// stdin 위치에서 첫번째 개행 또는 MAX_LEN_LINE만큼 string을 복사하여 command에 저장한다.
        s = fgets(command, MAX_LEN_LINE, stdin);
        len = strlen(command);
        //printf("%d\n", len);
        if (command[len - 1] == '\n') {
            	command[len - 1] = '\0'; 
       	}
        if (s == NULL) {
            fprintf(stderr, "fgets failed\n");
            exit(1);
        }
	else if (!strcmp(s, "info")) {
	    printf("-----MYSHELL INSTRUCTIONS-----\n");
	    printf("1. info->SHOWING INSTRUCTION OF THE SHELL.\n");
	    printf("2. ls->SHOWING COMPONENT OF CURRENT DIRECTIORY.\n");
	    printf("3. vi->OPENING VI TEXT EDITOR.\n");
	    printf("4. exit->TERMINATING THE SHELL.\n");
	    printf("5. cd->MOVING TO THE DEFAULT FOLDER.\n");
	    continue;
	}
	else if (!strcmp(s, "exit")) {
	    printf("-----MYSHELL TERMINATED-----\n");
	    break;
	}
	else if (strstr(s, "; ") != NULL) {
		int i = 0;
		char *ptr = strtok(command, "; ");
		while(ptr != NULL) {
			prog[i] = ptr;
			i++;
			prog_count +=1 ;
			ptr = strtok(NULL, "; ");
		}
	}
	else {
		prog_count += 1;
		prog[0] = command;
	}

	for(int i=0; i < 10; i++) {
		if(prog[i] != NULL) {
			if(strstr(prog[i], "ls") != NULL) prog[i] = "/bin/ls";
			else if(strstr(prog[i], "vi") != NULL) prog[i] = "/usr/bin/vi";
			else if(strstr(prog[i], "cd") != NULL) {
				if(strstr(prog[i], " ") == NULL) {
					chdir(getenv("HOME"));
					pos = NULL;
					continue;
				}
				else {
					char *ptr = strchr(prog[i], ' ');
					argv[0] = ptr+1;
					pos = ptr+1;
					chdir(argv[0]);
					continue;
				}
			}	
			printf("[%s]\n", prog[i]);
		}
	}
	//printf("NUMBER OF RUNNING PROGRAM: %d\n", prog_count);

      
	for(int i=0;i<prog_count;i++) {
		args[0] = prog[i];
		printf("%s\n", args[0]);
		pid = fork();
		if (pid < 0) {
		    fprintf(stderr, "fork failed\n");
		    exit(1);
		} 
		if (pid != 0) {   /* parent */ 
		    cpid = waitpid(pid, &status, 0);
		    if (cpid != pid) {
			fprintf(stderr, "waitpid failed\n");        
		    }
		    printf("Child process terminated\n");
		    if (WIFEXITED(status)) {
			printf("Exit status is %d\n", WEXITSTATUS(status)); 
		    }
		}
		else {   /* child */
		    ret = execve(args[0], args, NULL);
		    if (ret < 0) {
			fprintf(stderr, "execve failed\n");   
			return 1;
		    }
		}
		exec_prog_count += 1;
	}
	printf("NUMBER OF EXECUTED PROGRAM: %d\n", exec_prog_count);
    }
    return 0;
}
