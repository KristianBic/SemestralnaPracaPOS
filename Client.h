#ifndef SEMESTRALNAPRACAPOS_CLIENT_H
#define SEMESTRALNAPRACAPOS_CLIENT_H

#include <pthread.h>

#include "functions.h"
#include "config.h"
#include "conn.h"
#include "download.h"


typedef struct
{
    CLIENT_INFO vlakna[MAX_QUEUE_SIZE];
    int pocetPrvkov;
} ZOZNAM_VLAKIEN;

void printInformations(ZOZNAM_VLAKIEN *ptr);
void* downloadThread(void* zoznamVlakien);
void initialInput();
URL_SLICED downloadInput();
int imputPlanningTime();
#endif //SEMESTRALNAPRACAPOS_CLIENT_H
