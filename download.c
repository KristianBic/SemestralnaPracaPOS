#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <pthread.h>

#include "download.h"




CLIENT_INFO download(URL_SLICED *slicedURL, int socked, int id) {
    CLIENT_INFO clientA;
    clientA.slicedURL = slicedURL;
    clientA.sockfd = socked;
    clientA.localFile = DEFAULT_HTTP_LOCALFILE;
    clientA.username = "";
    clientA.password = "";
    clientA.fileSize = 0;
    clientA.downloadedSize = 0;
    clientA.priority = 0;
    clientA.pause = false;
    clientA.id = id;
    printf("Data na stahovanie boli inicializovane\n");
    return clientA;
}

void startDownload(CLIENT_INFO* client) {
    client->downloading = true;
    if (client->pause) {
        resumeDownload(client);
        return;
    }
    if (strcmp(client->slicedURL->protocol, "http") == 0) {
        downloadHTTP(client);
    } else if (strcmp(client->slicedURL->protocol, "https") == 0) {
        downloadHTTP(client);
        //https();
    } else if (strcmp(client->slicedURL->protocol, "ftp") == 0) {
        downloadHTTP(client);
        //ftp();
    } else if (strcmp(client->slicedURL->protocol, "ftps") == 0) {
        //ftps();
    }
}

void pauseDownload(CLIENT_INFO* client) {
    printf("Stahovanie sa pozastavilo %s\n", client->slicedURL->domain);

    client->downloading = false;
    client->pause = true;
    client->resume = false;
    printf("Stahovanie sa pozastavilo\n");
}

void stopDownload(CLIENT_INFO client) {
    client.downloading = false;
    client.stop = true;
    client.pause = true;
    printf("Stahovanie sa zruselo\n");
    //pridat tu nieco
}

void resumeDownload(CLIENT_INFO* client) {
    client->downloading = true;
    client->resume = true;
    client->pause = false;
    printf("Stahovanie sa znovu spustilo\n");
    if (client->stop) {
        return;
    }
    if (strcmp(client->slicedURL->protocol, "http") == 0) {
        http_download_file(client);
    } else if (strcmp(client->slicedURL->protocol, "https") == 0) {
        http_download_file(client);
        //https();
    } else if (strcmp(client->slicedURL->protocol, "ftp") == 0) {
        http_download_file(client);
        //ftp();
    } else if (strcmp(client->slicedURL->protocol, "ftps") == 0) {
        //ftps();
    }
}

void downloadHTTP(CLIENT_INFO* client) {
    printf("Starting sownload %s\n", client->slicedURL->domain);

    int fd;
    if((fd = open(client->localFile, O_WRONLY | O_CREAT, 0666)) == -1) { //open file
        printf("Error. Subor sa neda otvorit\n");
        return;
    }
    //nasledujuci kod nefunguje ... neviem preco
    /*
    if(access(client.localFile, F_OK) == 0) { //if file exists
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
    */
    printf("Stahovanie suboru %s...\n", client->localFile);

    char recv_data[BUFFER_SIZE];
    char send_data[BUFFER_SIZE];

    sprintf(send_data, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", client->slicedURL->domainPath, client->slicedURL->domain);
    if (send(client->sockfd, send_data, strlen(send_data), 0) < 0)
    {
        perror("Error sending HTTP request - Send data");
        exit(1);
    }
    printf("Data send...\n");

    //recv(sockfd,recv_data,sizeof(recv_data),0);
    if (recv(client->sockfd, recv_data, BUFFER_SIZE - 1, 0) < 0)
    {
        perror("Error reading HTTP response - send data");
        exit(2);
    }
    recv_data[BUFFER_SIZE - 1] = '\0';
    printf("Received HTTP response\n");

    // Extract content length from HTTP response
    char *length = strtok(recv_data, "\r\n");
    client->fileSize = contentLength(length);
    printf("Content length of file is: %d\n", client->fileSize);

    if (client->fileSize < 0)
    {
        fprintf(stderr, "Error: content length not found in HTTP response\n");
        exit(3);
    }

    http_download_file(client);

    close(fd);
}

void* http_download_file(CLIENT_INFO *client)
{
    printf("Zacina sa stahovanie\n");

    // Open local file for writing
    FILE *fp = fopen(client->localFile, "w");
    if (fp == NULL) {
        perror("Error opening local file for writing");
        pthread_exit(NULL);
    }

    // Read file data from HTTP response
    char buffer[BUFFER_SIZE];
    int bytes_read;
    struct timeval start, end;
    gettimeofday(&start, NULL);
    while ((bytes_read = recv(client->sockfd, buffer, BUFFER_SIZE, 0)) > 0) {
        if(client->pause) {
            sleep(1);
        } else {
        fwrite(buffer, 1, bytes_read, fp);
        client->downloadedSize += bytes_read;

        // Update download status
        gettimeofday(&end, NULL);
        double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
        double speed = client->downloadedSize / elapsed;
        double percentage = (double) client->downloadedSize / client->fileSize;

        // Display progress bar
        int bar_length = 50;
        int progress = (int)(percentage * bar_length);
        char time_str[20];
        time_t t = time(NULL);
        strftime(time_str, 20, "%d.%m.%Y %H:%M:%S", localtime(&t));
        //printf("\r%s Downloading... %.2f/%.2f M bytes (%.2f%%) received (%.2f MB/s) [", time_str, (double)bytes_received/(1024*1024), (double)content_length/(1024*1024), percentage * 100, speed / 1024.0 / 1024.0);
        for (int i = 0; i < bar_length; i++) {
            if (i <= progress) {
                //printf("#");
            } else {
                //printf(" ");
            }
        }
        //printf("] (Time elapsed: %.2f seconds)",elapsed);
        fflush(stdout);
        }
    }
    write_to_log(client->localFile, client->slicedURL->domain, client->downloadedSize, getSizeUnit(client->downloadedSize), 0);
    // Close local file
    fclose(fp);
    // Close socket
    close(client->sockfd);
    printf("\nFile download complete: %s\n", client->localFile);
    printf("File download pthread closed\n");
    pthread_exit(NULL);
}
