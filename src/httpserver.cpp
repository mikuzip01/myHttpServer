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
#include <signal.h>
#include "ThreadPool.h"
#include "utils.h"
#include "HttpData.h"
#include "Responser.h"
#include "Timer.h"
#include "Error.h"
#include "AsyncLoger.h"


using std::string; using std::exception; using std::runtime_error;
using std::cout; using std::endl;

const int BUFFERSIZE = 1024;
// 长链接的超时事件，单位秒钟，默认15秒，调试阶段设置为5秒
const int TIMEOUT = 15;

// 统一信号源的管道
static int pipeline[2];


// 信号处理句柄
void signalHandler( int sig ){
    int saveErrno = errno;
    int msg = sig;
    write( pipeline[ 1 ], reinterpret_cast<char*>( &msg ), 1 ); // fixme 
    errno = saveErrno;
}

// 设置句柄
void addSingal( int sig ){
    struct sigaction sa;
    memset( &sa, 0, sizeof( sa ) );
    sa.sa_handler = signalHandler;
    sa.sa_flags |= SA_RESTART; // ??????这个是做什么的
    sigfillset( &sa.sa_mask ); // 利用sigfillset函数初始化sa_mask，设置所有的信号
    sigaction( sig, &sa, nullptr );
}


void httpserver( TaskData taskData ){
    int clientSocketFd = taskData.clientfd;
    std::shared_ptr<AsyncLoger> loger;
    AsyncLoger::getInstance(loger);
    try{
        HttpData httpData( clientSocketFd );
        httpData.parseData();
        // 一旦被标记为badRequest或者UNSUPPORT，那么httpData中的大部分数据都将不会生成。因此在要继续访问httpData前需要进行一次检查
        if( httpData.badRequest() ) { 
            #ifdef __PRINT_INFO_TO_DISP__
            printf("bad request\n"); 
            #endif
            #ifdef __LOG_INFO__
            #endif
            return; }
        if( httpData.getRequestMethod() == HttpData::UNSUPPORT ) { 
            #ifdef __PRINT_INFO_TO_DISP__
            printf("unsupport method\n"); 
            #endif
            #ifdef __LOG_INFO__
            #endif
            return; }

        Responser responser( httpData );

        #ifdef __PRINT_INFO_TO_DISP__
        printf("requset method: %s, url: %s, http verison: %s\n", httpData.getRequestMethod_string().c_str(), httpData.getUrl().c_str(), httpData.getVersion().c_str());
        #endif
        #ifdef __LOG_INFO__
        loger->logInfoTempVar(std::string("requset method: ") + httpData.getRequestMethod_string() + ", url:" + httpData.getUrl() + ", http verison: " +  httpData.getVersion() + "\n");
        #endif
        // printf("%s\n", httpData.getUserAgent().c_str() );

        if( !fileExist( httpData.getUrl().c_str() ) && httpData.getUrlResourceType() != "memory" ){ responser.sendNotFound(); return; }

        if( httpData.getRequestMethod() == HttpData::RequestMethod::GET ){
            if(httpData.getUrlResourceType() == "cgi") responser.executeCGI();
            else if( httpData.getUrlResourceType() == "memory") responser.sendMemoryPage();
            else responser.sendStaticFileToClient();;
        }
        else if( httpData.getRequestMethod() == HttpData::RequestMethod::POST ){
            if( httpData.getUrlResourceType() == "cgi" ) responser.executeCGI();
            else responser.sendNotFound();
        }
        
        // 设置长连接
        if( httpData.keepConnection() ){
            taskData.timer->addfd( clientSocketFd );
        }
        else{
            close( clientSocketFd );
        }
    }
    catch( exception &e ){
        taskData.timer->deleteFd( clientSocketFd );
        close( clientSocketFd );
    }


}


