#ifndef __UTILS__
#define __UTILS__

#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string>

extern char *memory_index_page;

void bufferPrinter(char* buffer, const int bufferSize);
void parseLineFromSocket( char* buffer, const int buffSisze);
void setPortReuse( int socketfd );
void sendHello( int sockedfd );
void dispAddrInfo( struct sockaddr_in &addr);

// 输入文件路径，判断文件是否存在
bool fileExist(const char [] );

// 输入文件路径，判断文件是否存在
void loadIndexTomemory( std::string );

#endif
