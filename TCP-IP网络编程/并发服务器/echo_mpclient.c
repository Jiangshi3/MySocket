#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <assert.h>

/*
    回声客户端的I/O程序分割
*/

const static int BUF_SIZE=30;
void read_routine(int sock, char* buf);
void write_routine(int sock, char* buf);

int main(int argc, const char* argv[])
{
    int clnt_sock;
    struct sockaddr_in serv_adr;
    pid_t pid;
    char buf[BUF_SIZE];
    // 输入的命令是： ./name port
    if(argc!=3){
        printf("Usage : %s <IP><port>\n", argv[0]);
        exit(1);
    }
    const char* ip = argv[1];
    int port = atoi(argv[2]);

    clnt_sock = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    inet_pton(AF_INET, ip, &serv_adr.sin_addr);  
    // inet_pton(); 将用字符串表示的IP地址(const char* ip)转换成用网络字节序整数表示的IP地址，并把转换结果存储在&address.sin_addr地址中；AF_INET指定地址族；
    serv_adr.sin_port = htons(port);

    int ret = connect(clnt_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr));
    assert(ret!=-1);

    // 读写分离
    pid = fork();
    if(pid == 0)
        write_routine(clnt_sock, buf);
    else
        read_routine(clnt_sock, buf);
    
    close(clnt_sock);


    return 0;
}

void read_routine(int sock, char* buf){
    while(1){
        int str_len = read(sock, buf, BUF_SIZE);
        if(str_len==0)
            return ;
        buf[str_len]=0;
        printf("Message from server : %s\n", buf);
    }
}
void write_routine(int sock, char* buf){
    while(1){
        fgets(buf, BUF_SIZE, stdin);
        if(!strcmp(buf, "q\n") || !strcmp(buf, "Q\n"))
        {
            shutdown(sock, SHUT_WR);
            return ;
        }
        write(sock, buf, strlen(buf));
    }
}                                                                                                       

/*
fgets(buf, BUF_SIZE, stdin);
    这一行代码使用 fgets 函数从标准输入（stdin）中读取字符串，并将其存储到名为 buf 的字符数组中。
    第二个参数 BUF_SIZE 是要读取的最大字符数，这有助于防止缓冲区溢出.

if(!strcmp(buf, "q\n") || !strcmp(buf, "Q\n"))
    strcmp 函数用于比较两个字符串是否相等。如果用户输入的字符串与 "q\n" 或 "Q\n" 相等（即返回值为0），则条件为真。
shutdown(sock, SHUT_WR);
    这一行调用 shutdown 函数，关闭与 sock 关联的写入（WR）通道。
    这表示在关闭写入通道后，不能再通过这个套接字发送数据
*/

