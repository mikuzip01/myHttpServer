#ifndef __UTILS__
#define __UTILS__

#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string>
#include <sys/epoll.h>
#include <sys/fcntl.h>
#include "Error.h"

extern char *memory_index_page;

void bufferPrinter(char* buffer, const int bufferSize);
void parseLineFromSocket( char* buffer, const int buffSisze);
void setAddrReuse( int socketfd );
void sendHello( int sockedfd );
void dispAddrInfo( struct sockaddr_in &addr);
void sprintAddrInfo(  struct sockaddr_in &addr, std::string& str );

// 输入文件路径，判断文件是否存在
bool fileExist(const char [] );

// 载入指定页面文件到内存
void loadIndexTomemory( std::string );
// 设置文件描述符为非阻塞
int setFdNonblock( int fd );
// 向epoll中添加需要监听的文件描述符;可读事件配合电平触发
void addFdToEpoll_INLT( int epollfd, int addedfd );
// 向epoll中添加需要监听的文件描述符;可读事件配合边缘触发
void addFdToEpoll_INET( int epollfd, int addedfd );
// 向epoll中添加需要监听的文件描述符;可读事件配合边缘单次触发，防止在epoll中，正在被其他线程处理的客户端链接被探测到，导致客户端链接被重复传入线程池
void addFdToEpoll_INETONESHOT( int epollfd, int addedfd );
// 重置Epoll的oneshot设置
void resetOneshot_INETONESHOT( int epollfd, int resetfd );
// 从epoll中删除被监听的文件描述符
void deleteFdFromEpoll( int epollfd, int targetfd );
// 打印客户端的相关网络信息到屏幕上
void dispPeerConnection( int clientFd );
void dispPeerConnection( int clientFd , std::string& str);

#endif
