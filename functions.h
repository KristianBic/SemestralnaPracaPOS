
#ifndef SEMESTRALNAPRACAPOS_FUNCTIONS_H
#define SEMESTRALNAPRACAPOS_FUNCTIONS_H
typedef struct
{
    const char* domain;
    const char* domainPath;
    const char* protocol;
    const char* port;
} URL_SLICED;

URL_SLICED* split_url(URL_SLICED* info, const char* url);
int contentLength(char *length);

#endif //SEMESTRALNAPRACAPOS_FUNCTIONS_H
