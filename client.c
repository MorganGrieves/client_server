#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

#define SIZE 1024

struct sockaddr_in local;

int client()
{
    int connection;
    FILE *fd;

    char *filename = "./file.pdf";
    //открываем сокет под TCP/IP, получаем дескриптор
    int s = socket (AF_INET, SOCK_STREAM, 0);
    if (s < 0)
    {
        printf("Error: socket");
        return -1;
    }
    
    //создаем адрес 
    inet_aton("127.0.0.1", &local.sin_addr);
    local.sin_port = htons(8080);
    local.sin_family = AF_INET;
    //связываем сокет с адресом
    connection = connect(s, (struct sockaddr*) &local, sizeof(local));
    if (connection < 0)
    {
        printf("Error: connect");
        close(s);
        return -1;
    }
    //открываем файл, отправляем кусками
    fd = fopen(filename, "r");
    if (fd == NULL)
    {
        printf("Error: file open failed");
        close(s);
        return -1;   
    }
    char data[SIZE] = {0};
    int bytes; 
    while (!feof(fd))
    {
        if ((bytes = fread(&data, 1, SIZE, fd)) > 0)
        {
            if (send(s, data, bytes, 0) < 0)
            {
                printf("Error: sending - (%s)", strerror(errno));
                return -1;   
            }
        }
        bzero(data, SIZE);
    }
    fclose(fd); 
    close(s);
}

int main(int argc, char *argv[])
{
    if (client() < 0)
        return -1;
   
    return 0; 
}
