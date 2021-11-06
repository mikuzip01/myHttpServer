#ifndef __RESPONSER__
#define __RESPONSER__

#include <string.h>
#include <exception>
#include <unistd.h>
#include <sys/stat.h>
#include "sys/socket.h"
#include "HttpData.h"
#include "utils.h"

extern const int PAGE_BUFFER_SIZE;

class Responser{
public:
    Responser( const HttpData &_httpData ) : httpData( _httpData ), clientSocket( httpData.getClientSocket() ) {}
    // 发送url对应的静态网页给客户机
    void sendStaticFileToClient();

private:
    // 发送403网页给客户机
    void sendForBidden();
    // 发送404网页给客户机
    void sendNotFound();
    // 200 OK 的响应头
    void okHeader( char [] );
    // 服务器上的静态文件
    void serversStaticFile( char [] );
    const HttpData &httpData;
    int clientSocket;
};

#endif