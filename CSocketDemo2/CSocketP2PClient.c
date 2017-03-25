//
//  CSocketP2PClient.c
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
    printf("-----------1.创建socket成功-----------\n");
    int socket_client = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_client == -1) {
        perror("-----------创建socket失败-----------\n");
        exit(1);
    }
    
    printf("-----------创建socket成功-----------\n");
    printf("-----------2.开始connect-----------\n");
    
    struct sockaddr_in socket_addr;
    socket_addr.sin_family = AF_INET;// 设置地址簇
    socket_addr.sin_port = htons(8899);// 设置端口
    socket_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    int conn = connect(socket_client, (struct sockaddr *)&socket_addr, sizeof(socket_addr));
    if (conn == -1) {
        perror("-----------创建connect失败-----------\n");
        close(socket_client);
        exit(1);
    }
    
    printf("-----------创建connect成功-----------\n");
    
    pid_t pid = fork();
    if (pid == -1) {
        perror("-----------fork失败-----------\n");
        exit(1);

    } else if (pid == 0) {// 创建子进程成功，接收数据
        char receiveBuf[1024] = {0};
        while (1) {
            memset(receiveBuf, 0, sizeof(receiveBuf));
            // 获取服务端回写的数据
            ssize_t read_result = read(socket_client, (char *)receiveBuf, sizeof(receiveBuf));
            if (read_result == 0) {
                printf("-----------服务端关闭-----------\n");
                break;// 退出循环
                
            } else if (read_result == -1) {// 读取失败
                exit(1);// 退出子进程
                
            } else {
                // 打印接收到的数据
                fputs(receiveBuf, stdout);
            }
        }
        
    } else {// 父进程，用来发送数据
        char sendBuf[1024] = {0};
        while (fgets(sendBuf, sizeof(sendBuf), stdin) != NULL) {
            // 往服务端写数据
            write(socket_client, (char *)sendBuf, sizeof(sendBuf));
            
            memset(sendBuf, 0, sizeof(sendBuf));
        }
    }
    
    return 0;
}
