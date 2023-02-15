#ifndef INCLUDES_H
#define INCLUDES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define BuffSize 4096

#define READ 0
#define WRITE 1

#define ASSERT(condition, errorMessage)                 \
            if(!(condition)) {                          \
                fprintf(stderr, "%s", errorMessage);    \
                abort();                                \
            }


int read_line(int fd, char* buff);

int get_hash(const char* file_name, char* buff);


int send_file(int sd, const char* file_name, const char* receive_name);

int receive_file(int sd, char* file_name);

#endif
