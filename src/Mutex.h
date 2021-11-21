#ifndef __MUTEX__
#define __MUTEX__
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


#endif __MUTEX__