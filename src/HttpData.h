#include <sys/socket.h>
#include <string.h>
#include <exception>
#include <stdio.h>
#include <string>
#include <vector>
#include <utility>
#include <memory>

using std::string; using std::vector;
using std::shared_ptr;

const string NULLINFO("NULL");

// 专门用于处理和客户机http数据的交互
const int HTTPDATA_BUFFERSIZE = 64;
const int LINE_BUFFERSIZE = 256;

class HttpData{
public:
    HttpData(int clientSocket);
    void parseData();
    const string& getUrl();
    const string& getRequestMethod_s();
    const string& getVersion();
    const string& getUserAgent(){ return userAgent ? *userAgent : NULLINFO; }
private:
    // 根据CRLF标志符，将套接字中的数据分行取出。读取成功的最后\r\n会被替换成\0
    string parseOneLine(); // 一次性从recv中读出一大段数据后续在来处理代替利用recv每次只从缓冲区取一个字符出来。理论上系统调用会有不少开销，以此减少对recv的调用次数

    // get requestmethod, url and http verison
    void parseStartLine();
    // 解析剩余的头部信息
    void parseHeader();
    // 从套接字中取出一段数据
    void readRawDataFromSocket();
    bool dataBufferEmpty();
    // 获得请求头的名称
    string getHeaderLineName( string &s );

    // 根据指定字符分割字符串
    void stringSplit(const string& s, vector<string>& tokens, const string& delimiters = " ");
    void getRequestMethod( const vector<string>& headerInfo );

    int clientSocket;
    char dataBuffer[ HTTPDATA_BUFFERSIZE ];
    int readIndex, dataEndIndex; // 指向未被读取的数据的第一个字符

    char prev;

    // data
    enum RequestMethod{ UNSUPPORT, GET, POST } requestMethod;
    string requestMethod_string, url, version;
    shared_ptr<string> host, userAgent, accept, acceptLanguage, acceptEncoding, connection, upgradeInsecurceRequests, contentType, contentLength; // 弄成堆上对象，因为有可能为空。用智能指针来管理
};