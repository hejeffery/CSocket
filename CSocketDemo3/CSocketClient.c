//
//  CSocketClient.c
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
#include <errno.h>

// 用来处理粘包问题
ssize_t readn(int fildes, void *buf, size_t nbyte) {
    
    size_t left = nbyte;
    ssize_t nreaded = 0;
    char *pbuf = (char *)buf;
    
    while (left > 0) {
        
        nreaded = read(fildes, buf, left);
        if (nreaded < 0) {
            if (errno == EINTR) {// 信号中断了，上面的也会返回负数，这里要特别的处理一下
                continue;
            }
            return -1;
            
        } else if (nreaded == 0) {// 对方关闭
            break;
            
        } else {
            
            pbuf += nreaded;// 指针偏移
            left -= nreaded;
        }
    }
    
    return nbyte - left;
}

// 用来处理粘包问题
ssize_t writen(int fildes, const void *buf, size_t nbyte) {
    
    size_t left = nbyte;
    ssize_t nwrited = 0;
    char *pbuf = (char *)buf;
    
    while (left > 0) {
        
        nwrited = write(fildes, buf, left);
        if (nwrited < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
            
        } else if (nwrited == 0) {// 对方关闭
            continue;
            
        } else {
            
            pbuf += nwrited;// 指针偏移
            left -= nwrited;
        }
    }
    return nbyte - left;
}

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
    socket_addr.sin_port = htons(7877);// 设置端口
    socket_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    int conn = connect(socket_client, (struct sockaddr *)&socket_addr, sizeof(socket_addr));
    if (conn == -1) {
        perror("-----------创建connect失败-----------\n");
        close(socket_client);
        exit(1);
    }
    
    printf("-----------创建connect成功-----------\n");
    
    char sendBuf[1024] = {0};
    char receiveBuf[1024] = {0};
    while (fgets(sendBuf, sizeof(sendBuf), stdin) != NULL) {
        // 往服务端写数据
        writen(socket_client, (char *)sendBuf, sizeof(sendBuf));
        
        // 获取服务端回写的数据
        readn(socket_client, (char *)receiveBuf, sizeof(receiveBuf));
        // 打印回写数据
        fputs(receiveBuf, stdout);
        
        memset(sendBuf, 0, sizeof(sendBuf));
        memset(receiveBuf, 0, sizeof(receiveBuf));
    }
    
    return 0;
}
