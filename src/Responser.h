#ifndef __RESPONSER__
#define __RESPONSER__

#include <string.h>
#include <exception>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "sys/socket.h"
#include "HttpData.h"
#include "utils.h"

extern const int PAGE_BUFFER_SIZE;

class Responser{
public:
    Responser( const HttpData &_httpData ) : httpData( _httpData ), clientSocket( httpData.getClientSocket() ), isClose( false ) {}
    ~Responser();
    // 发送url对应的静态网页给客户机
    void sendStaticFileToClient();
    // 执行CGI
    void executeCGI();
    // 发送403网页给客户机
    void sendForBidden();
    // 发送404网页给客户机
    void sendNotFound();
    // 发送内存中的网页给客户机
    void sendMemoryPage();
    // 关闭和客户端的套截字
    void closePeerConnection();

private:
    // 200 OK 的响应头
    void okHeader( char [] );
    // 设置返回文件类型为静态HTML
    void htmlContentType( char [] );
    // 服务器上的静态文件
    void serversStaticFile();

    const HttpData &httpData;
    int clientSocket;
    bool isClose;
};

#endif