// 1. exit 입력 시 shell 종료
// 2. a; b; c 와 같이 입력시 a, b, c 세개의 프로그램 차례대로 실행
// 3. username@hostname$ 형식의 shell prompt. username과 hostname 얻는 방법은 아래 코드 참조
// https://github.com/jongchank/syssw/blob/master/13_process_management/gethostusername.c

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <fcntl.h>

#define MAX_LEN_LINE    50
#define LEN_HOSTNAME	30
#define CRT_SECURE_NO_WARNINGS

int main(void)
{
    char command[MAX_LEN_LINE];
    char *args[] = {command, NULL};
    int ret, status;
    pid_t pid, cpid;
    char *argv_cd[] = {NULL};
    char *path[] = {NULL};
    int pos = 0;


    while (true) {
        char *s;
        int len;
        char *prog[10] = {NULL, };
	int prog_count = 0;
	int exec_prog_count = 0;
	uid_t user_id;
	struct passwd *pw_name;
        char *argv_rm[] = {NULL};
	char *argv_mkdir[] = {NULL};
	char *argv_mv[] = {NULL, NULL, NULL};
	
	char hostname[LEN_HOSTNAME + 1];
	// Initializing array of hostname.
	memset(hostname, 0x00, sizeof(hostname));
	gethostname(hostname, LEN_HOSTNAME);

	if(pos == 0) printf("%s@%s:~$ ", getpwuid(getuid())->pw_name, hostname);
	else printf("%s@%s:~/%s$ ", getpwuid(getuid())->pw_name, hostname, path[0]);

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
	    printf("4. cd->MOVING TO THE DEFAULT DIRECTORY OR SPECIFIC DIRECTORY.\n");
	    printf("5. rm->REMOVING THE SPECIFIC FILE OR DIRECTORY.\n");
	    printf("6. mkdir->CREATING THE NEW DIRECTORY.\n");
	    printf("7. cp->COPYING THE FILE TO CURRENT DIRECTORY.\n");
	    printf("8. exit->TERMINATING THE SHELL.\n");
	    continue;
	}
	else if (!strcmp(s, "exit")) {
	    printf("-----MYSHELL TERMINATED-----\n");
	    break;
	}
	else if (strstr(s, "; ") != NULL) {
		int i = 0;
		char *ptr = strtok(command, ";\0");
		while(ptr != NULL) {
			if(i==0) prog[i] = ptr;
			else prog[i] = ptr + 1;
			i++;
			prog_count += 1;
			ptr = strtok(NULL, ";\0");
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
			printf("[%s]\n", prog[i]);
		}
	}
	//printf("NUMBER OF RUNNING PROGRAM: %d\n", prog_count);

      
	for(int i=0;i<prog_count;i++) {
		if(strstr(prog[i], "cd") != NULL) {
			if(strstr(prog[i], " ") == NULL) {
				chdir(getenv("HOME"));
				pos = 0;
			}
			else {
				char *ptr = strchr(prog[i], ' ');
				argv_cd[0] = ptr+1;
				printf("%s\n", ptr+1);
				path[0] = ptr+1;
				int result = chdir(argv_cd[0]);
				if(result == -1) {
					printf("NOT EXISTING DIRECTORY, TRY AGAIN.\n");
					if(pos == 0) pos = 0;
				}
				else pos = 1;
			}
			exec_prog_count += 1;
			continue;
		}
		else if(strstr(prog[i], "rm") != NULL) {
			if(strstr(prog[i], " ") == NULL) {
				printf("PLEASE ENTER EXACT FILE OR DIRECTORY NAME.\n");
			}
			else {
				char *ptr = strchr(prog[i], ' ');
				argv_rm[0] = ptr+1;
				int result = remove(argv_rm[0]);
				if(result == -1) printf("NOT EXISTING FILE OR DIRECTORY, TRY AGAIN.\n");
				else printf("%s HAS BEEN REMOVED.\n", argv_rm[0]);
			}
			continue;
			exec_prog_count += 1;
		}
		else if(strstr(prog[i], "mkdir") != NULL) {
			if(strstr(prog[i], " ") == NULL) {
				printf("PLEASE ENTER DIRECTORY NAME.\n");
			}
			else {
				char *ptr = strchr(prog[i], ' ');
				argv_mkdir[0] = ptr+1;
				int result = mkdir(argv_mkdir[0], 0755);
				if(result == -1) printf("FAILED TO CREATE FOLDER, PLEASE CHECK NAME IS NOT DUPLICATED.\n");
				else printf("%s HAS BEEN CREATED.\n", argv_mkdir[0]);
			}
			continue;
			exec_prog_count += 1;
		}
		else if(strstr(prog[i], "mv") != NULL) {
			int i = 0;
			char *ptr = strtok(prog[i], " ");
			while(ptr != NULL) {
				if(i != 0) argv_mv[i] = ptr;
				i++;
				ptr = strtok(NULL, " ");
			}
			int fd1, fd2;
			int r_size, w_size;
			char buf[100];
			fd1 = open(argv_mv[1], O_RDONLY);
			fd2 = open(argv_mv[2], O_RDWR | O_CREAT | O_EXCL, 0664);

			r_size = read(fd1, buf, 100);
			w_size = write(fd2, buf, r_size);
			while(r_size == 100) {
				r_size = read(fd1, buf, 100);
				w_size = write(fd2, buf, r_size);
			}
			unlink(argv_mv[1]);
			exec_prog_count += 1;
			continue;
		}

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
