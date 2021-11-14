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
        htmlContentType( buf );
        serversStaticFile( buf );
        send( clientSocket, buf, strlen( buf ), 0 );
    }
}

Responser::~Responser(){
    closePeerConnection();
}

void Responser::executeCGI(){
    char buf [ PAGE_BUFFER_SIZE ];
    memset( buf, 0, PAGE_BUFFER_SIZE );
    okHeader( buf );
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
        size_t dataCount = (*httpData.getPostBodyData()).size();
        char env_dataCount[256];
        sprintf( env_dataCount, "DATACOUNT=%d", dataCount);
        putenv(env_dataCount);
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
        const auto postData = httpData.getPostBodyData();
        for( const auto &iter : (*postData) ){
            write( cgi_input[ 1 ], iter.first.c_str(), strlen( iter.first.c_str() ) );
            write( cgi_input[ 1 ], " ", 1);
            write( cgi_input[ 1 ], iter.second.c_str(), strlen( iter.second.c_str() ) );
            write( cgi_input[ 1 ], "\n", 1);
        }
        write( cgi_input[ 1 ], "EOF\n", 1);
        int ret;
        while (true)
        {
            ret = read( cgi_output[ 0 ], buf + strlen( buf ), PAGE_BUFFER_SIZE - strlen( buf ) );
            if( ret == -1 ) throw std::runtime_error("read() failed");
            if( ret == 0 ) break;
        }
        ret = send( clientSocket, buf, strlen( buf ), 0 );
        if( ret == -1 ) throw std::runtime_error("send() failed");
    }
    waitpid( pid, &status, 0); // 必须对CGI进程调用waitpid，不然就会产生僵尸进程
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
    
}

void Responser::htmlContentType( char buf[] ){
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
    if( ret == -1 ) throw std::runtime_error("send page to client error!\n");
}

void Responser::sendForBidden(){
    char buf[1024];
    sprintf(buf, "HTTP/1.0 403 Forbidden\r\n" );
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