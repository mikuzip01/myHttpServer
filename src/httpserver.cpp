#include <iostream>
#include <stdio.h>
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
#include <exception>
#include <unistd.h>
#include "ThreadPool.h"
#include "utils.h"
#include "HttpData.h"
#include "Responser.h"


using std::string; using std::exception; using std::runtime_error;
using std::cout; using std::endl;

const int BUFFERSIZE = 1024;


void httpserver( int clientSocketFd ){
    HttpData httpData( clientSocketFd );
    httpData.parseData();

    Responser responser( httpData );

    printf("requset method: %s, url: %s, http verison: %s\n", httpData.getRequestMethod_string().c_str(), httpData.getUrl().c_str(), httpData.getVersion().c_str());
    printf("%s\n", httpData.getUserAgent().c_str() );

    if( !fileExist( httpData.getUrl().c_str() ) ){ responser.sendNotFound(); return; }

    if( httpData.getRequestMethod() == HttpData::RequestMethod::GET ){
        if( httpData.getUrlResourceType() == "html") responser.sendStaticFileToClient();
        else responser.sendNotFound();
    }
    else if( httpData.getRequestMethod() == HttpData::RequestMethod::POST ){
        if( httpData.getUrlResourceType() == "cgi" ) responser.executeCGI();
        else responser.sendNotFound();
    }
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
    if( socketfd == -1 ) throw runtime_error("socket create failed\n");
    printf("socket create success\n");
    
    setPortReuse( socketfd );

    struct sockaddr_in ipaddr;
    ipaddr.sin_family = AF_INET;
    ipaddr.sin_port = htons( port );
    ipaddr.sin_addr.s_addr = inet_addr( ip.c_str() );  


    int ret = bind( socketfd, reinterpret_cast< struct sockaddr*>( &ipaddr ), sizeof( ipaddr ) );
    if( ret == -1 ) throw runtime_error("bind ipaddr failed\n");
    printf("bind ipaddr create success\n");

    ret = listen( socketfd, 20 );
    if( ret == -1 ) throw runtime_error("call listen failed\n");
    printf("listen create success\n");

    struct sockaddr_in clientaddr;
    socklen_t clientaddrLength = sizeof( clientaddr );

    // int clientsocketfd = accept( socketfd, reinterpret_cast< struct sockaddr*> ( &clientaddr ), &clientaddrLength);

    int epollFd = epoll_create(5);
    struct epoll_event socketEvent;
    memset( &socketEvent, 0, sizeof( socketEvent ) );
    setFdNonblock( socketfd );
    socketEvent.data.fd = socketfd;
    socketEvent.events = ( EPOLLIN | EPOLLET ); // 边缘触发模式
    epoll_ctl( epollFd, EPOLL_CTL_ADD, socketfd, &socketEvent); // socketfd在epoll_event需要设置在epoll_ctl()函数中也需要设置？

    struct epoll_event epollEvents[5];
    ThreadPool threadPool(httpserver);
    printf("TheadPool create success\n");
    
    while(true){
        ret = epoll_wait( epollFd, epollEvents, 5, -1 );
        for( int i = 0; i < ret; ++i ){
            if ( epollEvents[i].data.fd == socketfd ) {
                int clientSocketFd = accept( socketfd, reinterpret_cast< struct sockaddr* >(&clientaddr), &clientaddrLength );
                printf("\n");
                dispAddrInfo( clientaddr );
                threadPool.appendFd( clientSocketFd );
                threadPool.notifyOneThread();
            }
        }
    }

    return 0;
}