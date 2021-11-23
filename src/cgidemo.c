#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(){
    printf("Content-type: text/html\r\n");
    char *envs = 0;
    envs = getenv("CGIDATA");
    if( envs != 0 ){
        
        char key[256], value[256];
        int datacount = 0;
        int serverCgiLength = 0;
        sscanf( envs, "%d %d", &datacount, &serverCgiLength );
        printf("Content-Length: %d\r\n", serverCgiLength + 18 + datacount * 9 );
        printf("\r\n");
        printf("<h1>CGI ECHO</h1>\n");
        while( datacount != 0 ){
            scanf("%s", key);
            scanf("%s", value);
            printf("<p>%s:%s</p>\n", key, value);
            --datacount;
        }
    }
    else{
        printf("Content-Length: 22\r\n");
        printf("\r\n");
        printf("<h1>Input Error!</h1>\n");
    }
    

    return 0;
}