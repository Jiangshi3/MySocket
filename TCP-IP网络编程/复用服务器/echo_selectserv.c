#include <stdio.h>    
// 提供了标准输入输出函数，例如 printf 和 scanf，用于在控制台进行输入和输出操作。
#include <stdlib.h>  
// 包含了一些常用的函数，例如内存分配函数 malloc、free、exit 等。这个库主要用于动态内存分配和程序的终止。
#include <string.h>
// 包含了一系列字符串处理函数，例如 strcpy、strcat、strlen 等，用于对字符串进行操作。
#include <unistd.h>
// 包含了一些系统调用函数的声明，如 fork、exec、pipe 等。这个库主要用于提供对POSIX操作系统的基本功能的访问。
#include <arpa/inet.h>
// 包含了一些用于处理网络地址的函数，例如 inet_addr、inet_ntoa 等。这个库主要用于进行IPv4地址的转换。
#include <sys/socket.h>
// 提供了一系列系统调用函数，用于进行网络通信的套接字编程，如 socket、bind、listen、accept 等。
#include <sys/time.h>
// 包含了与时间相关的函数，例如 gettimeofday，用于获取当前时间。
#include <sys/select.h>
// 提供了 select 函数，用于在一组文件描述符上等待读取、写入或异常事件的发生。这对于多路复用的网络编程非常有用。
#include <assert.h>


const int BUF_SIZE = 100;
int main(int argc, const char* argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    struct timeval timeout;  //  I/O 操作时设置超时时间
    fd_set reads, cpy_reads;
    socklen_t adr_sz;
    char buf[BUF_SIZE];
    int fd_max, str_len, fd_num, i;
    if(argc!=2){
        printf("Usage : %s <port>\n", argv[0]); 
        exit(1);
    }

    serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    /* INADDR_ANY 是一个特殊的宏，表示将套接字绑定到任意可用的本地IP地址。
        这在服务器程序中常用，允许服务器接受来自任何网络接口的连接请求。*/
    serv_adr.sin_port = htons(atoi(argv[1]));

    int ret = bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr));
    assert(ret!=-1);
    ret = listen(serv_sock, 5);
    assert(ret!=-1);

    FD_ZERO(&reads);
    FD_SET(serv_sock, &reads);  //注册服务端套接字
    fd_max = serv_sock;

    while(1){
        cpy_reads = reads;
        timeout.tv_sec = 5;     // 5秒
        timeout.tv_usec = 5000; // 5000 微妙 = 5毫秒

        fd_num = select(fd_max+1, &cpy_reads, 0, 0, &timeout);  //开始监视,每次重新监听
        if(fd_num==-1)
            break;
        if(fd_num==0)
            continue;
        
        // 有变化
        for(int i=0; i<fd_max+1; ++i)  /*这里要记得fd_max+1  !!!!*/
        {
            if(FD_ISSET(i, &cpy_reads))  //查找发生变化的套接字文件描述符  
            {
                if(i==serv_sock)  // connection request --如果是服务端套接字时,受理连接请求
                {
                    adr_sz = sizeof(clnt_adr);
                    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
                    FD_SET(clnt_sock, &reads);  // 注册clnt_sock
                    if(fd_max<clnt_sock)
                        fd_max = clnt_sock;
                    printf("Connected client: %d \n", clnt_sock);
                }
                else //不是服务端套接字时; read message
                {
                    str_len = read(i, buf, BUF_SIZE);  //i指的是当前发起请求的客户端
                    if(str_len==0)
                    {
                        FD_CLR(i, &reads);
                        close(i);
                        printf("closed client: %d \n", i);
                    }
                    else
                        write(i, buf, BUF_SIZE);  // echo 回声

                }
            }
        }
    }
    close(serv_sock);

    return 0;
}








// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <arpa/inet.h>
// #include <sys/socket.h>
// #include <sys/time.h>
// #include <sys/select.h>

// #define BUF_SIZE 100
// void error_handling(char *message);

// int main(int argc, char *argv[])
// {
//     int serv_sock, clnt_sock;
//     struct sockaddr_in serv_adr, clnt_adr;
//     struct timeval timeout;
//     fd_set reads, cpy_reads;

//     socklen_t adr_sz;
//     int fd_max, str_len, fd_num, i;
//     char buf[BUF_SIZE];
//     if (argc != 2)
//     {
//         printf("Usage : %s <port>\n", argv[0]);
//         exit(1);
//     }
//     serv_sock = socket(PF_INET, SOCK_STREAM, 0);
//     memset(&serv_adr, 0, sizeof(serv_adr));
//     serv_adr.sin_family = AF_INET;
//     serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
//     serv_adr.sin_port = htons(atoi(argv[1]));

//     if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
//         error_handling("bind() error");
//     if (listen(serv_sock, 5) == -1)
//         error_handling("listen() error");

//     FD_ZERO(&reads);
//     FD_SET(serv_sock, &reads); //注册服务端套接字
//     fd_max = serv_sock;

//     while (1)
//     {
//         cpy_reads = reads;
//         timeout.tv_sec = 5;
//         timeout.tv_usec = 5000;

//         if ((fd_num = select(fd_max + 1, &cpy_reads, 0, 0, &timeout)) == -1) //开始监视,每次重新监听
//             break;
//         if (fd_num == 0)
//             continue;

//         for (i = 0; i < fd_max + 1; i++)
//         {
//             if (FD_ISSET(i, &cpy_reads)) //查找发生变化的套接字文件描述符
//             {
//                 if (i == serv_sock) //如果是服务端套接字时,受理连接请求
//                 {
//                     adr_sz = sizeof(clnt_adr);
//                     clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &adr_sz);

//                     FD_SET(clnt_sock, &reads); //注册一个clnt_sock
//                     if (fd_max < clnt_sock)
//                         fd_max = clnt_sock;
//                     printf("Connected client: %d \n", clnt_sock);
//                 }
//                 else //不是服务端套接字时
//                 {
//                     str_len = read(i, buf, BUF_SIZE); //i指的是当前发起请求的客户端
//                     if (str_len == 0)
//                     {
//                         FD_CLR(i, &reads);
//                         close(i);
//                         printf("closed client: %d \n", i);
//                     }
//                     else
//                     {
//                         write(i, buf, str_len);
//                     }
//                 }
//             }
//         }
//     }
//     close(serv_sock);
//     return 0;
// }

// void error_handling(char *message)
// {
//     fputs(message, stderr);
//     fputc('\n', stderr);
//     exit(1);
// }



