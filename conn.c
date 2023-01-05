#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#include "conn.h"

/* serverConnection() je funkcia, ktorá sa používa na vytvorenie sieťovej konektivity s vzdialeným serverom.
Funkcia prijíma ako parameter ukazovateľ na štruktúru URL_SLICED, ktorá obsahuje informácie o protokole, domene, porte a ceste k súboru na vzdialenom servere.

Funkcia vytvorí socket pomocou funkcie socket() a následne sa pokúsi vytvoriť konektivitu s vzdialeným serverom pomocou funkcie connect().
Ak sa podarí vytvoriť konektivitu, funkcia vráti soketový deskriptor pre túto konektivitu.
Ak sa konektivita nepodarí vytvoriť, funkcia vypíše chybovú hlášku a ukončí program.*/
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
