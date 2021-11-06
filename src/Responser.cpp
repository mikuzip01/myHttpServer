#include "Responser.h"
#include "Pages.h"

const int PAGE_BUFFER_SIZE = 2048;

void Responser::sendStaticFileToClient(){
    struct stat staticFileState;
    int ret = stat( httpData.getUrl().c_str(), &staticFileState );

    if( ret == -1 ){  // 没有对应的文件和文件夹或拒绝存取
        if( errno == EACCES ) sendForBidden();
        else sendNotFound();  
    }
    else{
        char buf [ PAGE_BUFFER_SIZE ];
        okHeader( buf );
        serversStaticFile( buf );
        send( clientSocket, buf, strlen( buf ), 0 );
    }

}

void Responser::serversStaticFile( char buf[] ){
    FILE *resource = NULL;
    resource = fopen( httpData.getUrl().c_str() , "r");  // 以只读方法打开url指定的文件
    char readBuf [ 256 ];
    memset( readBuf, 0, 256 );
    fgets(readBuf, 256, resource);  // 
    while (!feof(resource))
    {   
        strcat( buf, readBuf );
        fgets(readBuf, 256, resource);
    }
    fclose( resource );
}

void Responser::okHeader( char buf[] ){
    sprintf(buf, "HTTP/1.0 200 OK\r\n" );
    sprintf(buf, "%s%s", buf, SERVER_NAME );
    sprintf(buf, "%s%s", buf, "Content-Type: text/html\r\n" );
    sprintf(buf, "%s%s", buf, "\r\n" );  // 和page body的分界线
}

void Responser::sendNotFound(){
    char buf[1024];
    sprintf(buf, "HTTP/1.0 404 NOT FOUND\r\n" );
    sprintf(buf, "%s%s", buf, SERVER_NAME );
    sprintf(buf, "%s%s", buf, "Content-Type: text/html\r\n" );
    sprintf(buf, "%s%s", buf, "\r\n" );
    sprintf(buf, "%s%s", buf, NOT_FOUND_PAGE );
    int ret = send( clientSocket, buf, strlen( buf ), 0 );
    if( ret == -1 ) throw std::runtime_error("send page to client error!");
}

void Responser::sendForBidden(){
    char buf[1024];
    sprintf(buf, "HTTP/1.0 403 Forbidden\r\n" );
    sprintf(buf, "%s%s", buf, SERVER_NAME );
    sprintf(buf, "%s%s", buf, "Content-Type: text/html\r\n" );
    sprintf(buf, "%s%s", buf, "\r\n" );
    sprintf(buf, "%s%s", buf, NOT_FOUND_PAGE );
    int ret = send( clientSocket, buf, strlen( buf ), 0 );
    if( ret == -1 ) throw std::runtime_error("send page to client error!");
}