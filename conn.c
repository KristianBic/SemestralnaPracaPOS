#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#include "conn.h"


int serverConnection(URL_SLICED* slicedURL) {
    struct sockaddr_in server;
    struct hostent *he;
    struct in_addr **addr_list;
    char *ipaddr;
    int sockfd;

    if((he = gethostbyname(slicedURL->domain)) == NULL) {
        herror("Error resolving hostname");
        exit(-1);
    }

    addr_list = (struct in_addr **) he->h_addr_list;

    server.sin_family = AF_INET;
    if (strcmp(slicedURL->protocol, "ftp") == 0) {
        server.sin_port = htons(21);
    } else {
        server.sin_port = htons(atoi(slicedURL->port));
    }
    server.sin_addr = *addr_list[0];

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) { //create socket
        perror("Socket failed");
        close(sockfd);
        exit(-1);
    }

    ipaddr = inet_ntoa(*addr_list[0]);
    printf("Connecting to %s [%s]...\n\n", slicedURL->domain, ipaddr);
    if(connect(sockfd, (struct sockaddr *)&server,sizeof(struct sockaddr_in)) == -1) { //create connection
        perror("Connection failed");
        close(sockfd);
        exit(-1);
    }

    printf("Successful Connection. \n");
    return sockfd;
}