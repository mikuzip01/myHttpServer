#include "ThreadPool.h"


void httpserver( int ); // 为什么一定要void*才能调用、pthread create

ThreadPool::ThreadPool(void (*task)(TaskData), int threadNum, int maxWorkListLen) : task(task), threadNum( threadNum ), maxWorkListLen( maxWorkListLen ),
    threads( threadNum ){
        for ( int i = 0; i < threadNum; ++i){
            pthread_create( &threads[ i ] , nullptr , worker, this); // fixme为什么不接受成员函数？？????????？？？？?????
        }
    }

void* ThreadPool::worker( void* args ){
    ThreadPool* threadPool = reinterpret_cast< ThreadPool* > ( args );
    while( true ){

        TaskData taskData( -1, nullptr );
        {
            MutexLockGuard mutexLockGuard( threadPool->mutex );
            while( threadPool->workList.empty() ){
                threadPool->cond.wait( threadPool->mutex );
            }

            taskData = threadPool->workList.front();
            threadPool->workList.pop_front();
        }
        threadPool->task( taskData ); // 执行设定的用户task函数的回调
    }
}