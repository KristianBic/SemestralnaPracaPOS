#ifndef SEMESTRALNAPRACAPOS_DOWNLOAD_H
#define SEMESTRALNAPRACAPOS_DOWNLOAD_H
#include <stdbool.h>
#include "functions.h"
#include "config.h"

typedef struct clientInformations{
    URL_SLICED* slicedURL;
    int sockfd;
    char* localFile;
    char* username;
    char* password;
    int fileSize;
    int downloadedSize;
    int priority;
    bool pause;
    bool resume;
    bool stop;
    bool downloading;
} CLIENT_INFO;


void download(URL_SLICED* slicedURL, int socked);
void startDownload();
void downloadHTMLfromHTTP(int sockfd);
void http_download_file(void *arg);
void downloadHTTP();
void pauseDownload();
void resumeDownload();

#endif //SEMESTRALNAPRACAPOS_DOWNLOAD_H
