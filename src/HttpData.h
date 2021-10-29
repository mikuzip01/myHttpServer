#include <sys/socket.h>
#include <string.h>
#include <exception>
#include <stdio.h>
#include <string>
#include <vector>
#include <utility>

using std::string; using std::vector;

// 专门用于处理和客户机http数据的交互
const int HTTPDATA_BUFFERSIZE = 64;
const int LINE_BUFFERSIZE = 64;

class HttpData{
public:
    HttpData(int clientSocket);
    void parseData();
    const string& getUrl();
    const string& getRequestMethod_s();
    const string& getVersion();
private:
    string parseOneLine(); // 一次性从recv中读出一大段数据后续在来处理代替利用recv每次只从缓冲区取一个字符出来。理论上系统调用会有不少开销，以此减少对recv的调用次数
    void readRawDataFromSocket();
    bool dataBufferEmpty();

    void stringSplit(const string& s, vector<string>& tokens, const string& delimiters = " ");
    void getRequestMethod( const vector<string>& headerInfo );

    int clientSocket;
    char dataBuffer[ HTTPDATA_BUFFERSIZE ];
    int readIndex, dataEndIndex; // 指向未被读取的数据的第一个字符

    char prev;

    enum RequestMethod{ UNSUPPORT, GET, POST } requestMethod;
    string requestMethod_s;
    string url;
    string version;
};