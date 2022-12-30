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
#include <pthread.h>



const char *localFile;

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

void http_download_file(void *arg)
{
    DownloadArgs *args = (DownloadArgs *)arg;
    int sock = args->sock;
    int content_length = args->content_length;
    printf("args Content length of file in download: %d\n", content_length);
    int bytes_received = 0;
    struct timeval start, end;
    gettimeofday(&start, NULL);

    // Open local file for writing
    FILE *fp = fopen(localFile, "w");
    if (fp == NULL) {
        perror("Error opening local file for writing");
        pthread_exit(NULL);
    }

    // Read file data from HTTP response
    char buffer[BUFFER_SIZE];
    int bytes_read;
    while ((bytes_read = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, 1, bytes_read, fp);
        bytes_received += bytes_read;

        // Update download status
        gettimeofday(&end, NULL);
        double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
        double speed = bytes_received / elapsed;
        double percentage = (double) bytes_received / content_length * 100;
        // Display progress bar
        int bar_length = 15;
        int progress = (int)((percentage/100) * bar_length);
        printf("\rDownloading... %d/%d bytes (%.2f%%) received (%.2f MB/s) [", bytes_received, content_length, percentage, speed / 1024.0 / 1024.0);
        for (int i = 0; i < bar_length; i++) {
            if (i < progress) {
                printf("#");
            } else {
                printf(" ");
            }
        }
        printf("]");
        fflush(stdout);
    }

    // Close local file
    fclose(fp);
    // Close socket
    close(sock);
    printf("\nFile download complete: %s\n", localFile);
    pthread_exit(NULL);
}

void downloadHTTP(int sockfd, URL_SLICED* slicedURL) {
    int fd;

    if(access(localFile, F_OK) == 0) { //if file exists
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

    if((fd = open(localFile, O_WRONLY | O_CREAT, 0666)) == -1) { //open file
        printf("Error. Subor sa neda otvorit\n");
        return;
    }
    printf("Stahovanie suboru %s...\n", localFile);

    char recv_data[BUFFER_SIZE];
    char send_data[BUFFER_SIZE];

    sprintf(send_data, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", slicedURL->domainPath, slicedURL->domain);
    if (send(sockfd, send_data, strlen(send_data), 0) < 0)
    {
        perror("Error sending HTTP request - Send data");
        exit(1);
    }
    printf("Data send...\n");

    //recv(sockfd,recv_data,sizeof(recv_data),0);
    if (recv(sockfd, recv_data, BUFFER_SIZE - 1, 0) < 0)
    {
        perror("Error reading HTTP response - send data");
        exit(2);
    }
    recv_data[BUFFER_SIZE - 1] = '\0';
    printf("Received HTTP response\n");

    // Extract content length from HTTP response
    char *length = strtok(recv_data, "\r\n");
    int content_length = contentLength(length);
    printf("Content length of file is: %d\n", content_length);

    if (content_length < 0)
    {
        fprintf(stderr, "Error: content length not found in HTTP response\n");
        exit(3);
    }

    DownloadArgs args;
    args.sock = sockfd;
    args.content_length = content_length;

    pthread_t download_thread;
    if (pthread_create(&download_thread, NULL, http_download_file, (void *)&args) != 0)
    {
        perror("Error creating pthread");
        exit(4);
    }

    // Wait for pthread to finish
    if (pthread_join(download_thread, NULL) != 0)
    {
        perror("Error waiting for pthread");
        exit(5);
    }

    close(fd);
}

int main(int argc, char *argv[])
{
    URL_SLICED slicedURL;
    CLIENT_INFO clientInfo; // prerobit aby to bolo iba v klientovi a nie v URL_SLICERI ...
    int sockfd;

    if(argc < 2) {
        fprintf(stderr, "Nedostatocny pocet argumentov! Program zada nasledujuce udaje: \n");
        split_url(&slicedURL, DEFAULT_HTTP_URL);
        clientInfo.url = &slicedURL;
        localFile = DEFAULT_HTTP_LOCALFILE;
        printf("Protocol: %s\nSite: %s\nPort: %s\nPath: %s\n", slicedURL.protocol, slicedURL.domain, slicedURL.port, slicedURL.domainPath);
    } else {
        //domain = argv[1];
    }

    if (atoi(slicedURL.port) > 65536 || atoi(slicedURL.port) < 0) {
        printf("Invalid Port Number!");
        return -1;
    }
    printf("\n");

    sockfd = serverConnection(&slicedURL);
    downloadHTTP(sockfd, &slicedURL);

    return 0;
}