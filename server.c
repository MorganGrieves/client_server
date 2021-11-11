#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <errno.h>

#define SIZE 1024

FILE *fd = NULL;
int cs = -1;
int ss = -1;
char *filename = "./new_file.pdf";

struct sockaddr_in local;

void signal_handler(int signalno)
{
    if (fd != NULL)
    {
        fclose(fd);
        unlink(filename);
    }
    if (cs > -1)
        close(cs);
    
    if (ss > -1)
        close(ss);  

    exit(EXIT_SUCCESS);  
}

int server()
{
    int connection;
    
    //устанавливаем сигналы
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = &signal_handler;
    sigset_t set;
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGHUP);
    sigaction(SIGTERM, &act, 0);
    sigaction(SIGHUP, &act, 0);

    //открываем серверный сокет
    ss = socket(AF_INET, SOCK_STREAM, 0);
    if (ss < 0)
    {
        return -1;
    }
    inet_aton("127.0.0.1", &local.sin_addr);
    local.sin_port = htons(8080);
    local.sin_family = AF_INET;
    
    //связываем адрес и серверный сокет
    connection = bind(ss, (struct sockaddr*) &local, sizeof(local));
    if (connection < 0)
    {
        return -1;
    }
    if (listen(ss, 5) < 0)
    {
        return -1;
    }

    //сокет, который будет связан с клиентом
    cs = accept(ss, NULL, NULL);
    
    int n;
    char buf[SIZE];

    fd = fopen(filename, "w");

    while(1)
    {
        n = recv(cs, buf, SIZE, 0);
        
        if (n <= 0)
        {
            break;
        }
        fwrite(&buf, 1, n, fd);
        bzero(buf, SIZE);
    }
    fclose(fd);
    close(cs);
    return 0;
}

int main()
{
    //создаем потомка
    int pid = fork();
    if (pid == -1)
    {
        printf("Error: start failed - (%s)\n", strerror(errno));
        return -1;
    }
    else if (!pid) // если потомок
    {
        //выставляем права доступа
        umask(0);
        //новый сеанс
        if (setsid() < 0)
        {
            printf("Error: setsid failed");
            return -1;
        }
        //переходим в директорию хранения
        chdir("/home/user/dir");
        //закрываем дескриптор ввода, вывода, ошибок
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        
        if (server() < 0)
            return -1;

        return 0;
    }
    else
    {
        //если родитель, заканчиваем
        return 0;
    }
}
