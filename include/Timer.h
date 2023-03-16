#ifndef __TIMER__
#define __TIMER__

#include <sys/time.h>
#include <unordered_map>
#include <list>
#include <memory>
#include "AsyncLoger.h"
#include "Mutex.h" // 互斥锁类
#include "utils.h" // epoll的操作函数

class Timer{
public:
    // _timeout表示长连接的超时时间; _maxSize表示允许的长链接的最大数量上限，默认1000条
    Timer(int _epollfd, int _timeout , int _maxSize = 1000 ) :epollFd( _epollfd ), timeout( _timeout ), maxSize( _maxSize ), curSize( 0 ) { AsyncLoger::getInstance( loger ); }
    // 添加需要长链接的客户端，添加成功返回true，失败返回false
    bool addfd( int clientFd );
    // 检查处理过期的客户端连接
    void deleteExpiredFd();
    // 删除指定的文件描述符
    void deleteFd( int clientFd );
    // 屏蔽某个文件描述符使得其不被进行超时检查
    void forbidenFd( int clientFd );

private:
    // 用于保存每一个需要长链接的套接字的文件描述符，加入时间以及过期的时间
    class Node{
    public:
        using TimeVal = struct timeval;
        // _timeout表示长连接的超时时间，默认10s
        Node( int _clientfd, int _timeout );

        const TimeVal& curTime() const { return m_curTime; }
        const TimeVal& expireTime() const { return m_expireTime; }
        const int clientFd() const { return m_clientfd; }
        void resetExpireTime( int _timeout );
        void setExpireTime( int _timeout );
    private:
        int m_clientfd;
        TimeVal m_curTime;
        TimeVal m_expireTime;
    };

    // 保存节点的链表，快失效的在链表头，新的在链表尾部
    std::list< Node > timeList;
    // 保存客户端套接字文件描述符到时间节点的映射关系
    std::unordered_map< int, std::list< Node >::iterator > hashMap;
    // 添加数据时的写锁
    MutexLock mutex;
    // 节点超时时间
    int timeout;
    // 当前在时间链表中的元素
    int curSize;
    // 最大允许存在的元素数量
    int maxSize;
    // Epoll的文件描述符
    int epollFd;
    // 日志记录
    std::shared_ptr<AsyncLoger> loger;

};

#endif // __TIMER__