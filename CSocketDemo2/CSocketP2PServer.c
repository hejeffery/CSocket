//
//  CSocketP2PServer.c
//
//  Created by HeJeffery on 2017/3/25.
//  Copyright © 2017年 HeJeffery. All rights reserved.
//

#include <stdio.h>

#include <stdlib.h>
#include <memory.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

int main(int argc, const char * argv[]) {
    
    printf("-----------1.创建socket-----------\n");
    // int socket(int domain, int type, int protocol)
    // 第一个参数：通信协议簇，推荐使用PF开头的，AF也可以
    // 第二个参数：socket的类型，SOCK_STREAM流类型，SOCK_DGRAM数据报类型
    // 第三个参数：通常为0。IPPROTO_TCP也行
    // 返回值：失败返回-1
    int socket_server = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_server == -1) {
        perror("-----------创建socket失败-----------\n");
    }
    
    printf("-----------创建socket成功-----------\n");
    printf("-----------2.开始绑定-----------\n");
    
    // 使用REUSEADDR选项可以使得不必等待TIME_WAIT状态消失就可以重启服务器
    // int setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len)
    // 第一个参数：socket函数返回的id
    // 第二个参数：level，SO_REUSEADDR
    // 第三个参数：标识，SO_REUSEADDR
    // 第四个参数：选项
    // 第五个参数：选项的大小
    // 返回值：失败返回-1
    int open_reuse_addr = 1;
    if (setsockopt(socket_server, SOL_SOCKET, SO_REUSEADDR, (int *)&open_reuse_addr, sizeof(open_reuse_addr)) < 0) {
        printf("-----------设置REUSEADDR失败-----------\n");
    }

    struct sockaddr_in socket_addr;
    socket_addr.sin_family = AF_INET;// 设置地址簇
    socket_addr.sin_port = htons(8899);// 设置端口
    socket_addr.sin_addr.s_addr = htons(INADDR_ANY);// 设置地址
//    socket_addr.sin_addr.s_addr = inet_addr("127.0.0.1");// 这种方式也OK
    
    //  int bind(int socket, const struct sockaddr *address, socklen_t address_len)
    // 第一个参数：socket函数返回的id
    // 第二个参数：struct sockaddr 要绑定的地址
    // 第三个参数：地址的长度
    // 返回值：成功返回0
    int bind_result = bind(socket_server, (struct sockaddr *)&socket_addr, sizeof(socket_addr));
    if (bind_result == -1) {
        perror("-----------绑定失败-----------\n");
        exit(1);
    }
    
    printf("-----------绑定成功-----------\n");
    printf("-----------3.开始监听-----------\n");
    
    // int listen(int socket, int backlog)
    // 第一个参数：socket函数返回的id
    // 第二个参数：规定内核为此socket排队的最大连接个数
    // 返回值：成功返回0
    int listen_reslut = listen(socket_server, SOMAXCONN);
    if (listen_reslut == -1) {
        perror("-----------监听失败-----------\n");
        exit(1);
    }
    
    printf("-----------监听成功-----------\n");
    printf("-----------4.开始accept-----------\n");

    // int accept(int socket, struct sockaddr *restrict address, socklen_t *restrict address_len);
    // 第一个参数：socket函数返回的id
    // 第二个参数：将返回对等方的套接字地址
    // 第三个参数：将返回对等方的套接字地址长度
    // 返回值：失败返回-1
    struct sockaddr_in socket_client_addr;
    socklen_t socket_client_length = sizeof(socket_client_addr);
    int connect = accept(socket_server, (struct sockaddr *)&socket_client_addr, &socket_client_length);
    if (connect == -1) {
        perror("-----------accept失败-----------\n");
        exit(1);
    }
    
    printf("-----------accept成功-----------\n");
    printf("客户端ip = %s, 客户端端口 = %d\n", inet_ntoa(socket_client_addr.sin_addr), ntohs(socket_client_addr.sin_port));
    
    // 创建子进程
    pid_t pid = fork();
    if (pid == -1) {
        perror("-----------fork失败-----------\n");
        exit(1);

    } else if (pid == 0) {// 创建子进程成功，用来发送数据
        char sendBuff[1024] = {0};
        while (fgets(sendBuff, sizeof(sendBuff), stdin) != NULL) {

            write(connect, sendBuff, strlen(sendBuff));
            memset(sendBuff, 0, sizeof(sendBuff));
        }
        
    } else {// 父进程，用来读取数据
        char receiverBuff[1024] = {0};
        while (1) {
            memset(receiverBuff, 0, sizeof(receiverBuff));
            ssize_t read_result = read(connect, receiverBuff, sizeof(receiverBuff));
            
            if (read_result == 0) {
                printf("-----------客户端关闭-----------\n");
                break;// 退出循环

            } else if (read_result == -1) {// 读取失败
                exit(1);// 退出子进程

            } else {
                // 打印接收到的数据
                fputs(receiverBuff, stdout);
            }
        }
    }

    return 0;
}