int main(){ 

    string ip = "127.0.0.1";
    int port = 12345;
    std::shared_ptr<AsyncLoger> loger;
    AsyncLoger::getInstance(loger);

    int serverSocketfd = socket( AF_INET, SOCK_STREAM, 0 );
    if( serverSocketfd == -1 ) throw runtime_error("socket create failed\n");
    printf("socket create success\n");
    loger->logInfoTempVar("socket create success\n");
    
    setAddrReuse( serverSocketfd );
    setFdNonblock( serverSocketfd );

    struct sockaddr_in ipaddr;
    ipaddr.sin_family = AF_INET;
    ipaddr.sin_port = htons( port );
    ipaddr.sin_addr.s_addr = inet_addr( ip.c_str() );  


    int ret = bind( serverSocketfd, reinterpret_cast< struct sockaddr*>( &ipaddr ), sizeof( ipaddr ) );
    if( ret == -1 ) throw runtime_error("bind ipaddr failed\n");
    printf("bind ipaddr create success\n");
    loger->logInfoTempVar("bind ipaddr create success\n");
    

    ret = listen( serverSocketfd, 20 );
    if( ret == -1 ) throw runtime_error("call listen failed\n");
    printf("listen create success\n");
    loger->logInfoTempVar("listen create success\n");

    ret = pipe( pipeline );
    if( ret == -1 ) throw runtime_error("pipeline create failed\n");
    printf("pipeline create success\n");
    loger->logInfoTempVar("pipeline create success\n");

    struct sockaddr_in clientaddr;
    socklen_t clientaddrLength = sizeof( clientaddr );


    int epollFd = epoll_create(5);
    addFdToEpoll_INET( epollFd, serverSocketfd );
    addFdToEpoll_INLT( epollFd, pipeline[ 0 ] );

    struct epoll_event epollEvents[5];
    ThreadPool threadPool(httpserver, 4);
    printf("TheadPool create success\n");
    loger->logInfoTempVar("TheadPool create success\n");

    loadIndexTomemory("www/index.html");
    printf("load index page to memory success\n");
    loger->logInfoTempVar("load index page to memory success\n");

    addSingal( SIGALRM );
    setFdNonblock( pipeline[ 1 ] );
    alarm( 1 ); // 一秒后触发一次定时。单次有效，不会循环触发
    Timer timer( epollFd, TIMEOUT ); // 定义管理长链接的定时器类
    char signals[ 128 ];
    
    while(true){
        ret = epoll_wait( epollFd, epollEvents, 5, -1 );
        for( int i = 0; i < ret; ++i ){
            if ( epollEvents[i].data.fd == serverSocketfd ) {
                int clientSocketFd = -1;
                while( (clientSocketFd = accept( serverSocketfd, reinterpret_cast< struct sockaddr* >(&clientaddr), &clientaddrLength ) ) != -1 )
                {
                    #ifdef __PRINT_INFO_TO_DISP__ 
                    printf("\n");
                    dispAddrInfo( clientaddr );
                    #endif
                    #ifdef __LOG_INFO__
                    std::string logline;
                    dispPeerConnection(epollEvents[i].data.fd, logline);
                    loger->logInfo(logline);
                    #endif
                    
                    threadPool.appendFd( TaskData( clientSocketFd, &timer ) );
                }
            }
            else if( epollEvents[i].data.fd != pipeline[ 0 ] ){ // 再一次被激活的长链接
                timer.forbidenFd( epollEvents[i].data.fd ); // 防止复用的链接在较长时间的处理过程中（例如传大文件）时被定时器给关闭
                #ifdef __PRINT_INFO_TO_DISP__
                printf("\n");
                printf("reused connection:\n");
                dispPeerConnection( epollEvents[i].data.fd );
                #endif
                #ifdef __LOG_INFO__
                std::string logline("reused connection: ");
                dispPeerConnection(epollEvents[i].data.fd, logline);
                loger->logInfo(logline);
                #endif

                threadPool.appendFd( TaskData( epollEvents[i].data.fd, &timer ) );
            }
            else{ // 收到时间信号
                int sigNums = read( pipeline[ 0 ], signals, 128 );
                for( int i = 0; i < sigNums; ++i ){
                    if( signals[ i ] == SIGALRM ){
                        // 执行定时器
                        timer.deleteExpiredFd();
                        alarm( 1 );
                    }
                    else{
                        continue;
                    }
                }
                
            }
        }
    }

    return 0;
}