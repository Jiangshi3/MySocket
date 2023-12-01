/*
    有客户端连接请求时，回声服务端创建子进程提供服务
*/
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <assert.h>

// #define BUF_SIZE 30
const static int BUF_SIZE=30;
// void error_handling(char* message);
void read_childproc(int sig);

int main(int argc, const char* argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    struct sigaction act;
    socklen_t adr_sz;
    pid_t pid;
    int str_len, state;
    char buf[BUF_SIZE];
    // 输入的命令是： ./name port
    if(argc!=2){
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }
    int port = atoi(argv[1]);

    // 信号
    act.sa_handler = read_childproc;  // 结构体的sa_handler成员保存信号处理函数的指针值(地址值)
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    state = sigaction(SIGCHLD, &act, 0); // 使用sigaction进行信号处理；  在sigaction注册; 子进程终止产生SIGCHLD信号；

    serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(port);

    int ret = bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)); // 将serv_adr所指的socket地址分配给为命名的serv_sock文件描述符；
    assert(ret!=-1); // 表示如果ret的值不等于-1，则程序继续执行；否则，将引发AssertionError异常
    ret = listen(serv_sock, 5);
    assert(ret!=-1);

    while(1){
        adr_sz = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
        if(clnt_sock==-1)
            continue;
        else
            puts("new client connect...");
        pid = fork();  //失败返回-1
        if(pid==-1){
            close(clnt_sock);
            continue;
        }
        if(pid==0)  /*子进程运行区域*/
        { 
            close(serv_sock);  // 关闭无关的套接字文件描述符
            while((str_len = read(clnt_sock, buf, BUF_SIZE)) != 0)
                write(clnt_sock, buf, str_len);
            close(clnt_sock);
            puts("client disconnect...");
            return 0;
        }
        else
            close(clnt_sock);  // 父进程关闭客户端套接字文件描述符
    }
    close(serv_sock);   

    return 0;
}

void read_childproc(int sig)
{
    pid_t pid;
    int status;
    pid = waitpid(-1, &status, WNOHANG); // 成功时返回终止的子进程id； 子进程终止后产生的返回值保存在status
    printf("removed proc id: %d\n", pid);
}
                                                                                                                             
