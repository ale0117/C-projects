// Names: An Le, Abbas Nurie
// NetIDs: anvl, anurie
// Student IDs: 26801474, 64031491
// An : Lecture A, Abbas : Lecture B

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int heap[64];

void parseline(char* cmdline, char** argv) {
    int i = 0;
    argv[i] = strtok(cmdline, " ");
    while (argv[i] != NULL) {
        argv[++i] = strtok(NULL, " ");
    }
    argv[++i] = NULL;
}

void mymalloc(int size) {
    if (size == 0) 
        return;
    int i = 0;
    while ((i<64) && ((heap[i] & 1) || ((heap[i] / 2) < size+2))) {
        i+= heap[i] / 2;
    }
    if (i<64) {
        int newsize = size+2;
        int oldsize = heap[i] / 2;
        heap[i] = (newsize * 2) | 1;
        heap[i+newsize-1] = (newsize * 2) | 1;
        printf("%d\n", i+1);
        if (newsize < oldsize) {
            heap[i+newsize] = (oldsize - newsize) * 2;
            heap[i+oldsize-1] = (oldsize - newsize) * 2;
        }
    }
}

void myfree(int index) {
    int i;
    int start = index - 1;
    for (i=index; i<start+(heap[start]/2)-1; i++) {
        heap[i] = 0;
    }
    heap[start] = heap[start] & -2;
    heap[start+(heap[start]/2)-1] = heap[start+(heap[start]/2)-1] & -2;
    if (start-1 >= 0 && start+(heap[start]/2) <= 63 && (heap[start-1] & 1) == 0 && (heap[start+(heap[start]/2)] & 1) == 0) {
        int temp1 = heap[start];
        int temp2 = heap[start-1];
        int temp3 = heap[start+(heap[start]/2)];
        heap[start-(temp2/2)] = temp1 + temp2 + temp3;
        heap[start+(temp1/2)+(temp3/2)-1] = temp1 + temp2 + temp3;
        heap[start+(temp1/2)-1] = 0;
        heap[start+(temp1/2)] = 0;
        heap[start] = 0;
        heap[start-1] = 0;
    }
    else if (start-1 >= 0 && (heap[start-1] & 1) == 0) {
        int temp1 = heap[start];
        int temp2 = heap[start-1];
        heap[start-(temp2/2)] = temp1 + temp2;
        heap[start+(temp1/2)-1] = temp1 + temp2;
        heap[start] = 0;
        heap[start-1] = 0;
    }
    else if (start+(heap[start]/2) <= 63 && (heap[start+(heap[start]/2)] & 1) == 0) {
        int temp1 = heap[start];
        int temp2 = heap[start+(heap[start]/2)];
        heap[start+(heap[start]/2)+(temp2/2)-1] = temp1 + temp2;
        heap[start] = temp1 + temp2;
        heap[start+(temp1/2)-1] = 0;
        heap[start+(temp1/2)] = 0;
    }
}

void myblocklist() {
    int i = 0;
    while (i<64) {
        if (heap[i] & 1) {
            printf("%d, %d, allocated.\n", i+1, heap[i]/2-2);
        }
        else {
            printf("%d, %d, free.\n", i+1, heap[i]/2-2);
        }
        i+= heap[i] / 2;
    }
}

void mywritemem(int index, char* str) {
    int i = 0;
    int flag = 0;
    while (i<64 && (index < i || index > i+heap[i]/2-1)) {
        i+= heap[i] / 2;
    }
    if (i<64 && (heap[i] & 1) == 1) {
        flag = 1;
    }
    if (flag) {
        for (i=0; i<strlen(str); i++) {
            heap[index+i] = str[i];
        }
    }
}

void myprintmem(int index, int num_chars) {
    int i;
    for (i=index; i<index+num_chars; i++) {
        if (i == index+num_chars-1) {
            printf("%X\n", heap[i]);
        }
        else {
            printf("%X ", heap[i]);
        }
    }
}

void eval(char* cmdline) {
    char* argv[80];
    cmdline[strcspn(cmdline, "\n")] = 0;
    parseline(cmdline, argv);
    if (strcmp(argv[0], "malloc") == 0 && argv[1] != NULL && argv[2] == NULL) {
        mymalloc(atoi(argv[1]));
    }
    else if (strcmp(argv[0], "free") == 0 && argv[1] != NULL && argv[2] == NULL) {
        myfree(atoi(argv[1]));
    }
    else if (strcmp(argv[0], "blocklist") == 0 && argv[1] == NULL) {
        myblocklist();
    }
    else if (strcmp(argv[0], "writemem") == 0 && argv[1] != NULL && argv[2] != NULL && argv[3] == NULL) {
        mywritemem(atoi(argv[1]), argv[2]);
    }
    else if (strcmp(argv[0], "printmem") == 0 && argv[1] != NULL && argv[2] != NULL && argv[3] == NULL) {
        myprintmem(atoi(argv[1]), atoi(argv[2]));
    }
}

int main(){ 
    int i;
    heap[0] = 128;
    for (i=1; i<63; i++) {
        heap[i] = 0;
    }
    heap[63] = 128;
    char cmdline[80];
    while (1) {
        printf(">");
        fgets(cmdline, 80, stdin);
        if (strcmp(cmdline, "quit\n") == 0) {
            exit(0);
        }
        else if (strcmp(cmdline, "\n") != 0) {
            eval(cmdline);
        }
    }
}