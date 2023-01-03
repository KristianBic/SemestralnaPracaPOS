
#ifndef SEMESTRALNAPRACAPOS_FUNCTIONS_H
#define SEMESTRALNAPRACAPOS_FUNCTIONS_H
typedef struct
{
    const char* domain;
    const char* domainPath;
    const char* protocol;
    const char* port;
    const char* fileName;
} URL_SLICED;

URL_SLICED* split_url(URL_SLICED* info, const char* url);
int contentLength(char *length);
void downloadHTMLfromHTTP(int sockfd);

char *getCurrentTime();
void write_to_log(char *filename, char *url, int size, char *sizeUnit, double elapsed_time);
char *getSizeUnit(double size);
void printLog();
void clear_log();

#endif //SEMESTRALNAPRACAPOS_FUNCTIONS_H
