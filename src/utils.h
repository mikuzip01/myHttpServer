#ifndef __UTILS__
#define __UTILS__

#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>



void bufferPrinter(char* buffer, const int bufferSize);
void parseLineFromSocket( char* buffer, const int buffSisze);
void setPortReuse( int socketfd );
void sendHello( int sockedfd );
void dispAddrInfo( struct sockaddr_in &addr);

#endif
