#include "ThreadPool.h"


void httpserver( int ); // 为什么一定要void*才能调用、pthread create

ThreadPool::ThreadPool(void (*task)(TaskData), int threadNum, int maxWorkListLen) : task(task), threadNum( threadNum ), maxWorkListLen( maxWorkListLen ),
    threads( threadNum ), running( true ) {
        for ( int i = 0; i < threadNum; ++i){
            pthread_create( &threads[ i ] , nullptr , workerWarper, this);
        }
    }

ThreadPool::~ThreadPool(){
    running = false;
    cond.notifyAll();
    for( pthread_t threadId : threads ){
        pthread_join( threadId, nullptr );
    }
}

void* ThreadPool::workerWarper( void* args ){
    ThreadPool* threadPool = reinterpret_cast< ThreadPool* > ( args );
    threadPool->worker();
    return nullptr;
}

void ThreadPool::worker(){
    while( running ){
        TaskData taskData( -1, nullptr );
        {
            MutexLockGuard mutexLockGuard( mutex );
            while( workList.empty() && running ){
                cond.wait( mutex );
            }
            if( !running ) return;
            taskData = workList.front();
            workList.pop_front();
        }
        task( taskData ); // 执行设定的用户task函数的回调
    }
    return;
}