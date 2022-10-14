// Names: An Le, Abbas Nurie
// NetIDs: anvl, anurie
// Student IDs: 26801474, 64031491

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <signal.h>
#include <fcntl.h>

struct job {
    int job_id;
    pid_t pid;
    char status[15];
    char cmdline[80];
};

struct job jobs[5];
struct job foregroundjob;

void chld_handler(int sig) {
    pid_t child_pid;
    int status;
    while (child_pid = waitpid(-1, &status, WNOHANG|WUNTRACED) > 0) {
        int i;
        if (WIFEXITED(status) || WIFSIGNALED(status)){
            for (i=0; i<5; i++) {
                if (jobs[i].pid == child_pid) {
                    jobs[i].job_id = -2;
                    jobs[i].pid = -2;
                    strcpy(jobs[i].status, " ");
                    strcpy(jobs[i].cmdline, " ");
                }
            }
            if (child_pid == foregroundjob.pid) {
                foregroundjob.job_id = -1;
                foregroundjob.pid = -1;
                strcpy(foregroundjob.status, " ");
                strcpy(foregroundjob.cmdline, " ");
            }
        }
        else if (WIFSTOPPED(status)) {
            int i;
            int j = 0;
            for (i=0; i<5; i++) {
                if (jobs[i].pid == foregroundjob.pid) {
                    strcpy(jobs[i].status,"Stopped");
                    j = 1;
                }
            }
            int k = -1;
            if (j == 0) {
                for (i=0; i<5; i++) {
                    if (strcmp(jobs[i].status, " ") != 0 && strcmp(jobs[i].status, "Foreground") != 0)
                        k = i;
                }
                jobs[k+1].job_id = k+2;
                jobs[k+1].pid = foregroundjob.pid;
                strcpy(jobs[k+1].status, "Stopped");
                strcpy(jobs[k+1].cmdline, foregroundjob.cmdline);
            }
            foregroundjob.job_id = -1;
            foregroundjob.pid = -1;
            strcpy(foregroundjob.status, " ");
            strcpy(foregroundjob.cmdline, " ");
        }
    }
}

void handler1(int sig) {
    if (foregroundjob.pid != -1) {
        kill(foregroundjob.pid, SIGINT);
    }
}

void handler2(int sig) {
    if (foregroundjob.pid != -1) {
        kill(foregroundjob.pid, SIGTSTP);
    }
}

void printjobs() {
    int i;
    for (i=0; i<5; i++) {
        if (strcmp(jobs[i].status, "Running") == 0 || strcmp(jobs[i].status, "Stopped") == 0 )
            printf("[%d] (%d) %s %s\n", jobs[i].job_id, jobs[i].pid, jobs[i].status, jobs[i].cmdline);
    }
}

int parseline(char* cmdline, char** argv) {
    int i = 0, bg = 0;
    argv[i] = strtok(cmdline, " ");
    while (argv[i] != NULL){
        argv[++i] = strtok(NULL, " ");
    }
    argv[++i] = NULL;
    if (strcmp(argv[i-2], "&") == 0) {
        bg = 1;
    }
    return bg;
}

