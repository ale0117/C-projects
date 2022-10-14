// Names: An Le, Abbas Nurie
// NetIDs: anvl, anurie
// Student IDs: 26801474, 64031491
// An : Lecture A, Abbas : Lecture B

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct table_entry {
    int valid_bit;
    int dirty_bit;
    int page_number;
    int initialized;
    int used;
};

int HANDLER;
int GLOBAL_COUNT = 1;
int DISK[128];
int MAIN_MEM[32];
struct table_entry PAGE_TABLE[16];

void parseline(char* cmdline, char** argv) {
    int i = 0;
    argv[i] = strtok(cmdline, " ");
    while (argv[i] != NULL) {
        argv[++i] = strtok(NULL, " ");
    }
    argv[++i] = NULL;
}

void page_fault_handler(int page_num) {
    int i;
    int count = 0;
    int highest_page = -1;
    for (i=0; i<16; i++) {
        if (PAGE_TABLE[i].valid_bit == 1) {
            count++;
            if (highest_page < PAGE_TABLE[i].page_number) {
                highest_page = PAGE_TABLE[i].page_number;
            }
        }
    }
    if (count < 4) {
        int x = page_num * 8;
        int y = (highest_page+1) * 8;
        while (x < page_num*8 + 8) {
            MAIN_MEM[y] = DISK[x];
            x++;
            y++;
        }
        PAGE_TABLE[page_num].page_number = highest_page+1;
    }
    else if (count == 4) {
        int victim_page;
        if (HANDLER) {
            int temp = 9999;
            for (i=0; i<16; i++) {
                if (PAGE_TABLE[i].valid_bit == 1 && PAGE_TABLE[i].initialized < temp) {
                    temp = PAGE_TABLE[i].initialized;
                    victim_page = i;
                }
            }
        }
        else {
            int temp = 9999;
            for (i=0; i<16; i++) {
                if (PAGE_TABLE[i].valid_bit == 1) {
                    if (PAGE_TABLE[i].used < temp || ((PAGE_TABLE[i].used == temp && PAGE_TABLE[i].page_number < PAGE_TABLE[victim_page].page_number))) {
                        temp = PAGE_TABLE[i].used;
                        victim_page = i;
                    }
                }
            }
        }
        int x = victim_page * 8;
        int y = PAGE_TABLE[victim_page].page_number * 8;
        if (PAGE_TABLE[victim_page].dirty_bit == 1) {
            while (x < victim_page*8 + 8) {
                DISK[x++] = MAIN_MEM[y++];
            }
        }
        x = page_num * 8;
        y = PAGE_TABLE[victim_page].page_number * 8;
        while (x < page_num*8 + 8) {
            MAIN_MEM[y++] = DISK[x++];
        }
        PAGE_TABLE[page_num].page_number = PAGE_TABLE[victim_page].page_number;
        PAGE_TABLE[victim_page].valid_bit = 0;
        PAGE_TABLE[victim_page].dirty_bit = 0;
        PAGE_TABLE[victim_page].page_number = victim_page;
        PAGE_TABLE[victim_page].initialized = 0;
        PAGE_TABLE[victim_page].used = 0;
    }
    PAGE_TABLE[page_num].initialized = GLOBAL_COUNT++;
    PAGE_TABLE[page_num].valid_bit = 1;
}

void read(int addr) {
    int page_num = addr/8;
    int address = addr%8;
    if (page_num >= 0 && page_num < 16) {
        if (PAGE_TABLE[page_num].valid_bit == 1) {
            printf("%d\n", MAIN_MEM[PAGE_TABLE[page_num].page_number*8 + address]);
            PAGE_TABLE[page_num].used++;
        }
        else {
            printf("A Page Fault Has Occurred\n");
            page_fault_handler(page_num);
            read(addr);
        }
    }

}

void write(int addr, int val) {
    int page_num = addr/8;
    int address = addr%8;
    if (page_num >= 0 && page_num < 16) {
        if (PAGE_TABLE[page_num].valid_bit == 1) {
            MAIN_MEM[PAGE_TABLE[page_num].page_number*8 + address] = val;
            PAGE_TABLE[page_num].dirty_bit = 1;
            PAGE_TABLE[page_num].used++;
        }
        else {
            printf("A Page Fault Has Occurred\n");
            page_fault_handler(page_num);
            write(addr, val);
        }
    }

}

void printmain(int page_num) {
    int i;
    for (i=page_num*8; i<page_num*8+8; i++) {
        printf("%d: %d\n", i, MAIN_MEM[i]);
    }
}

void printtable() {
    int i;
    for (i=0; i<16; i++) {
        printf("%d:%d:%d:%d\n", i, PAGE_TABLE[i].valid_bit, PAGE_TABLE[i].dirty_bit, PAGE_TABLE[i].page_number);
    }
}

void eval(char* cmdline) {
    char* argv[80];
    cmdline[strcspn(cmdline, "\n")] = 0;
    parseline(cmdline, argv);
    if (strcmp(argv[0], "read") == 0 && argv[1] != NULL && argv[2] == NULL) {
        read(atoi(argv[1]));
    }
    else if (strcmp(argv[0], "write") == 0 && argv[1] != NULL && argv[2] != NULL && argv[3] == NULL) {
        write(atoi(argv[1]), atoi(argv[2]));
    }
    else if (strcmp(argv[0], "showmain") == 0 && argv[1] != NULL && argv[2] == NULL) {
        printmain(atoi(argv[1]));
    }
    else if (strcmp(argv[0], "showptable") == 0 && argv[1] == NULL) {
        printtable();
    }
}

int main(int argc, char* argv[]){ 
    if (argc == 1 || strcmp(argv[1], "FIFO") == 0) {
        HANDLER = 1;
    }
    else if (strcmp(argv[1], "LRU") == 0) {
        HANDLER = 0;
    }
    else {
        printf("Invalid command-line arguments\n");
        exit(0);
    }
    int i;
    for (i=0; i<128; i++) {
        DISK[i] = -1;
    }
    for (i=0; i<32; i++) {
        MAIN_MEM[i] = -1;
    }
    for (i=0; i<16; i++) {
        struct table_entry temp = {0,0,i,0,0};
        PAGE_TABLE[i] = temp;
    }
    char cmdline[80];
    while (1) {
        printf("> ");
        fgets(cmdline, 80, stdin);
        if (strcmp(cmdline, "quit\n") == 0) {
            exit(0);
        }
        else if (strcmp(cmdline, "\n") != 0) {
            eval(cmdline);
        }
    }
}