#include "Client.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

/* The DEFAULT IP address and port number to connect to */
#define DEFAULT_IPADDR "212.183.159.230"
//#define DEFAULT_IPADDR "frios2.fri.uniza.sk"
#define DEFAULT_PORTNUM 80

#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif

char *address; //will be displayed after every command
int port;
curl_socket_t sockfd;

struct FtpFile {
    const char *filename;
    FILE *stream;
};

static size_t my_fwrite(void *buffer, size_t size, size_t nmemb, void *stream)
{
    struct FtpFile *out = (struct FtpFile *)stream;
    if (!out->stream)
    {
        /* open file for writing */
        out->stream = fopen(out->filename, "wb");
        if (!out->stream)
            return -1; /* failure, cannot open file to write */
    }
    return fwrite(buffer, size, nmemb, out->stream);
}

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}

static int closecb(void *clientp, curl_socket_t item)
{
    (void)clientp;
    printf("libcurl wants to close %d now\n", (int)item);
    return 0;
}

static curl_socket_t opensocket(void *clientp,
                                curlsocktype purpose,
                                struct curl_sockaddr *address)
{
    curl_socket_t sockfd;
    (void)purpose;
    (void)address;
    sockfd = *(curl_socket_t *)clientp;
    /* the actual externally set socket is passed in via the OPENSOCKETDATA
       option */
    return sockfd;
}

static int sockopt_callback(void *clientp, curl_socket_t curlfd,
                            curlsocktype purpose)
{
    (void)clientp;
    (void)curlfd;
    (void)purpose;
    /* This return code was added in libcurl 7.21.5 */
    return CURL_SOCKOPT_ALREADY_CONNECTED;
}

void download(curl_socket_t sockfd) {
    CURL *curl;
    CURLcode res;
    struct FtpFile ftpfile = {
            "prvy.c", /* name to store the file as if successful */
            NULL
    };

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://99.99.99.99:9999");
        //curl_easy_setopt(curl, CURLOPT_USERPWD, "meno:heslo");

        /* no progress meter please */
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

        //curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_fwrite);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ftpfile);

        /* call this function to get a socket */
        curl_easy_setopt(curl, CURLOPT_OPENSOCKETFUNCTION, opensocket);
        curl_easy_setopt(curl, CURLOPT_OPENSOCKETDATA, &sockfd);

        /* call this function to close sockets */
        curl_easy_setopt(curl, CURLOPT_CLOSESOCKETFUNCTION, closecb);
        curl_easy_setopt(curl, CURLOPT_CLOSESOCKETDATA, &sockfd);

        /* call this function to set options for the socket */
        curl_easy_setopt(curl, CURLOPT_SOCKOPTFUNCTION, sockopt_callback);

        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

        res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);

        close(sockfd);

        if(res) {
            printf("libcurl error: %d\n", res);
            exit(4);
        } else {
            printf("Stahovanie prebehlo uspesne...\n\n\n");
        }
    }
    if (ftpfile.stream) {
        fclose(ftpfile.stream); /* close the local file */
    }
    curl_global_cleanup();
}

int serverConnection() {
    curl_socket_t sockfd;
    struct sockaddr_in server;

    sockfd = socket(AF_INET, SOCK_STREAM, 0); //vytvorenie soketu
    if(sockfd == CURL_SOCKET_BAD) { //sockfd == -1
        perror("Error pri vytvarani socketu\n");
        close(sockfd);
        exit(3);
    }

    memset(&server, 0, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(address); // musi tam byt zrejme IP-cka

    if(INADDR_NONE == server.sin_addr.s_addr) {
        close(sockfd);
        exit(2);
    }

    printf("Pripajanie ku %s...\n\n", address);
    if(connect(sockfd, (struct sockaddr *) &server, sizeof(server)) == -1) {
        close(sockfd);
        printf("client error: connect: %s\n", strerror(errno));
        exit(1);
    }
    printf("Pripojenie prebehlo uspesne...\n\n\n");

    return sockfd;
}


int main(int argc, char *argv[])
{
    printf("-----------------------------------------\n");
    printf("***** Vytajte v Download Manazerovi *****\n");
    printf("-----------------------------------------\n");

    if(argc < 3) {
        fprintf(stderr, "Nedostatocny pocet argumentov! Program zada nasledujuce udaje: \n");
        printf("Hostname: %s\n", DEFAULT_IPADDR);
        printf("PortNumber: %d\n", DEFAULT_PORTNUM);
        address = DEFAULT_IPADDR;
        port = DEFAULT_PORTNUM;
    } else {
        address = argv[1];
        port = atoi(argv[2]);
    }
    printf("\n");

    sockfd = serverConnection();
    download(sockfd);

    return 0;
}