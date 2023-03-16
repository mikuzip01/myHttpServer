#ifndef __ASYNCLOGER__
#define __ASYNCLOGER__

// #define _ASYNCLOGER_TEST_

#include <fcntl.h>
#include <string>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <algorithm>
#include <vector>
#include <queue>
#include <atomic>
#include <memory>
#include "Mutex.h"

#define BUFLEN 256
// 每个缓冲区的大小为1M
static const int LOGBUFFERSIZE = 1 * 1024 * 1024;

// 有效信息固定14个Char
static const int DATE_STRING_LEGTH = 15;
void sprintCurTime(char*, int);

class Buffer{
public:
    Buffer(size_t);
    ~Buffer();
    // 向buffer添加字符串信息
    void append(const char*, size_t len);
    // 测试
    bool appendAvailable(size_t strLen);
    bool empty() const;
    int writeToFile(int fd);
private:
    size_t bufferSize, end;
    char* dataPtr;
};

// 支持多线程的异步日志记录器
class AsyncLoger{
public:
    // 单例模式
    static void getInstance(std::shared_ptr<AsyncLoger>&);
    // 用于前端线程提交日志信息
    int logInfo(std::string&);
    int logInfoTempVar( std::string );
private:
    AsyncLoger(std::string);
    ~AsyncLoger();
    // 单例模式
    static std::shared_ptr<AsyncLoger> instance;
    static MutexLock singletonMutex;
    AsyncLoger& operator=(const AsyncLoger&) = delete;
    AsyncLoger(const AsyncLoger&) = delete;
    static void Deleter(AsyncLoger* );
    // 后端的写入线程
    void threadFun();
    static void* threadWrap(void*);
    int logFileFd;
    pthread_t logBackGroundthreadId;
    MutexLock mutex;
    ConditionTimeWait conditionVar;
    // 双缓冲区
    std::unique_ptr<Buffer> curBuffer;
    std::vector<std::unique_ptr<Buffer>> emptyBuffers;
    std::queue<std::unique_ptr<Buffer>> toWriteBuffers;
    std::atomic<bool> running;
};
#endif