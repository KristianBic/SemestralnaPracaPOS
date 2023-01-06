
#ifndef SEMESTRALNAPRACAPOS_FUNCTIONS_H
#define SEMESTRALNAPRACAPOS_FUNCTIONS_H

#include <stdbool.h>

typedef struct
{
    const char* fullUrl;
    const char* domain;
    const char* domainPath;
    const char* protocol;
    char* port;
    char* fileName;
    const char* localDomainPath;
    char* localFileName;

    char* username;
    char* password;
} URL_SLICED;

URL_SLICED* split_url(URL_SLICED* info, const char* url);
int contentLength(char *length);

char *getCurrentTime();
void write_to_log(char *filename, char *url, int size, char *sizeUnit, double elapsed_time,  pthread_cond_t * start,  pthread_mutex_t* mut);
char *getCurrentDirectory();
char *getSizeUnit(double size);
void printLog();
void clear_log();

int createDirectory(const char* path);
int directoryExists(const char* path);
void printDirectoryContents(const char *directory);
void printDirectory(const char *directory);
void deleteFile(const char *filename);
void deleteDirectory(const char *directory);
void get_unique_filename(char *filename);
#endif //SEMESTRALNAPRACAPOS_FUNCTIONS_H
