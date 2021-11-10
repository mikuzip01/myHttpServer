#ifndef __HTTPDATA__
#define __HTTPDATA__

#include <sys/socket.h>
#include <string.h>
#include <exception>
#include <stdio.h>
#include <string>
#include <vector>
#include <utility>
#include <memory>
#include <unordered_map>

using std::string; using std::vector;
using std::shared_ptr;

extern const string NULLINFO;

// 专门用于处理和客户机http数据的交互
const int HTTPDATA_BUFFERSIZE = 64;
const int LINE_BUFFERSIZE = 256;

class HttpData{
public:
    HttpData(int clientSocket);
    enum RequestMethod{ UNSUPPORT, GET, POST };
    // 解析从客户机发来的所有信息
    void parseData();
    const string& getUrl() const { return url; }
    const string& getRequestMethod_string() const { return requestMethod_string; }
    const int getRequestMethod() const { return requestMethod; }
    const string& getVersion() const { return version; }
    const string& getUserAgent() const { return userAgent ? *userAgent : NULLINFO; }
    int getClientSocket() const { return clientSocket; }

private:
    // 根据CRLF标志符，将套接字中的数据分行取出。读取成功的最后\r\n会被替换成\0
    string parseOneLine(); // 一次性从recv中读出一大段数据后续在来处理代替利用recv每次只从缓冲区取一个字符出来。理论上系统调用会有不少开销，以此减少对recv的调用次数

    // get requestmethod, url and http verison
    void parseStartLine();
    // 解析剩余的头部信息
    void parseHeader();
    // 读取body中的POST数据
    void parsePostData();
    // 将content-length字符串转换为数字
    int numeralContentLength();
    // 从套接字中取出一段数据
    void readRawDataFromSocket();
    inline bool dataBufferEmpty(){ return readIndex > dataEndIndex || readIndex >= HTTPDATA_BUFFERSIZE; };
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
    RequestMethod requestMethod;
    string requestMethod_string, url, version;
    shared_ptr<string> host, userAgent, accept, acceptLanguage, acceptEncoding, connection, upgradeInsecurceRequests, contentType, contentLength; // 弄成堆上对象，因为有可能为空。用智能指针来管理
    shared_ptr< std::unordered_map< string, string > > postBodyData;  // 利用哈希表保存post中的键值对
};

#endif