#ifndef __THREADPOOL__
#define __THREADPOOL__

#include <vector>
#include <list>
#include <atomic>
#include "Timer.h"
#include "Mutex.h"



class ThreadTask{
    virtual void task(int clientSocket) = 0;
};

class TaskData{
public:
    TaskData( int _clientfd , Timer *_timer ) : clientfd( _clientfd ), timer( _timer ) {}
    int clientfd;
    Timer* timer;
};

class ThreadPool{
public:
    ThreadPool(void (*task)(TaskData), int threadNum = 4, int maxWorkListLen = 8);
    ~ThreadPool();
    // 返回线程池拥有的互斥锁
    MutexLock& getMutexLock(){
        return mutex;
    }
    // 向工作队列中添加客户连接，fixme 还没有做最大连接数量的限制
    void appendFd( TaskData taskData ){
        {
            MutexLockGuard mutexLockGuard( mutex );
            workList.push_back( taskData );
        }
        notifyOneThread();
    }

    void notifyOneThread(){
        cond.notify();
    }

private:
    // 标志线程池正在运行，设定为false之后池中的所有线程就退出事件循环
    std::atomic<bool> running;
    int threadNum;
    int maxWorkListLen;
    std::list< TaskData > workList;  // 用于接受来自主线程已经接受的socketfd
    std::vector< pthread_t > threads;
    MutexLock mutex;
    Condition cond;
    void (*task)(TaskData);  // task固定接受clinetSockfd，后期应该可以扩展

    static void* workerWarper( void* args );
    void worker();  // 非静态成员函数也不可以作为pthread_create的入口
};

#endif