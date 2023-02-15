#include "includes.h"
#include <random>

int read_line(int fd, char* buff)
{
    int ret;

    for(int i = 0; (ret = read(fd, &buff[i], 1)); ++i)
    {
        if(ret == -1){
            fprintf(stderr, "Eroare la citire din fisier!!");
            return -1;
        }

        if(buff[i] == '\n'){
            buff[i] = '\0';
            return i;
        }
    }

    return 0;
}

int get_hash(const char* file_name, char* buff)
{
    std::random_device random;
    std::mt19937 mt(random());
    std::uniform_int_distribution<> number{ 0, 25 };

    char file_verify_cache[50];

    for(int i = 0; i < 49; ++i)
        file_verify_cache[i] = 'a' + number(mt);
    
    file_verify_cache[49] = '\0';

    while(access(file_name, F_OK) == -1)
        sleep(0);

    char command[1000] = "";
    sprintf(command, "sha256sum %s > %s", file_name, file_verify_cache);
    system(command);

    int fd1;
    if((fd1 = open(file_verify_cache, O_RDONLY)) == -1){
        fprintf(stderr, "Cannot open verify_cache!!");
        return -1;
    }

    memset(buff, 0, 1000);
    int bytes = read_line(fd1, buff);
    buff[bytes] = '\0';
    close(fd1);

    sprintf(command, "rm %s", file_verify_cache);
    system(command);
    return 0;
}

int send_file(int sd, const char* file_name, const char* receive_name)
{
    unsigned char buff[BuffSize+100];
    int fd;
    int bytes = 0;

    if((fd = open(file_name, O_RDONLY)) == -1){
        fprintf(stderr, "Cannot open %s!!", file_name);
        return -1;
    }

    bytes = strlen(receive_name) + 1;
    if(write(sd, &bytes, 4) == -1){
        fprintf(stderr, "Error at writing in socket!!");
        return -1;
    }

    if(write(sd, receive_name, bytes) == -1){
        fprintf(stderr, "Error at writing in socket!!");
        return -1;
    }

    memset(buff, 0, BuffSize);
    while(1) {
        if ((bytes = read(fd, buff, BuffSize)) == -1)
            return -1;

        if(write(sd, &bytes, sizeof(bytes)) == -1)
            return -1;

        if(bytes == 0)
            break;

        if(write(sd, buff, bytes) == -1)
            return -1;
    }

    close(fd);

    return 0;
}

int receive_file(int sd, char* file_name)
{
    int fd;
    int bytes = 0;

    if((read(sd, &bytes, 4)) == -1){
        fprintf(stderr, "Error at reading from socket!!");
        return -1;
    }

    memset(file_name, 0, 255);
    if((read(sd, file_name, bytes)) == -1){
        fprintf(stderr, "Error at reading from socket!!");
        return -1;
    }


    file_name[bytes] = '\0';

    while(access(file_name, F_OK) != -1){
        char tmp[255] = "new_";
        strcat(tmp, file_name);
        strcpy(file_name, tmp);
    }

    printf("%s\n", file_name);

    if(creat(file_name, 0666) == -1){
        fprintf(stderr, "Error at the creation of %s!!", file_name);
        return -1;
    }

    if((fd = open(file_name, O_WRONLY)) == -1){
        fprintf(stderr, "Cannot open %s!!\n", file_name);
        char mm[1000];
        getcwd(mm, sizeof(mm));
        fprintf(stderr, "current dir: %s\n", mm);
        return -1;
    }

    unsigned char buff[BuffSize + 100];
    memset(buff, 0, BuffSize);

    while(1) {
        if(read(sd, &bytes, sizeof(bytes)) == -1)
            return -1;

        if(bytes == 0)
            return 0;

        int left_to_read = bytes;
        int read_bytes = 0;
        while(1) {
            if((read_bytes = read(sd, buff + read_bytes, left_to_read)) == -1)
                return -1;

            if((left_to_read -= read_bytes) == 0)
                break;
        }

        if(write(fd, buff, bytes) == -1)
            return -1;
    }

    close(fd);

    return 0;
}