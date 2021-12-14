#ifndef __ERROR__
#define __ERROR__

#include <exception>

class SocketClosed : public std::exception {
public:
    SocketClosed() {}
    virtual const char* what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW{
        return "socket closed";
    }
    ~SocketClosed() throw() {}
};

#endif