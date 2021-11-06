#ifndef __THREADPOOL__
#define __THREADPOOL__

#include <vector>
#include <list>
#include <pthread.h>

// 互斥锁
class MutexLock{
public:
    MutexLock() {
        pthread_mutex_init(&mutex_, NULL);
    }
    ~MutexLock() {
        pthread_mutex_destroy(&mutex_);
    }
    void lock() {
        pthread_mutex_lock(&mutex_);
    }
    void unlock() {
        pthread_mutex_unlock(&mutex_);
    }
    pthread_mutex_t* getMutex() {
        return &mutex_;
    }
private:
    pthread_mutex_t mutex_;
};


class MutexLockGuard{

public:
    explicit MutexLockGuard(MutexLock &mutex): mutex_(mutex) {
        mutex_.lock();
    }

    ~MutexLockGuard() {
        mutex_.unlock();
    }
private:
    MutexLock &mutex_;
};


// 条件变量
class Condition{
public:
    explicit Condition(){
        pthread_cond_init(&cond_, NULL);
    }
    ~Condition() {
        pthread_cond_destroy(&cond_);
    }

    void wait( MutexLock &mutex ) {
        pthread_cond_wait(&cond_, mutex.getMutex());
    }

    void notify() {
        pthread_cond_signal(&cond_);
    }

    void notifyAll() {
        pthread_cond_broadcast(&cond_);
    }

private:
    pthread_cond_t cond_;
};

class ThreadTask{
    virtual void task(int clientSocket) = 0;
};

class ThreadPool{
public:
    ThreadPool(void (*task)(int), int threadNum = 4, int maxWorkListLen = 8);

    MutexLock& getMutexLock(){
        return mutex;
    }

    void appendFd( int fd ){
        {
            MutexLockGuard mutexLockGuard( mutex );
            workList.push_back( fd );
        }
    }

    void notifyOneThread(){
        cond.notify();
    }

private:
    int threadNum;
    int maxWorkListLen;
    std::list< int > workList;  // 用于接受来自主线程已经接受的socketfd
    std::vector< pthread_t > threads;
    MutexLock mutex;
    Condition cond;
    void (*task)(int);  // task固定接受clinetSockfd，后期应该可以扩展

    static void* worker( void* args );  // 非静态成员函数也不可以作为pthread_create的入口
};

#endif