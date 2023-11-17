#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

static bool stop = false;

static void handle_term(int sig){
    stop = true;
}

/*  argc和argv[]
    argc 表示命令行参数的数量，即程序运行时通过命令行输入的参数的个数。它包括程序名称在内。例如，如果你运行程序时输入了./my_program arg1 arg2 arg3，那么argc 的值将为 4。
    argv 是一个指向字符指针数组的指针，用于存储命令行参数的字符串。每个字符串指针指向一个命令行参数。
        argv[0] 存储程序的名称，argv[1] 存储第一个参数，依此类推。在上述的例子中，argv[0] 将是程序的名称，argv[1] 将是 "arg1"，argv[2] 将是 "arg2";
    这两个参数是由操作系统传递给程序的。当你在终端中执行一个可执行文件时，你可以通过命令行为程序传递参数。
    所以，通过检查 argc 和访问 argv 数组，你可以获取程序运行时传递的命令行参数。
*/

//  ./testlisten 192.168.1.108 12345 5
int main(int argc, char* argv[]){
    signal( SIGTERM, handle_term );
    /*
    signal() 函数用于在程序中捕获和处理信号。
        signal(SIGTERM, handle_term); 的目的是将信号 SIGTERM 的处理函数设置为 handle_term。
    信号 SIGTERM 是一个用于请求进程正常终止的信号。通常，当用户或系统管理员希望结束一个正在运行的进程时，会发送 SIGTERM 信号。
        这种信号允许进程在收到信号后进行清理工作，然后正常退出。如果进程没有捕获或忽略 SIGTERM 信号，它将导致进程被终止。
    */

    if( argc <= 3 )
    {
        printf( "usage: %s ip_address port_number backlog\n", basename( argv[0] ) );
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi(argv[2]);    // atoi()  将字符串"12345"转换为相应的整数12345
    int backlog = atoi(argv[3]);

    int sock = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));  // memset()是更通用的清零内存的函数，也更为标准和可移植。 
    //bzero(&address, sizeof(address));
    address.sin_family = AF_INET;  //地址族
    inet_pton(AF_INET, ip, &address.sin_addr);  
    // inet_pton(); 将用字符串表示的IP地址(const char* ip)转换成用网络字节序整数表示的IP地址，并把转换结果存储在&address.sin_addr地址中；AF_INET指定地址族；
    address.sin_port = htons(port);  // htons() 将短整型(short 16bit)主机字节序数据(host)转换成网络字节序数据(Network)

    int ret = bind(sock, (struct sockaddr*)&address, sizeof(address));  // 将address所指的socket地址分配给为命名的sock文件描述符；
    assert(ret != -1);

    ret = listen(sock, backlog);  //listen() 创建一个监听队列以存放待处理的客户连接；  sock指定被监听的socket；backlog 内核监听队列的最大长度；
    assert(ret != -1);

    while (!stop)
    {
        sleep(1);
    }

    close(sock);
    std::cout << "close socket" << std::endl;
    
    return 0;
}




