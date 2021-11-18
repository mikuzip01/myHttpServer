#include "utils.h"

char *memory_index_page = nullptr;

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