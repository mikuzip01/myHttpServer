#include <stdio.h>
#include <sys/socket.h>
#include <string.h>

void bufferPrinter(char* buffer, const int bufferSize);
void parseLineFromSocket( char* buffer, const int buffSisze);
void setPortReuse( int socketfd );
void sendHello( int sockedfd );
