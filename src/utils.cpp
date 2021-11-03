#include "utils.h"

void bufferPrinter(char* buffer, const int bufferSize){
    if( buffer[ bufferSize - 1 ] != '\0' ) buffer[ bufferSize - 1 ] = '\0';
    printf("%s\n", buffer);
}

void setPortReuse( int socketfd ){
    int opt = 1;
    int len = sizeof(opt);
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &opt, len);
}

void sendHello( int sockedfd ){
    char buf[128];

    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    send(sockedfd, buf, strlen(buf), 0);

    sprintf(buf, "Content-type: text/html\r\n");
    send(sockedfd, buf, strlen(buf), 0);

    sprintf(buf, "\r\n");
    send(sockedfd, buf, strlen(buf), 0);

    sprintf(buf, "<P>Hello!\r\n");
    send(sockedfd, buf, strlen(buf), 0);

    // sleep(1);
    printf("send hello finish!\n");
}

void dispAddrInfo( struct sockaddr_in &addr){
    printf("client addr:%s, port:%d\n", inet_ntoa( addr.sin_addr ), ntohs( addr.sin_port ) );
}