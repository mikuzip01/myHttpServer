#include "utils.h"

char *memory_index_page = nullptr;

void bufferPrinter(char* buffer, const int bufferSize){
    if( buffer[ bufferSize - 1 ] != '\0' ) buffer[ bufferSize - 1 ] = '\0';
    printf("%s\n", buffer);
}

void setAddrReuse( int socketfd ){
    int opt = 1;
    int len = sizeof(opt);
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &opt, len);
}

void sendHello( int sockedfd ){
    char buf[128];
    int ret = 0;
    sprintf(buf, "HTTP/1.1 200 OK\r\n");
    ret = send(sockedfd, buf, strlen(buf), MSG_NOSIGNAL );
    if( ret == -1 ) throw SocketClosed();

    sprintf(buf, "Content-type: text/html\r\n");
    ret = send(sockedfd, buf, strlen(buf), 0);
    if( ret == -1 ) throw SocketClosed();

    sprintf(buf, "\r\n");
    ret = send(sockedfd, buf, strlen(buf), 0);
    if( ret == -1 ) throw SocketClosed();

    sprintf(buf, "<P>Hello!\r\n");
    ret = send(sockedfd, buf, strlen(buf), 0);
    if( ret == -1 ) throw SocketClosed();

    // sleep(1);
    printf("send hello finish!\n");
}

void dispAddrInfo( struct sockaddr_in &addr){
    printf("client addr:%s, port:%d\n", inet_ntoa( addr.sin_addr ), ntohs( addr.sin_port ) );
}

void sprintAddrInfo( struct sockaddr_in &addr, std::string& str){
    char cs[128];
    sprintf(cs, "client addr:%s, port:%d\n", inet_ntoa( addr.sin_addr ), ntohs( addr.sin_port ) );
    str += cs;
}

bool fileExist(const char path [] ){
    struct stat staticFileState;
    int ret = stat( path, &staticFileState );
    if( ret == -1 ){  // 没有对应的文件和文件夹或拒绝存取
        return false; 
    }
    else return true;
}

void loadIndexTomemory( std::string file_path ){
    struct stat staticFileState;
    int ret = stat( file_path.c_str(), &staticFileState );
    memory_index_page = new char[ staticFileState.st_size + 1 ];
    FILE *resource = NULL;
    resource = fopen( file_path.c_str() , "r");  // 以只读方法打开url指定的文件
    char readBuf [ 256 ];
    memset( readBuf, 0, 256 );
    fgets(readBuf, 256, resource);  // 
    while (!feof(resource))
    {   
        strcat( memory_index_page, readBuf );
        fgets(readBuf, 256, resource);
    }
    fclose( resource );
}

int setFdNonblock( int fd ){
    int old_options = fcntl( fd, F_GETFL );
    int new_options = old_options | O_NONBLOCK;
    fcntl( fd, F_SETFL, new_options );
    return old_options;
}

void addFdToEpoll_INLT( int epollfd, int addedfd ){
    struct epoll_event epollEvent;
    memset( &epollEvent, 0, sizeof( epollEvent ) );
    epollEvent.data.fd = addedfd;
    epollEvent.events = ( EPOLLIN ); // 可读事件 + 电平触发模式
    epoll_ctl( epollfd, EPOLL_CTL_ADD, addedfd, &epollEvent); // socketfd在epoll_event需要设置在epoll_ctl()函数中也需要设置？
}


void addFdToEpoll_INET( int epollfd, int addedfd ){
    struct epoll_event epollEvent;
    memset( &epollEvent, 0, sizeof( epollEvent ) );
    epollEvent.data.fd = addedfd;
    epollEvent.events = ( EPOLLIN | EPOLLET ); // 可读事件 + 边缘触发模式
    epoll_ctl( epollfd, EPOLL_CTL_ADD, addedfd, &epollEvent); // socketfd在epoll_event需要设置在epoll_ctl()函数中也需要设置？
}

void addFdToEpoll_INETONESHOT( int epollfd, int addedfd ){
    struct epoll_event epollEvent;
    memset( &epollEvent, 0, sizeof( epollEvent ) );
    epollEvent.data.fd = addedfd;
    epollEvent.events = ( EPOLLIN | EPOLLET | EPOLLONESHOT ); // 可读事件 + 边缘单次触发模式
    epoll_ctl( epollfd, EPOLL_CTL_ADD, addedfd, &epollEvent); // socketfd在epoll_event需要设置在epoll_ctl()函数中也需要设置？
}

void resetOneshot_INETONESHOT( int epollfd, int resetfd ){
    struct epoll_event epollEvent;
    memset( &epollEvent, 0, sizeof( epollEvent ) );
    epollEvent.data.fd = resetfd;
    epollEvent.events = ( EPOLLIN | EPOLLET | EPOLLONESHOT ); // 可读事件 + 边缘单次触发模式
    epoll_ctl( epollfd, EPOLL_CTL_MOD, resetfd, &epollEvent);
}

void deleteFdFromEpoll( int epollfd, int targetfd ){
    epoll_ctl( epollfd, EPOLL_CTL_DEL, targetfd, nullptr );
}

void dispPeerConnection( int clientFd ){
    struct sockaddr_in clientaddr;
    socklen_t clientaddrLength = sizeof( clientaddr );
    getpeername( clientFd, reinterpret_cast< struct sockaddr* >(&clientaddr), &clientaddrLength);
    dispAddrInfo( clientaddr );
}

void dispPeerConnection( int clientFd , std::string& str){
    struct sockaddr_in clientaddr;
    socklen_t clientaddrLength = sizeof( clientaddr );
    getpeername( clientFd, reinterpret_cast< struct sockaddr* >(&clientaddr), &clientaddrLength);
    sprintAddrInfo( clientaddr, str);
}
