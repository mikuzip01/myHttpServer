#include <iostream>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <pthread.h>
#include <list>
#include <vector>
#include "ThreadPool.h"

// using namespace std;
using std::string; using std::cout; using std::endl;



void httpserver( int clientSocketFd ){
    char buffer[64];
    int ret = 0;
    while( true ){
        printf("wait message\n");
        ret = recv( clientSocketFd, buffer, 64, 0);  // fixme 从工作队列中取得socketfd
        if( ret == -1 ){
            printf("recv error!\n");
        }
        else{
            printf("get message{%s}\n", buffer);
            ret = send( clientSocketFd, buffer, ret, 0);
            if( ret == -1) printf("send error!\n");
            else{
                if( string("exit") == string(buffer) ){
                    printf("client disconnect\n");
                    break;
                }
            }
        }
        
    }
    return;
}


int setFdNonblock( int fd ){
    int old_options = fcntl( fd, F_GETFL );
    int new_options = old_options | O_NONBLOCK;
    fcntl( fd, F_SETFL, new_options );
    return old_options;
}

int main(){ 

    string ip = "127.0.0.1";
    int port = 12345;


    int socketfd = socket( AF_INET, SOCK_STREAM, 0 );

    struct sockaddr_in ipaddr;
    ipaddr.sin_family = AF_INET;
    ipaddr.sin_port = htons( port );
    ipaddr.sin_addr.s_addr = inet_addr( ip.c_str() );

    int ret = bind( socketfd, reinterpret_cast< struct sockaddr*>( &ipaddr ), sizeof( ipaddr ) );
    if( ret == -1 ){
        cout << "bind ipaddr failed" << endl;
        return -1;
    }

    ret = listen( socketfd, 20 );
    if( ret == -1 ){
        cout << "call listen failed" << endl;
        return -1;
    }

    char buffer[64];
    struct sockaddr_in clientaddr;
    socklen_t clientaddrLength = sizeof( clientaddr );



    // int clientsocketfd = accept( socketfd, reinterpret_cast< struct sockaddr*> ( &clientaddr ), &clientaddrLength);

    int epollFd = epoll_create(5);
    struct epoll_event socketEvent;
    memset( &socketEvent, 0, sizeof( socketEvent ) );
    setFdNonblock( socketfd );
    socketEvent.data.fd = socketfd;
    socketEvent.events = ( EPOLLIN | EPOLLET );
    epoll_ctl( epollFd, EPOLL_CTL_ADD, socketfd, &socketEvent); // socketfd在epoll_event需要设置在epoll_ctl()函数中也需要设置？

    struct epoll_event epollEvents[5];
    ThreadPool threadPool(httpserver);
    
    while(true){
        ret = epoll_wait( epollFd, epollEvents, 5, -1 );
        for( int i = 0; i < ret; ++i ){
            if ( epollEvents[i].data.fd == socketfd ) {
                int clientSocketFd = accept( socketfd, reinterpret_cast< struct sockaddr* >(&clientaddr), &clientaddrLength );
                threadPool.appendFd( clientSocketFd );
                threadPool.notifyOneThread();
            }
        }
    }

    return 0;
}