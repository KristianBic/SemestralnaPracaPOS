#ifndef SEMESTRALNAPRACAPOS_DOWNLOAD_H
#define SEMESTRALNAPRACAPOS_DOWNLOAD_H
#include <stdbool.h>
#include "functions.h"
#include "config.h"

typedef struct mtx{
    pthread_mutex_t* mut;
    pthread_cond_t * stop;
    pthread_cond_t * start;
    bool logging;
} MUTEX;

typedef struct clientInformations{
    MUTEX* mutex;
    URL_SLICED* slicedURL;
    int sockfd;
    char* localFile;
    char* username;
    char* password;
    int fileSize;
    int downloadedSize;
    int priority;
    int id;
    int planningTime;
    bool pause;
    bool resume;
    bool stop;
    bool downloading;
} CLIENT_INFO;

CLIENT_INFO download(URL_SLICED *slicedURL, int socked, int id, MUTEX* mut);
void startDownload(CLIENT_INFO* client);
void stopDownload(CLIENT_INFO client);
void* http_download_file(CLIENT_INFO* client);
void downloadHTTP(CLIENT_INFO* client);
void pauseDownload(CLIENT_INFO* client);
void resumeDownload(CLIENT_INFO* client);

#endif //SEMESTRALNAPRACAPOS_DOWNLOAD_H
