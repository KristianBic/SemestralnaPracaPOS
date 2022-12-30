#include "Client.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    URL_SLICED urlSliced;

    if(argc < 2) {
        fprintf(stderr, "Nedostatocny pocet argumentov! \n");
        split_url(&urlSliced, DEFAULT_HTTP_URL);
        printf("Protocol: %s\nSite: %s\nPort: %s\nPath: %s\n",
               urlSliced.protocol, urlSliced.domain, urlSliced.port, urlSliced.domainPath);
    }
    else {
        split_url(&urlSliced, argv[1]);
        printf("Protocol: %s\nSite: %s\nPort: %s\nPath: %s\n",
               urlSliced.protocol, urlSliced.domain, urlSliced.port, urlSliced.domainPath);

    }

    if (atoi(urlSliced.port) > 65536 || atoi(urlSliced.port) < 0) {
        printf("Invalid Port Number!");
        return -1;
    }
    printf("\n");

    int sock = serverConnection(&urlSliced);
    download(&urlSliced, sock);

    return 0;
}