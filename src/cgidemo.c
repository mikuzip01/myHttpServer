#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(){
    char key[256], value[256];
    int datacount = atoi( getenv("DATACOUNT") );
    printf("Content-type: text/html\r\n\r\n");
    printf("<h1>CGI ECHO</h1>");
    while( datacount != 0 ){
        scanf("%s", key);
        scanf("%s", value);
        printf("<p>%s:%s\n</p>", key, value);
        --datacount;
    }

    return 0;
}