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
#include <openssl/md5.h>

int open_clientfd(char *hostname, char *port) 
{
    int client_socket;
    struct sockaddr_in serv_addr;
    client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (client_socket < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(9999);
    // The server IP address should be supplied as an argument when running the application.
    int addr_status = inet_pton(AF_INET, hostname, &serv_addr.sin_addr);
    if (addr_status <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    int connect_status = connect(client_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (connect_status < 0) {
        printf("\nConnection Failed \n");
        perror("");
        return -1;
    }
    return client_socket;
}


void parseline(char* cmdline, char** argv) {
    int i = 0;
    argv[i] = strtok(cmdline, " ");
    while (argv[i] != NULL) {
        argv[++i] = strtok(NULL, " ");
    }
    argv[++i] = NULL;
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
    close (inFile);
    int i;
    char temp[10];
    for (i=0; i<MD5_DIGEST_LENGTH; i++) {
	sprintf(temp, "%02x", c[i]);
	strcat(mystr, temp); 
    }
}
int I = 0;
void eval(char* cmdline, int clientfd, FILE* cmdfile) {
    char* argv[80];
    FILE* file;
    char buf[1024];
    char filepath[512] = "Local Directory/";
    char cmdcopy[80];

    int total_bytes;
    ssize_t current_bytes;
    int file_size;
    int chunk_size = 1000;
    char file_chunk[chunk_size];
    int current_chunk_size;

    cmdline[strcspn(cmdline, "\n")] = 0;
    strcpy(cmdcopy, cmdline);
    parseline(cmdline, argv);

    if (strcmp(argv[0], "pause") == 0 && argv[1] != NULL && argv[2] == NULL) {
        sleep(atoi(argv[1]));
    }
    else if (strcmp(argv[0], "append") == 0 && argv[1] != NULL && argv[2] == NULL) {
        write(clientfd, cmdcopy, strlen(cmdcopy));
	    bzero(buf, 1024);
        read(clientfd, buf, 1024);
        if (strcmp(buf, "0") == 0) {
            printf("File [%s] could not be found in remote directory.\n", argv[1]);
        }
        else if (strcmp(buf, "-1") == 0) {
            printf("File [%s] is currently locked by another user.\n", argv[1]);
        }
        while (strcmp(buf, "1") == 0) {
            printf("Appending> "); fflush(stdout);
	        bzero(cmdline, 80);
            fgets(cmdline, 80, cmdfile);
	        printf("%s", cmdline);
            cmdline[strcspn(cmdline, "\n")] = 0;
            strcpy(cmdcopy, cmdline);
            parseline(cmdline, argv);
            if (strcmp(argv[0], "pause") == 0 && argv[1] != NULL && argv[2] == NULL) {
                sleep(atoi(argv[1]));
            }
            else {
                write(clientfd, cmdcopy, strlen(cmdcopy));
		bzero(buf, 1024);
                read(clientfd, buf, 1024);
            }
        }
    }
    else if (strcmp(argv[0], "upload") == 0 && argv[1] != NULL && argv[2] == NULL) {
        strcat(filepath, argv[1]);
	if (access(filepath, F_OK) != 0) {
            printf("File [%s] could not be found in local directory.\n", argv[1]);
        }
        else {
            write(clientfd, cmdcopy, strlen(cmdcopy));
	    bzero(buf, 1024);
            read(clientfd, buf, 1024);
            if (strcmp(buf, "-1") == 0) {
                printf("File [%s] is currently locked by another user.\n", argv[1]);
            }
            else if (strcmp(buf, "1") == 0) {
                file = fopen(filepath, "rb");
		fseek(file, 0L, SEEK_END);
		file_size = ftell(file);
		fseek(file, 0L, SEEK_SET);
		total_bytes = 0;
		bzero(file_chunk, chunk_size);
		sprintf(file_chunk, "%d", file_size);
		send(clientfd, &file_chunk, sizeof(file_chunk), 0);
		while (total_bytes < file_size) {
		    memset(file_chunk, '\0', chunk_size);
		    current_chunk_size = fread(&file_chunk, sizeof(char), chunk_size, file);
		    current_bytes = send(clientfd, &file_chunk, current_chunk_size, 0);
		    total_bytes = total_bytes + current_bytes;
		}
		close(file);
		read(clientfd, buf, 1024);
		bzero(buf, 1024);
		printf("%i bytes uploaded successfully.\n", total_bytes);
            }
        }
    }
    else if (strcmp(argv[0], "download") == 0 && argv[1] != NULL && argv[2] == NULL) {
	strcat(filepath, argv[1]);
        write(clientfd, cmdcopy, strlen(cmdcopy));
	bzero(buf, 1024);
        read(clientfd, buf, 1024);
        if (strcmp(buf, "1") == 0) {
            file = fopen(filepath, "wb");
            bzero(file_chunk, chunk_size);
	    recv(clientfd, file_chunk, chunk_size, 0);
	    file_size = atoi(file_chunk);
	    total_bytes = 0;
	    while(total_bytes < file_size) {
		memset(file_chunk, '\0', chunk_size);
		current_bytes = recv(clientfd, file_chunk, chunk_size, 0);
		total_bytes = total_bytes + current_bytes;
		fwrite(&file_chunk, sizeof(char), current_bytes, file);
		fflush(file);
	    }
	    close(file);
	    printf("%i bytes downloaded successfully.\n", total_bytes);
	    write(clientfd, "", sizeof(""));
        }
        else if (strcmp(buf, "0") == 0) {
            printf("File [%s] could not be found in remote directory.\n", argv[1]);
        }
        else if (strcmp(buf, "-1") == 0) {
            printf("File [%s] is currently locked by another user.\n", argv[1]);
        }
    }
    else if (strcmp(argv[0], "delete") == 0 && argv[1] != NULL && argv[2] == NULL) {
        write(clientfd, cmdcopy, strlen(cmdcopy));
	bzero(buf, 1024);
        read(clientfd, buf, 1024);
        if (strcmp(buf, "1") == 0) {
            printf("File deleted successfully.\n");
        }
        else if (strcmp(buf, "0") == 0) {
            printf("File [%s] could not be found in remote directory.\n", argv[1]);
        }
        else if (strcmp(buf, "-1") == 0) {
            printf("File [%s] is currently locked by another user.\n", argv[1]);
        }
    }
    else if (strcmp(argv[0], "syncheck") == 0 && argv[1] != NULL && argv[2] == NULL) {
        printf("Sync Check Report:\n");
        strcat(filepath, argv[1]);
	if (access(filepath, F_OK) == 0) {
            struct stat st;
            stat(filepath, &st);
            printf("- Local Directory:\n");
            printf("-- File Size: %d bytes.\n", st.st_size);
        }
        write(clientfd, cmdcopy, strlen(cmdcopy));
	bzero(buf, 1024);
        read(clientfd, buf, 1024);
        if (strcmp(buf, "1") == 0) {
            printf("- Remote Directory:\n");
	    bzero(buf, 1024);
            read(clientfd, buf, 1024);
            printf("-- File Size: %s bytes.\n", buf);
            if (access(filepath, F_OK) == 0) {
                write(clientfd, "1", strlen("1"));
		char temp_hash[100] = "";
		MDFile(filepath, temp_hash);
		write(clientfd, temp_hash, sizeof(temp_hash));
		bzero(buf, 1024);
                read(clientfd, buf, 1024);
                printf("-- Sync Status: %s.\n", buf);
            }
            else {
	        write(clientfd, "0", strlen("0"));
                printf("-- Sync Status: unsynced.\n");
            }
	    bzero(buf, 1024);
            read(clientfd, buf, 1024);
            printf("-- Lock Status: %s.\n", buf);
        }
    }
    else {
        printf("Command [%s] is not recognized.\n", argv[0]);
    }
}

int main(int argc, char **argv) {
    int clientfd;
    char *host, *port;
    host = argv[2];
    port = "9999";
    clientfd = open_clientfd(host, port);
    FILE* file = fopen(argv[1], "r");
    char cmdline[80];
    char servermsg[1024];
    
    bzero(servermsg, 1024);
    recv(clientfd, servermsg, 1024, 0);
    printf("%s", servermsg); fflush(stdout);
    while (1) {
	printf("> ");
	bzero(cmdline, 80);
        fgets(cmdline, 80, file);
	    printf("%s", cmdline);
        if (strcmp(cmdline, "quit\n") == 0 || strcmp(cmdline, "quit") == 0) {
            write(clientfd, cmdline, sizeof(cmdline));
	        bzero(servermsg, 1024);
            read(clientfd, servermsg, 1024);
            close(clientfd);
            close(file);
            break;
        }
        else if (strcmp(cmdline, "\n") != 0) {
            eval(cmdline, clientfd, file);
        }
        else {
            printf("Command [] is not recognized.\n");
        }
    } 
}
