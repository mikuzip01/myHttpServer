#include "Responser.h"
#include "Pages.h"

const int PAGE_BUFFER_SIZE = 2048;

void Responser::sendStaticFileToClient(){
    struct stat staticFileState;
    int ret = stat( httpData.getUrl().c_str(), &staticFileState );
    size_t fileSize = staticFileState.st_size;

    if( ret == -1 ){  // 没有对应的文件和文件夹或拒绝存取
        if( errno == EACCES ) sendForBidden();
        else sendNotFound();  
    }
    else{
        char buf [ PAGE_BUFFER_SIZE ];

        fisrtLine_200( buf );
        if( httpData.keepConnection() ) header_keepAlive( buf );

        if( httpData.getUrlResourceType() == "png" ) header_pngContentType( buf );
        else header_htmlContentType( buf );
        
        header_contentLength( buf, fileSize );
        header_body( buf );
        ret = send( clientSocket, buf, strlen( buf ), MSG_NOSIGNAL );
        if( ret == -1 ) throw SocketClosed();
        serversStaticFile();
    }
}

Responser::~Responser(){
    // closePeerConnection();
}

void Responser::executeCGI(){
    if( httpData.getParamCount() == 0 ){ // 非空处理, 发现非法直接就不执行
        sendForBidden();
        return;
    }

    char buf [ PAGE_BUFFER_SIZE ];
    memset( buf, 0, PAGE_BUFFER_SIZE );
    fisrtLine_200( buf );
    int cgi_output[2]; // 负责CGI进程到父进程的信息传递
    int cgi_input[2]; // 负责父进程到CGI的信息传递
    pid_t pid;
    int status;
    // 创建父进程和CGI进程之间的数据传输管道
    if( pipe( cgi_output ) == -1 ) throw std::runtime_error("cgi_output pipe create failed!\n");
    if( pipe( cgi_input ) == -1 ) throw std::runtime_error("cgi_input pipe create failed!\n");

    pid = fork();
    if( pid == -1 ) throw std::runtime_error("fork CGI process error!\n");

    if( pid == 0 ){ // CGI 进程
        // CGI中需要关闭管道的读端
        close( cgi_output[ 0 ] );
        // CGI中需要关闭管道的写端
        close( cgi_input[ 1 ] );
        // 将标准输入(scanf)接到输入管道的读端
        dup2( cgi_input[ 0 ], 0 );
        // 将标准输出(printf)接到输出管道的写端
        dup2( cgi_output[ 1 ], 1);
        // 统计POST数据的数量作为环境变量输入CGI程序
        size_t dataCount = (*httpData.getClientParamData()).size();
        char env_str[256];
        // cgiContentLength()直接计算最后的长度，目前没有扩扩展性
        sprintf( env_str, "CGIDATA=%d %d", dataCount, cgiContentLength() ); // 数据的数目，输入字符的长度
        putenv(env_str);
        // 执行CGI程序
        execl( httpData.getUrl().c_str() , httpData.getUrl().c_str(), NULL);
        // CGI进程关闭时就自动的调用了close关闭了还打开的所有的文件描述符
        exit(0);
    }
    else{ // 父进程
        // 父进程中需要关闭管道的写端
        close( cgi_output[ 1 ] );  // !!!
        // 父进程中需要关闭管道的读端
        close( cgi_input[ 0 ] );
        const auto postData = httpData.getClientParamData();
        for( const auto &iter : (*postData) ){
            write( cgi_input[ 1 ], iter.first.c_str(), strlen( iter.first.c_str() ) );
            write( cgi_input[ 1 ], " ", 1);
            write( cgi_input[ 1 ], iter.second == "" ? NULLINFO.c_str() : iter.second.c_str(), iter.second == "" ? strlen( NULLINFO.c_str() ) : strlen( iter.second.c_str() ) );
            write( cgi_input[ 1 ], "\n", 1);
        }
        // write( cgi_input[ 1 ], "EOF\n", 1);
        int ret;
        while (true)
        {
            ret = read( cgi_output[ 0 ], buf + strlen( buf ), PAGE_BUFFER_SIZE - strlen( buf ) );
            if( ret == -1 ) throw std::runtime_error("read() failed");
            if( ret == 0 ) break;
        }
        ret = send( clientSocket, buf, strlen( buf ), MSG_NOSIGNAL );
        if( ret == -1 ) throw SocketClosed();
    }
    waitpid( pid, &status, 0); // 必须对CGI进程调用waitpid，不然就会产生僵尸进程
}

