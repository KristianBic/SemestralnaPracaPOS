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
    int sockfd = connectServ(slicedURL);

    if (strcmp(slicedURL->protocol, "ftp") == 0 || strcmp(slicedURL->protocol, "ftps") == 0)
    {
        char response[1024];
        recv(sockfd, response, sizeof(response), 0);
        printf("%s\n", response);

        char request[1024];
        sprintf(request, "USER %s\n", slicedURL->username);
        send(sockfd, request, strlen(request), 0);
        recv(sockfd, request, sizeof(request), 0);
        printf("%s\n", request);

        sprintf(request, "PASS %s\n", slicedURL->password);
        send(sockfd, request, strlen(request), 0);
        recv(sockfd, request, sizeof(request), 0);
        printf("%s\n", request);

        sprintf(request, "SIZE %s\n", slicedURL->domainPath);
        send(sockfd, request, strlen(request), 0);
        recv(sockfd, request, sizeof(request), 0);
        printf("%s\n", request);

        sprintf(request, "TYPE I\n");
        send(sockfd, request, strlen(request), 0);
        recv(sockfd, request, sizeof(request), 0);
        printf("%s\n", request);

        sprintf(request, "PASV\n");
        send(sockfd, request, 5, 0);
        recv(sockfd, request, sizeof(request), 0);
        printf("%s\n", request);

        int port;
        char host[16];
        char newPort[16];
       if (parseHostAndPort(request, host, &port) == 0) {
           printf("Error: parsovanie url");
       }

        sprintf(newPort, "%d", port);
        slicedURL->port = strcpy((char *)malloc(strlen(newPort) + 1), newPort);
        printf("Data connection host: %s, port: %s\n", slicedURL->domain, slicedURL->port);

        sockfd = connectServ(slicedURL);
    }

    printf("Pripojenie prebehlo uspesne \n");
    printf("------------------------------------------------------------------------------------------- \n");
    return sockfd;
}

int connectServ(URL_SLICED* slicedURL) {
    struct sockaddr_in server;
    struct hostent *he;
    struct in_addr **addr_list;
    char *ipaddr;
    int sockfd;

    if((he = gethostbyname(slicedURL->domain)) == NULL) {
        herror("Error: spracovanie domeny");
        exit(-1);
    }

    addr_list = (struct in_addr **) he->h_addr_list;
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(slicedURL->port));
    server.sin_addr = *addr_list[0];

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) { //create socket
        perror("Socket failed");
        close(sockfd);
        exit(-1);
    }

    ipaddr = inet_ntoa(*addr_list[0]);
    printf("Pripaja sa na server %s [%s]...\n", slicedURL->domain, ipaddr);

    if(connect(sockfd, (struct sockaddr *)&server,sizeof(struct sockaddr_in)) == -1) { //create connection
        perror("Pripojenie zlyhalo");
        close(sockfd);
        exit(-1);
    }
    return sockfd;
}