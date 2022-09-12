// Names: An Le, Abbas Nurie
// // NetIDs: anvl, anurie
// // Student IDs: 26801474, 64031491
// // An : Lecture A, Abbas : Lecture B

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <semaphore.h>
#include <openssl/md5.h>

static char * LOCKED[10][30];
static sem_t mutex;

int open_listenfd(char *ip) {
    int server_socket;
    struct sockaddr_in address;
    int opt = 1;
    // Creating socket file descriptor
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    int socket_status = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR , &opt, sizeof(opt));
    if (socket_status) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
// address.sin_addr.s_addr = INADDR_ANY;
// The server IP address should be supplied as an argument when running the application.
    address.sin_addr.s_addr = inet_addr(ip);
    address.sin_port = htons(9999);
    int bind_status = bind(server_socket, (struct sockaddr*)&address, sizeof(address));
    if (bind_status < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    int listen_status = listen(server_socket, 1);
    if (listen_status < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    return server_socket;
}

void parseline(char* cmdline, char** argv) {
    int i = 0;
    argv[i] = strtok(cmdline, " ");
    while (argv[i] != NULL) {
        argv[++i] = strtok(NULL, " ");
    }
    argv[++i] = NULL;
}

void addFile(char * filename) {
    sem_wait(&mutex);
    int i;
    for (i=0; i<10; i++) {
        if (strcmp(LOCKED[i], " ") == 0) {
            strcpy(LOCKED[i], filename);
            break;
        }
    }
    sem_post(&mutex);
}

void removeFile(char * filename) {
    sem_wait(&mutex);
    int i;
    for (i=0; i<10; i++) {
        if (strcmp(LOCKED[i], filename) == 0) {
            strcpy(LOCKED[i], " ");
            break;
        }
    }
    sem_post(&mutex);
}

int containFile(char * filename) {
    sem_wait(&mutex);
    int i;
    for (i=0; i<10; i++) {
        if (strcmp(LOCKED[i], filename) == 0) {
	    sem_post(&mutex);
            return 1;
        }
    }
    sem_post(&mutex);
    return 0;
}

void MDFile (char *filename, char* mystr)
{
    unsigned char c[MD5_DIGEST_LENGTH];
    FILE *inFile = fopen (filename, "rb");
    MD5_CTX mdContext;
    int bytes;
    unsigned char data[1024];
    MD5_Init (&mdContext);
    while ((bytes = fread (data, 1, 1024, inFile)) != 0)
        MD5_Update (&mdContext, data, bytes);
    MD5_Final (c, &mdContext);
    fclose (inFile);
    int i;
    char temp[10];
    for (i=0; i<MD5_DIGEST_LENGTH; i++) {
        sprintf(temp, "%02x", c[i]);
        strcat(mystr, temp);
    }
}

static void init_getandsendmsg(void) {
    sem_init(&mutex, 0, 1);
    int i;
    for (i=0; i<10; i++) {
        strcpy(LOCKED[i], " ");
    }
}

void getandsendmsg(int connfd) {
    char buf[1024];
    char* argv[80];
    char filename[80];
    FILE* file;
    static pthread_once_t once = PTHREAD_ONCE_INIT;

    int total_bytes;
    ssize_t current_bytes;
    int file_size;
    int chunk_size = 1000;
    char file_chunk[chunk_size];
    int current_chunk_size;
    
    pthread_once(&once, init_getandsendmsg);
    write(connfd, "Welcome to ICS53 Online Cloud Storage.\n", strlen("Welcome to ICS53 Online Cloud Storage.\n"));
    while(1) {
	char filepath[512] = "Remote Directory/";
        bzero(buf, 1024);
        read(connfd, buf, 1024);
        buf[strcspn(buf, "\n")] = 0;
        parseline(buf, argv);
	if (strcmp(argv[0], "quit") != 0) {
	    strcpy(filename, argv[1]);
	}
        if (strcmp(argv[0], "append") == 0) {
	    strcat(filepath, argv[1]);
            if (access(filepath, F_OK) != 0) {
                write(connfd, "0", strlen("0"));
            }
            else if (containFile(filename)) {
                write(connfd, "-1", strlen("-1"));
            }
            else {
                write(connfd, "1", strlen("1"));
                addFile(filename);
                file = fopen(filepath, "a");
                fprintf(file, "%s", "\n");
                while (1) {
                    bzero(buf, 1024);
                    read(connfd, buf, 1024);;
                    if (strcmp(buf, "close") == 0) {
                        removeFile(filename);
                        fclose(file);
                        write(connfd, "2", strlen("2"));
                        break;
                    }
                    else {
                        strcat(buf, "\n");
                        fprintf(file, "%s", buf);
			            fflush(file);
                        write(connfd, "1", strlen("1'"));
                    }
                }
            }
        }
        else if (strcmp(argv[0], "upload") == 0) {
	        strcat(filepath, filename);
            if (containFile(argv[1])) {
                write(connfd, "-1", strlen("-1"));
            }
            else {
                write(connfd, "1", strlen("1"));
                file = fopen(filepath, "wb");
		bzero(file_chunk, chunk_size);
		recv(connfd, file_chunk, chunk_size, 0);
		file_size = atoi(file_chunk);
		total_bytes = 0;
		while (total_bytes < file_size) {
		    memset(file_chunk, '\0', chunk_size);
		    current_bytes = recv(connfd, file_chunk, chunk_size, 0);
		    total_bytes = total_bytes + current_bytes;
		    fwrite(&file_chunk, sizeof(char), current_bytes, file);
		    fflush(file);
		}
		close(file);
		write(connfd, "", sizeof(""));
            }
        }
        else if (strcmp(argv[0], "download") == 0) {
	    strcat(filepath, argv[1]);
            if (access(filepath, F_OK) != 0) {
                write(connfd, "0", strlen("0"));
            }
            else if (containFile(filename)) {
                write(connfd, "-1", strlen("-1"));
            }
            else {
		write(connfd, "1", strlen("1"));

                file = fopen(filepath, "rb");
                fseek(file, 0L, SEEK_END);
		file_size = ftell(file);
		fseek(file, 0L, SEEK_SET);
		total_bytes = 0;
		bzero(file_chunk, chunk_size);
		sprintf(file_chunk, "%d", file_size);
		send(connfd, &file_chunk, sizeof(file_chunk), 0);
		while (total_bytes < file_size) {
		    memset(file_chunk, '\0', chunk_size);
		    current_chunk_size = fread(&file_chunk, sizeof(char), chunk_size, file);
		    current_bytes = send(connfd, &file_chunk, current_chunk_size, 0);
		    total_bytes = total_bytes + current_bytes;
		}
		close(file);
		read(connfd, buf, 1024);
		bzero(buf, 1024);
            }
        }
        else if (strcmp(argv[0], "delete") == 0) {
	    strcat(filepath, argv[1]);
            if (access(filepath, F_OK) != 0) {
                write(connfd, "0", strlen("0"));
            }
            else if (containFile(filename)) {
                write(connfd, "-1", strlen("-1"));
            }
            else {
                remove(filepath);
                write(connfd, "1", strlen("1"));
            }
        }
        else if (strcmp(argv[0], "syncheck") == 0) {
	    strcat(filepath, argv[1]);
            if (access(filepath, F_OK) != 0) {
                write(connfd, "0", strlen("0"));
            }
            else {
                write(connfd, "1", strlen("1"));
                struct stat st;
                stat(filepath, &st);
                char temp[80];
                sprintf(temp, "%d", st.st_size);
                write(connfd, temp, strlen(temp));
                bzero(buf, 1024);
                read(connfd, buf, 1024);
                if (strcmp(buf, "1") == 0) {
                    read(connfd, buf, 1024);
                    char temp_hash[100] = "";
	                MDFile(filepath, temp_hash);
                    if (strcmp(buf, temp_hash) == 0) {
                        write(connfd, "synced", strlen("synced"));
                    }
                    else {
                        write(connfd, "unsynced", strlen("unsynced"));
                    }
                }
                if (containFile(filename)) {
                    write(connfd, "locked", strlen("locked"));
                }
                else {
                    write(connfd, "unlocked", strlen("unlocked"));
                }
            }
        }
        else if (strcmp(argv[0], "quit") == 0) {
            write(connfd, "1", strlen("1"));
            break;
        }
	if (strcmp(argv[0], "quit") != 0 ){
	    bzero(argv[1], 100);
	}
    }
}

void *thread(void *vargp){
    int connfd = *((int *)vargp);
    pthread_detach(pthread_self()); 
    free(vargp);                    
    getandsendmsg(connfd);
    close(connfd);
    return NULL;
}

int main(int argc, char** argv) {
    int listenfd, *connfdp;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;
    listenfd = open_listenfd(argv[1]);
    while (1) {
        clientlen=sizeof(struct sockaddr_storage);
        connfdp = malloc(sizeof(int)); 
        *connfdp = accept(listenfd, &clientaddr, &clientlen); 
        pthread_create(&tid, NULL, thread, connfdp);
    }
}