void Responser::serversStaticFile(){
    FILE *resource = NULL;
    char readBuf [ PAGE_BUFFER_SIZE ];
    if ( httpData.getUrlResourceType() != "html"){ // 读取二进制文件
        resource = fopen( httpData.getUrl().c_str() , "rb");
        int count = 1;
        while ( true )
        {
            count = fread( readBuf, 1, PAGE_BUFFER_SIZE, resource ); // 第一个size_t指的是接受数据的内存区域的每个元素的大小，这里是char[]所以填1；第二个size_t指的有多少个元素
            if( count == 0 ) break;
            else{
                int ret = send( clientSocket, readBuf, count, MSG_NOSIGNAL );
                if( ret == -1 ) throw SocketClosed();
            }
        } 
    }
    else { // 读取文本文件
        resource = fopen( httpData.getUrl().c_str() , "r");  // 以只读方法打开url指定的文件
        memset( readBuf, 0, PAGE_BUFFER_SIZE );
        fgets(readBuf, PAGE_BUFFER_SIZE, resource);  // 
        while (!feof(resource))
        {   
            fgets(readBuf, PAGE_BUFFER_SIZE, resource);
            int ret = send( clientSocket, readBuf, strlen( readBuf ), MSG_NOSIGNAL );
            if( ret == -1 ) throw SocketClosed();
        }
    }
    fclose( resource );
}

void Responser::fisrtLine_200( char buf[] ){
    sprintf(buf, "HTTP/1.1 200 OK\r\n" );
    sprintf(buf, "%s%s", buf, SERVER_NAME );
    
}

void Responser::header_htmlContentType( char buf[] ){
    sprintf(buf, "%s%s", buf, "Content-Type: text/html\r\n" );
    
}

void Responser::header_pngContentType( char buf[] ){
    sprintf(buf, "%s%s", buf, "Content-Type: image/png\r\n" );
}

void Responser::header_body( char buf[] ){
    sprintf(buf, "%s%s", buf, "\r\n" );  // 和page body的分界线
}

void Responser::header_keepAlive( char buf[] ){
    sprintf( buf, "%s%s", buf, "Connection: keep-alive\r\n" );
    sprintf( buf, "%s%s", buf, "Keep-Alive: \r\n" ); // rfc2068标准说http1.1的keep alive header默认没有参数
}

void Responser::header_contentLength( char buf[], size_t fileSize ){
    sprintf( buf, "%s%s%d%s", buf, "Content-Length: ", fileSize, "\r\n" );
}

void Responser::sendNotFound(){
    char buf[1024];
    sprintf(buf, "HTTP/1.1 404 NOT FOUND\r\n" );
    sprintf(buf, "%s%s", buf, SERVER_NAME );
    sprintf(buf, "%s%s", buf, "Content-Type: text/html\r\n" );
    sprintf(buf, "%s%s", buf, "\r\n" );
    sprintf(buf, "%s%s", buf, NOT_FOUND_PAGE );
    int ret = send( clientSocket, buf, strlen( buf ), 0 );
    if( ret == -1 ) throw SocketClosed();
}

void Responser::sendForBidden(){
    char buf[1024];
    sprintf(buf, "HTTP/1.1 403 Forbidden\r\n" );
    sprintf(buf, "%s%s", buf, SERVER_NAME );
    sprintf(buf, "%s%s", buf, "Content-Type: text/html\r\n" );
    sprintf(buf, "%s%s", buf, "\r\n" );
    sprintf(buf, "%s%s", buf, NOT_FOUND_PAGE );
    int ret = send( clientSocket, buf, strlen( buf ), 0 );
    if( ret == -1 ) throw std::runtime_error("send page to client error!\n");
}

void Responser::closePeerConnection(){
    if( !isClose ){
        close( clientSocket );
        isClose = true;
    }
}

void Responser::sendMemoryPage(){
    char buf [ PAGE_BUFFER_SIZE ];
    fisrtLine_200( buf );
    header_htmlContentType( buf );
    sprintf(buf, "%s%s", buf, memory_index_page );
    send( clientSocket, buf, strlen( buf ), 0 );
}

int Responser::cgiContentLength(){
    int ans = 0;
    for( auto& iter : *(httpData.getClientParamData() ) ){
        ans += iter.first.size();
        ans += iter.second == "" ? NULLINFO.size() : iter.second.size();
    }
    return ans;
}