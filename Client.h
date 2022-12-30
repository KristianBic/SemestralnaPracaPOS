//
// Created by Kristian on 22. 12. 2022.
//

#ifndef SEMESTRALNAPRACAPOS_CLIENT_H
#define SEMESTRALNAPRACAPOS_CLIENT_H

#include "functions.h"
#include "config.h"
#include "conn.h"

typedef struct
{
    URL_SLICED* url;

} CLIENT_INFO;

typedef struct {
    int sock;
    int content_length;
} DownloadArgs;


#endif //SEMESTRALNAPRACAPOS_CLIENT_H
