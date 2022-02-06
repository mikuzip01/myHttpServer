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

// 等待时间默认一秒钟
class ConditionTimeWait{
public:
    explicit ConditionTimeWait( int _waitSecond = 1 ) : waitSecond( _waitSecond ){
        pthread_cond_init(&cond_, NULL);
    }
    ~ConditionTimeWait() {
        pthread_cond_destroy(&cond_);
    }

    void waitForTime( MutexLock &mutex ) {
        struct timespec abstime;
        clock_gettime(CLOCK_REALTIME, &abstime);

        const int64_t kNanoSecondsPerSecond = 1000000000;
        int64_t nanoseconds = static_cast<int64_t>(waitSecond * kNanoSecondsPerSecond);

        abstime.tv_sec += static_cast<time_t>((abstime.tv_nsec + nanoseconds) / kNanoSecondsPerSecond);
        abstime.tv_nsec = static_cast<long>((abstime.tv_nsec + nanoseconds) % kNanoSecondsPerSecond);

        pthread_cond_timedwait(&cond_, mutex.getMutex(), &abstime);
    }

    void notify() {
        pthread_cond_signal(&cond_);
    }

    void notifyAll() {
        pthread_cond_broadcast(&cond_);
    }
private:
    int waitSecond;
    pthread_cond_t cond_;
};
#endif