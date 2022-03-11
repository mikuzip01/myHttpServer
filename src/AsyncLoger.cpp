#include "AsyncLoger.h"

std::shared_ptr<AsyncLoger> AsyncLoger::instance;
MutexLock AsyncLoger::singletonMutex;

Buffer::Buffer(size_t _bufferSize): bufferSize(_bufferSize), end(0){
    dataPtr = new char[bufferSize];
}

Buffer::~Buffer(){
    delete [] dataPtr;
}

bool Buffer::appendAvailable(size_t strLen){
    if( end + strLen + DATE_STRING_LEGTH >= bufferSize ){
        return false;
    }
    else{
        return true;
    }
}

void Buffer::append(const char* logLine, size_t len){
    // 输入日志信息
    memcpy(dataPtr + end, logLine, len);
    end += len;
}

inline bool Buffer::empty() const {
    return end == 0;
}

inline int Buffer::writeToFile(int fd){
    if( empty() ) return 1;

    int ret = write( fd, dataPtr, end );
    if( ret == -1 ) return -1;
    else{
        end = 0;
        return 1;
    }
}



AsyncLoger::AsyncLoger(std::string logName = "LogFile"):
    logFileFd(-1), 
    conditionVar(3),
    running(true)
    {
    // 构造文件名，日期取的是日志系统进行构造时的时间
    char tmpBuf[BUFLEN];   
    sprintCurTime(tmpBuf, BUFLEN);
    std::string fileName = logName + tmpBuf + ".log";
    // 就算打开失败也不要要在构造函数中抛出异常
    logFileFd = open( fileName.c_str(), O_WRONLY | O_CREAT , 777 );
    // 初始化空缓冲区
    for( int i = 0; i < 4; ++i ){
        emptyBuffers.push_back(std::unique_ptr<Buffer>(new Buffer(LOGBUFFERSIZE)));
    }
    // 指定起始缓冲区
    curBuffer = std::move(emptyBuffers.back());
    emptyBuffers.pop_back();

    // 启动后端线程
    pthread_create(&logBackGroundthreadId, nullptr, threadWrap, this);
}

AsyncLoger::~AsyncLoger(){
    running = false;
    conditionVar.notifyAll();
    pthread_join( logBackGroundthreadId, nullptr );
    if( logFileFd != -1 ){
        close( logFileFd );
    }
}


void AsyncLoger::threadFun(){
    std::vector<std::unique_ptr<Buffer>> writeQueue;
    writeQueue.reserve(4); // 提前申请空间
    while(running){
        {   // 收集需要写入磁盘的缓冲区
            MutexLockGuard mutexLockGuard(mutex);
            conditionVar.waitForTime(mutex);
            // 尝试取出当前缓冲区
            if( curBuffer ){
                writeQueue.push_back(std::move(curBuffer));
                if( !emptyBuffers.empty() ){
                    curBuffer = std::move(emptyBuffers.back());
                    emptyBuffers.pop_back();
                }
            }
            // 取出全部等待写入的缓冲区
            while( !toWriteBuffers.empty() ){
                writeQueue.push_back( std::move( toWriteBuffers.front() ) );
                toWriteBuffers.pop();
            }

        }

        // 写入磁盘
        while( !writeQueue.empty() ){
            writeQueue.back()->writeToFile(logFileFd);
            {   // 归还完成了写入的缓冲区
                MutexLockGuard mutexLockGuard(mutex);
                emptyBuffers.push_back( std::move( writeQueue.back() ) );
            }
            writeQueue.pop_back();
        }
       
    }
}

void* AsyncLoger::threadWrap(void* args){
    AsyncLoger* asyncLoger = reinterpret_cast<AsyncLoger*>(args);
    asyncLoger->threadFun();
}

// 用于前端线程提交日志信息
int AsyncLoger::logInfo(std::string& logLine){
    size_t logSize = logLine.length(); 
    {
        MutexLockGuard mutexLockGuard(mutex);
        if ( curBuffer == nullptr ) return -1; // 缓冲区全满，直接丢弃日志信息

        // 有可用缓冲区，尝试写入
        if( logLine.length() > LOGBUFFERSIZE / 2 ){ // 过长（大于缓冲区一半长度的日志）的部分直接舍弃
            memcpy( &logLine[LOGBUFFERSIZE / 2] - 17, "Log Info Too Long", 17);
            logSize = LOGBUFFERSIZE / 2;
        }

        if( curBuffer->appendAvailable(logSize + DATE_STRING_LEGTH) ){ // 当前缓冲区可用直接添加
            // 添加时间
            char dateStr[DATE_STRING_LEGTH];
            sprintCurTime(dateStr, DATE_STRING_LEGTH);
            dateStr[DATE_STRING_LEGTH - 1] = ' ';
            curBuffer->append(dateStr, DATE_STRING_LEGTH);
            // 添加信息
            curBuffer->append(logLine.c_str(), logSize);
        }
        else{ // 当前缓冲区不可用
            // 主动将当前缓冲区提交到后端
            toWriteBuffers.push(std::move(curBuffer)); // move后此时curBuffer为空指针
            conditionVar.notify();
            // 尝试拿一块新的空闲缓冲区
            // FIXME 在有curBuffer但没有emptyBuffers时，日志有可能被丢弃
            if(!emptyBuffers.empty()){
                curBuffer = std::move(emptyBuffers.back());
                emptyBuffers.pop_back();
                // 添加时间
                char dateStr[DATE_STRING_LEGTH];
                sprintCurTime(dateStr, DATE_STRING_LEGTH);
                dateStr[DATE_STRING_LEGTH - 1] = ' ';
                curBuffer->append(dateStr, DATE_STRING_LEGTH);
                // 添加信息
                curBuffer->append(logLine.c_str(), logSize);
            }// else 无空闲缓冲区可用，丢弃日志
        }
    }
}

int AsyncLoger::logInfoTempVar(std::string logline){
    logInfo( logline );
}

inline void sprintCurTime(char* str, int len){
    time_t t = time( 0 );   
    strftime(str, len, "%Y%m%d%H%M%S", localtime(&t));
}

void AsyncLoger::getInstance( std::shared_ptr<AsyncLoger>& sPtr ){
    {
        MutexLockGuard mutexLockGuard( singletonMutex );
        if( !instance ){
            instance = std::shared_ptr<AsyncLoger>(new AsyncLoger("RunTimeLog"), AsyncLoger::Deleter);
        }
        sPtr = instance;
    }
}

void AsyncLoger::Deleter(AsyncLoger* ptr){
    delete ptr;
}

#ifdef _ASYNCLOGER_TEST_

class TestData{
public:
    pthread_t threadId;
};

void* testFunc(void* args){
    TestData& testData = *reinterpret_cast<TestData*>(args);
    std::shared_ptr<AsyncLoger> log;
    AsyncLoger::getInstance(log);
    pthread_t threadId = testData.threadId;
    char temp[255];
    for( int i = 0; i < 1000; ++i ){
        sprintf( temp, "thread %d log a random number %d\n", static_cast<int>(threadId), rand() );
        log->logInfo(temp);
    }
}

int main(){
    TestData testData[4];
    for( int i = 0; i < 4; ++i ){
        pthread_create( &(testData[i].threadId), nullptr, testFunc, &testData[i] );
    }
    for( int i = 0; i < 4; ++i ){
        pthread_join( testData[i].threadId, nullptr );
    }
}

#endif