void eval(char* cmdline) {
    int status;
    cmdline[strcspn(cmdline, "\n")] = 0;
    char temp_str[80];
    strcpy(temp_str, cmdline);
    char* argv[80];
    int bg;
    pid_t pid;
    bg = parseline(cmdline, argv);
    if (strcmp(argv[0], "jobs") == 0 && argv[1] == NULL) {
        printjobs();
    }
    else if (strcmp(argv[0], "fg") == 0 && argv[1] != NULL && argv[2] == NULL) {
        int temp = -5;
        if (argv[1][0] == '%') {
            int k = atoi(strtok(argv[1], "%"))-1;
            if (k < 5 && k >= 0) {
                temp = jobs[k].pid;
            }
            if (temp == -2) {
                temp = -5;
            }
        }
        else {
            temp = atoi(argv[1]);
        }
        int i;
        for (i=0; i<5; i++) {
            if (jobs[i].pid == temp && strcmp(jobs[i].status, "Foreground") != 0) {
                char temp_status[15];
                strcpy(temp_status, jobs[i].status);
                strcpy(jobs[i].status,"Foreground");
                foregroundjob.job_id = 0;
                foregroundjob.pid = jobs[i].pid;
                strcpy(foregroundjob.status, jobs[i].status);
                strcpy(foregroundjob.cmdline, jobs[i].cmdline);
                kill(temp, SIGCONT);
                pause();
                if (strcpy(temp_status, jobs[i].status) != 0) {
                    pause();
                }
            }
        }
    }
    else if (strcmp(argv[0], "bg") == 0 && argv[1] != NULL && argv[2] == NULL) {
        int temp = -5;
        if (argv[1][0] == '%') {
            int k = atoi(strtok(argv[1], "%"))-1;
            if (k < 5 && k >= 0) {
                temp = jobs[k].pid;
            }
            if (temp == -2) {
                temp = -5;
            }
        }
        else {
            temp = atoi(argv[1]);
        }
        int i;
        for (i=0; i<5; i++) {
            if (jobs[i].pid == temp && strcmp(jobs[i].status, "Foreground") != 0) {
                strcpy(jobs[i].status, "Running");
                kill(temp, SIGCONT);
            }
        }
    }
    else if (strcmp(argv[0], "kill") == 0 && argv[1] != NULL && argv[2] == NULL) {
        int temp = -5;
        if (argv[1][0] == '%') {
            int k = atoi(strtok(argv[1], "%"))-1;
            if (k < 5 && k >= 0) {
                temp = jobs[k].pid;
            }
            if (temp == -2) {
                temp = -5;
            }
        }
        else {
            temp = atoi(argv[1]);
        }
        int i;
        for (i=0; i<5; i++) {
            if (jobs[i].pid == temp && strcmp(jobs[i].status, "Foreground") != 0) {
                jobs[i].job_id = -2;
                jobs[i].pid = -2;
                strcpy(jobs[i].status, " ");
                strcpy(jobs[i].cmdline, " ");
                kill(temp, SIGINT);
            }
        }
    }
    else if (strcmp(argv[0], "cd") == 0 && argv[1] != NULL && argv[2] == NULL) {
        if (chdir(argv[1]) != 0) {
            printf("Cannot change to %s.\n", argv[1]);
        }
    }
    else {
        if ((pid = fork()) == 0) {
            setpgrp();
            int o = 0;
            mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO;
            while (argv[o+1] != NULL){
                if ((strcmp(argv[o], "<") == 0)) {
                    int inFileID = open(argv[o+1], O_RDONLY, mode);
                    dup2(inFileID, STDIN_FILENO);
                }
                o++;
            }
            o = 0;
            while (argv[o+1] != NULL){
                if (strcmp(argv[o], ">") == 0) {
                    int outFileID = open (argv[o+1], O_CREAT|O_WRONLY|O_TRUNC, mode);
                    dup2(outFileID, STDOUT_FILENO);
                }
                else if (strcmp(argv[o], ">>") == 0) {
                    int outFileID = open(argv[o+1], O_CREAT|O_WRONLY|O_APPEND, mode);
                    dup2(outFileID, STDOUT_FILENO);
                }
                o++;
            }
            if (execv(argv[0], argv) < 0 && execvp(argv[0], argv) < 0) {
                printf("%s: Command not found.\n", argv[0]);
                exit(1);
            }
            exit(0);
        }
        else if (pid != -1) {
            if (!bg) {
                foregroundjob.job_id = 0;
                foregroundjob.pid = pid;
                strcpy(foregroundjob.status, "Foreground");
                strcpy(foregroundjob.cmdline, temp_str);
                pause();
            }
            else {
                int k;
                int l = -1;
                for (k=0; k<5; k++) {
                    if (strcmp(jobs[k].status, " ") != 0 && strcmp(jobs[k].status, "Foreground") != 0)
                        l = k;
                }
                jobs[l+1].job_id = l+2;
                jobs[l+1].pid = pid;
                strcpy(jobs[l+1].status, "Running");
                strcpy(jobs[l+1].cmdline, temp_str);
            }
        }
    }
}

int main() {
    char cmdline[80];
    struct job temp_job = {.job_id = -1, .pid = -1, .status = " ", .cmdline = " "};
    foregroundjob = temp_job;
    struct job temp_job1 = {.job_id = -2, .pid = -2, .status = " ", .cmdline = " "};
    struct job temp_job2 = {.job_id = -2, .pid = -2, .status = " ", .cmdline = " "};
    struct job temp_job3 = {.job_id = -2, .pid = -2, .status = " ", .cmdline = " "};
    struct job temp_job4 = {.job_id = -2, .pid = -2, .status = " ", .cmdline = " "};
    struct job temp_job5 = {.job_id = -2, .pid = -2, .status = " ", .cmdline = " "};
    jobs[0] = temp_job1;
    jobs[1] = temp_job2;
    jobs[2] = temp_job3;
    jobs[3] = temp_job4;
    jobs[4] = temp_job5;
    setpgrp();
    signal(SIGCHLD, chld_handler);
    signal(SIGINT, handler1);
    signal(SIGTSTP, handler2);
    while (1) {
        printf("prompt> ");
        fgets(cmdline, 80, stdin);
        if (strcmp(cmdline, "quit\n") == 0) {
            exit(0);
        }
        else if (strcmp(cmdline, "\n") != 0) {
            eval(cmdline);;
        }
    }
}