#include "HttpData.h"

HttpData::HttpData(int clientSocket) : prev( 0 ), clientSocket( clientSocket ), readIndex( HTTPDATA_BUFFERSIZE ), dataEndIndex( HTTPDATA_BUFFERSIZE - 1 ){
    memset( dataBuffer, 0, HTTPDATA_BUFFERSIZE);
}

void HttpData::parseData(){
    string firstLine( parseOneLine() );
    vector<string> headerInfo;
    stringSplit( firstLine, headerInfo, " " );
    getRequestMethod( headerInfo );

    if( requestMethod == UNSUPPORT ) return;
    
    url = headerInfo[ 1 ];
    version = headerInfo [ 2 ];
}

bool HttpData::dataBufferEmpty(){
    return readIndex > dataEndIndex || readIndex >= HTTPDATA_BUFFERSIZE;
}

// 根据CRLF标志符，将套接字中的数据分行取出
string HttpData::parseOneLine(){
    if( dataBufferEmpty() ) readRawDataFromSocket();
    char lineBuffer[ LINE_BUFFERSIZE ];
    int index = 0;
    while( readIndex <= dataEndIndex ){
        // if( index >= bufferSize ){ printf("buffer overflow!"); throw std::exception(); }
        
        lineBuffer[ index ]= dataBuffer[ readIndex ];
        ++index;
        ++readIndex;
        if( prev == '\r' && dataBuffer[ readIndex - 1 ] == '\n' ) { prev = 0; break;}
        prev = dataBuffer[ readIndex - 1 ];

        if(readIndex > dataEndIndex) readRawDataFromSocket();
    }
    lineBuffer[ index - 2 ] = '\0';
    return string( lineBuffer );
}

// 从套接字中取出一段数据
void HttpData::readRawDataFromSocket(){
    int dataNum = recv( clientSocket, dataBuffer, HTTPDATA_BUFFERSIZE, 0);
    if( dataNum < 0 ) throw std::exception();
    readIndex = 0;
    dataEndIndex = dataNum - 1;
}

// 根据制定字符分割字符串
void HttpData::stringSplit(const string& s, vector<string>& tokens, const string& delimiters)  // 分割字符串。默认用空格分割字符串
{
	string::size_type lastPos = s.find_first_not_of(delimiters, 0);
	string::size_type pos = s.find_first_of(delimiters, lastPos);
	while (string::npos != pos || string::npos != lastPos) {
		tokens.push_back(s.substr(lastPos, pos - lastPos));//use emplace_back after C++11
		lastPos = s.find_first_not_of(delimiters, pos);
		pos = s.find_first_of(delimiters, lastPos);
	}
}

void HttpData::getRequestMethod( const vector<string>& headerInfo ){
    if( headerInfo[0] == "GET" ) { requestMethod = RequestMethod::GET; requestMethod_s = "GET"; }
    else if( headerInfo[0] == "POST" ) { requestMethod = RequestMethod::POST; requestMethod_s = "POST"; }
    else { requestMethod = RequestMethod::UNSUPPORT; requestMethod_s = "UNSUPPORT"; }
}

const string& HttpData::getUrl(){
    return url;
}

const string& HttpData::getRequestMethod_s(){
    return requestMethod_s;
}
const string& HttpData::getVersion(){
    return version;
}