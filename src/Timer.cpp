#include "Timer.h"

Timer::Node::Node(int _clientdf, int _timeout ) : 
    m_clientfd( _clientdf ){
    setExpireTime( _timeout );
}

void Timer::Node::resetExpireTime( int _timeout ){
    setExpireTime( _timeout );
}

void Timer::Node::setExpireTime( int _timeout ){
    gettimeofday( &m_curTime, nullptr );
    m_expireTime = m_curTime;
    m_expireTime.tv_sec += _timeout;
}


bool Timer::addfd( int clientFd ){
    {
        MutexLockGuard mutexLockGuard( mutex );
        if( curSize >= maxSize ) return false;
        if ( hashMap.find( clientFd ) == hashMap.end() ){ // 当前文件描述符不在队列中
            // 插入时间链表的末尾并将其位置记录到哈希表中
            hashMap[ clientFd ] = timeList.insert( timeList.end(), Node( clientFd, timeout ) );
            ++curSize;
            // 同时也要插入主线程的epoll中
            addFdToEpoll_INET( epollFd, clientFd );
            printf("\nTimer - add client to keep alive:");
            dispPeerConnection( clientFd );
        }
        else{ // 该文件描述符还没有过期且又再次发起了访问
            timeList.erase( hashMap[ clientFd ] );
            hashMap[ clientFd ] = timeList.insert( timeList.end(), Node( clientFd, timeout ) );
            printf("\nTimer - flash client keep alive:");
            dispPeerConnection( clientFd );
        }
        return true;
    }
}


void Timer::deleteExpiredFd(){ // 删除过期的链接
    {
        MutexLockGuard mutexLockGuard( mutex );
        Node::TimeVal curtime;
        gettimeofday( &curtime, nullptr );
        while( curSize != 0 && curtime.tv_sec > timeList.front().expireTime().tv_sec ){
            hashMap.erase( timeList.front().clientFd() );
            deleteFdFromEpoll( epollFd, timeList.front().clientFd() );
            printf("\nTimer - delete expire keep alive:");
            dispPeerConnection( timeList.front().clientFd() );
            close( timeList.front().clientFd() );
            timeList.pop_front();
            --curSize;
        }
    }
}

void Timer::deleteFd( int clientFd ){
    {
        MutexLockGuard mutexLockGuard( mutex );
        if( hashMap.find( clientFd ) == hashMap.end() ){ // clientfd本来就没有在计时器中
            // 什么都不做
        }
        else{
            printf("\nTimer - delete client keep alive:");
            dispPeerConnection( clientFd );
            timeList.erase( hashMap[ clientFd ] );
            hashMap.erase( clientFd );
            --curSize;
        }
    }
}