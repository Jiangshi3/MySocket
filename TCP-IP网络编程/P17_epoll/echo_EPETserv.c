/*
    边缘触发； 接收数据仅注册一次该事件 --> so 一旦发生输入相关事件，就应该读取输入缓冲中的全部数据；
              因此需要验证输入缓冲是否为空,read函数返回-1，变脸errno的值为EAGAIN时，说明没有数据可读；
              边缘触发方式要采用非阻塞的read和write函数。
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>


const int BUF_SIZE=4; //缓冲区设置为 4 字节
const int EPOLL_SIZE=50;
void setnonblockingmode(int fd);
void error_handling(char *message);

int main(int argc, const char* argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t adr_sz;
    int str_len, i;
    char buf[BUF_SIZE];

    struct epoll_event* ep_events;
    struct epoll_event event;
    int epfd, event_cnt;

    if (argc != 2)
    {
        printf("Usage : %s <port> \n", argv[0]);
        exit(1);
    }
    serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    int ret=bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr));  // 给创建好的套接字分配地址信息
    if(ret==-1)
        error_handling("bind() error");
    ret = listen(serv_sock, 5);  // 把套接字转换为可接受连接的状态
    if(ret==-1)
        error_handling("listen() error");
    
    epfd = epoll_create(EPOLL_SIZE);  // 创建epoll例程
    ep_events = malloc(sizeof(struct epoll_event)*EPOLL_SIZE);
    
    setnonblockingmode(serv_sock);  // 设置非阻塞
    // event.events = EPOLLIN | EPOLLET;         // 需要读取数据的情况
    event.events = EPOLLIN; 
    event.data.fd = serv_sock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event);  //例程epfd 中添加文件描述符 serv_sock，目的是监听 enevt 中的事件

    while(1)
    {
        event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);  //获取改变了的文件描述符，返回数量
        if(event_cnt==-1)
        {
            puts("epoll_wait() error");
            break;
        }
        puts("return epoll_wait");  // 验证边缘触发
        for(i=0;i<event_cnt;++i)    
        {
            if(ep_events[i].data.fd==serv_sock)  // 客户端请求连接时
            {
                adr_sz = sizeof(clnt_adr);
                clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
                setnonblockingmode(clnt_sock);     // 非阻塞模式
                event.events = EPOLLIN | EPOLLET;  // 改成边缘触发
                event.data.fd = clnt_sock;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event);
                printf("connected client : %d \n", clnt_sock);
            }
            else  // 是客户端套接字
            {
                while(1)
                {
                    str_len = read(ep_events[i].data.fd, buf, BUF_SIZE);
                    if(str_len==0)
                    {
                        epoll_ctl(epfd, EPOLL_CTL_DEL, ep_events[i].data.fd, NULL); //从epoll中删除套接字
                        close(ep_events[i].data.fd);
                        printf("closed client : %d \n", ep_events[i].data.fd);
                        break;  // 跳出最近的这个while()循环
                    }
                    else if(str_len<0)  
                    {
                        if(errno==EAGAIN)  //read 返回-1 且 errno 值为 EAGAIN ，意味读取了输入缓冲的全部数据
                            break;
                    }
                    else
                    {
                        write(ep_events[i].data.fd, buf, str_len);
                    }
                }
            }
        }
    }
    close(serv_sock);
    close(epfd);

    return 0;
}

void setnonblockingmode(int fd)
{
    int flag = fcntl(fd, F_GETFL, 0);     // 获取之前设置的属性信息
    fcntl(fd, F_SETFL, flag|O_NONBLOCK);  // 在此基础上添加非阻塞标志
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

