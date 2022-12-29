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
#include <netdb.h>
#include <ctype.h>
#include <fcntl.h>


/* The DEFAULT IP address and port number to connect to */
#define DEFAULT_IPADDR "example.com"
#define DEFAULT_PORTNUM 80

#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif

char *address; //will be displayed after every command
int port;
int sockfd;

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

static int closecb(void *clientp, int item)
{
    (void)clientp;
    printf("libcurl wants to close %d now\n", (int)item);
    return 0;
}

static int opensocket(void *clientp,
                                curlsocktype purpose,
                                struct curl_sockaddr *address)
{
    int sockfd;
    (void)purpose;
    (void)address;
    sockfd = *(int *)clientp;
    /* the actual externally set socket is passed in via the OPENSOCKETDATA
       option */
    return sockfd;
}

static int sockopt_callback(void *clientp, int curlfd,
                            curlsocktype purpose)
{
    (void)clientp;
    (void)curlfd;
    (void)purpose;
    /* This return code was added in libcurl 7.21.5 */
    return CURL_SOCKOPT_ALREADY_CONNECTED;
}

void download(int sockfd) {
    CURL *curl;
    CURLcode res;
    struct FtpFile ftpfile = {
            "prvy.txt", /* name to store the file as if successful */
            NULL
    };

    curl = curl_easy_init();
    if(curl) {
        //curl_easy_setopt(curl, CURLOPT_URL, "http://99.99.99.99:9999");
        curl_easy_setopt(curl, CURLOPT_URL, "http://212.183.159.230/5MB.zip");
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
void downloadHTMLfromHTTP(int sockfd) {
    char buf[2056];
    int byte_count;

    char *header = "GET /index.html HTTP/1.1\r\nHost: www.example.com\r\n\r\n";
    send(sockfd,header,strlen(header),0);
    printf("GET Sent...\n");
    //all right ! now that we're connected, we can receive some data!
    byte_count = recv(sockfd,buf,sizeof(buf),0);
    printf("recv()'d %d bytes of data in buf\n",byte_count);
    printf("%.*s",byte_count,buf); // <-- give printf() the actual data size
}

void downloadHTTP(int sockfd) {
    char *fileName = "stvrty.txt";
    struct FtpFile ftpfile = { "prvy.txt", NULL};
    int fd;

    if(access(fileName, F_OK) == 0) { //if file exists
        printf("Subor s rovnakym nazvom uz existuje. Chcete ho prepisat? (a/n): ");
        char answer[256];
        fgets(answer, sizeof(answer), stdin);
        answer[0] = tolower(answer[0]);

        if(answer[0] == 'a') {
            printf("Prepisujem subor. Pokracujeme v stahovani.\n");
        } else if(answer[0] == 'n') {
            printf("Canceling download\n");
            return;
        } else {
            printf("Error: Zly vstup. Rusi sa stahovanie.\n");
            return;
        }
    } else {
        printf("Pokracujeme v stahovani.\n");
    }

    if((fd = open(fileName, O_WRONLY | O_CREAT, 0666)) == -1) { //open file
        printf("Error. Subor sa neda otvorit\n");
        return;
    }
    printf("Stahovanie suboru %s...\n", fileName);

    char recv_data[2056];
    char* path = "index.html";
    char* domain = "www.example.com";
    char send_data[2056];
    snprintf(send_data, sizeof(send_data), "GET /%s HTTP/1.1\\r\\nHost: %s\\r\\n\\r\\n\"", path, domain);

    send(sockfd,send_data,strlen(send_data),0);
    printf("Send data...\n");
    recv(sockfd,recv_data,sizeof(recv_data),0);

    if(write(fd, recv_data, strlen(recv_data)) == -1) {
        perror("Error writing to file");
        close(fd);
        return;
    }
    printf("Stahovanie uspesne\n");

    close(fd);
}

int serverConnection() {
    struct sockaddr_in server;
    struct hostent *he;
    struct in_addr **addr_list;
    char *ipaddr;
    int sockfd;

    if((he = gethostbyname(address)) == NULL) {
        herror("Error resolving hostname");
        exit(-1);
    } //end if

    addr_list = (struct in_addr **) he->h_addr_list;

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr = *addr_list[0];

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) { //create socket
        perror("Socket failed");
        close(sockfd);
        exit(-1);
    } //end if

    ipaddr = inet_ntoa(*addr_list[0]);
    printf("Connecting to %s [%s]...\n\n", address, ipaddr);
    if(connect(sockfd, (struct sockaddr *)&server,
               sizeof(struct sockaddr_in)) == -1) { //create connection
        perror("Connection failed");
        close(sockfd);
        exit(-1);
    } //end if

    printf("Successful Connection. \n");
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
    if (port > 65536 || port < 0) {
        printf("Invalid Port Number!");
        exit(5);
    }
    printf("\n");

    sockfd = serverConnection();
    downloadHTTP(sockfd);
    //downloadHTMLfromHTTP(sockfd);
    //download(sockfd);

    return 0;
}