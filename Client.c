#include "Client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int main()
{

    ZOZNAM_VLAKIEN zoznamVlakien;
    zoznamVlakien.pocetPrvkov = 0;
    char input[100];
    pthread_t threadDownload;

    printf( "------------------------------------------------------------------------------------------- \n");
    while (1) {
        printf("Zadajte jeden z nasledujucich prikazov: download ; history ; exit \n");
        printf("\n->  ");
        gets(input);
        if(strcmp(input, "download") == 0) {

            URL_SLICED urlSliced;
            urlSliced = downloadInput();
            int sock = serverConnection(&urlSliced);
            zoznamVlakien.vlakna[zoznamVlakien.pocetPrvkov] = download(&urlSliced, sock, zoznamVlakien.pocetPrvkov);
            zoznamVlakien.vlakna[zoznamVlakien.pocetPrvkov].planningTime = imputPlanningTime();
            zoznamVlakien.pocetPrvkov++;

            pthread_create(&threadDownload, NULL, downloadThread, &zoznamVlakien);
            pthread_detach(threadDownload);

        }else if (strcmp(input, "information") == 0) {
            printInformations(&zoznamVlakien);
        }else if (strcmp(input, "priority") == 0) {
            printInformations(&zoznamVlakien);
            printf( "Zadajte id procesu, ktoremu chcete nastavit prioritu\n");
            printf("\n->  ");
            char proces[2];
            gets(proces);
            char *command = strtok(proces, " ");

            for (int i = 0; i < zoznamVlakien.pocetPrvkov; ++i) {
                char str[2];
                sprintf(str, "%d", i);
                zoznamVlakien.vlakna[i].priority = 0; //vynulovat ostatne
                if(strcmp(command, str) == 0) {
                    zoznamVlakien.vlakna[i].priority = 1; //nastavenie noveho
                }
            }
        } else if (strcmp(input, "historia") == 0) {
            printf( "Zadajte co chcete robit - show , clear\n");
            printf("\n->  ");

            char input[100];
            gets(input);
            if (strcmp(input, "show") == 0) {
                printLog();
            } else if (strcmp(input, "clear") == 0) {
                clear_log();
            } else {
                printf( "Nespravny imput\n");
            }

        } else if (strcmp(input, "pause") == 0) {
            printInformations(&zoznamVlakien);
            printf( "Zadajte id procesu\n");
            printf("\n->  ");
            char proces[2];
            gets(proces);
            char *command = strtok(proces, " ");

            for (int i = 0; i < zoznamVlakien.pocetPrvkov; ++i) {
                char str[2];
                sprintf(str, "%d", i);
                if(strcmp(command, str) == 0) {
                    pauseDownload(&zoznamVlakien.vlakna[i]);
                }
            }
        }else if (strcmp(input, "resume") == 0) {
            printInformations(&zoznamVlakien);
            printf( "Zadajte id procesu\n");
            printf("\n->  ");
            char proces[2];
            gets(proces);
            char *command = strtok(proces, " ");

            for (int i = 0; i < zoznamVlakien.pocetPrvkov; ++i) {
                char str[2];
                sprintf(str, "%d", i);
                if(strcmp(command, str) == 0) {
                    resumeDownload(&zoznamVlakien.vlakna[i]);
                }
            }

        }else if (strcmp(input, "stop") == 0) {
            printInformations(&zoznamVlakien);
            printf( "Zadajte id procesu\n");
            printf("\n->  ");
            char proces[2];
            gets(proces);
            char *command = strtok(proces, " ");

            for (int i = 0; i < zoznamVlakien.pocetPrvkov; ++i) {
                char str[2];
                sprintf(str, "%d", i);
                if(strcmp(command, str) == 0) {
                    stopDownload(zoznamVlakien.vlakna[i]);
                }
            }
        } else if (strcmp(input, "pauseALL") == 0) {
            for (int i = 0; i < zoznamVlakien.pocetPrvkov; ++i) {
                pauseDownload(&zoznamVlakien.vlakna[i]);
            }
        }else if (strcmp(input, "resumeALL") == 0) {
            for (int i = 0; i < zoznamVlakien.pocetPrvkov; ++i) {
                resumeDownload(&zoznamVlakien.vlakna[i]);
            }
        }else if (strcmp(input, "stopALL") == 0) {
            for (int i = 0; i < zoznamVlakien.pocetPrvkov; ++i) {
                stopDownload(zoznamVlakien.vlakna[i]);
            }
        }else if (strcmp(input, "exit") == 0) {
            break;
        } else {
            printf( "Zadali ste nespravny tvar. Skuste znovu. \n");
        }
        printf("\n");
    }
    printf( "------------------------------------------------------------------------------------------- \n");


    for (int i = 0; i < zoznamVlakien.pocetPrvkov; ++i) {
        free((void *) zoznamVlakien.vlakna[i].slicedURL->domain);
        free((void *) zoznamVlakien.vlakna[i].slicedURL->protocol);
    }

    return 0;
}

void printInformations(ZOZNAM_VLAKIEN *ptr) {
    for (int i = 0; i < ptr->pocetPrvkov; ++i) {
        printf("Download s id %d ma: Domain %s a prioritu %d\n",  ptr->vlakna[i].id, ptr->vlakna[i].slicedURL->domain, ptr->vlakna[i].priority);
    }
}


void* downloadThread(void* vlaknaPar) {
    ZOZNAM_VLAKIEN* zoznamVlakien = vlaknaPar;
    sleep(zoznamVlakien->vlakna[zoznamVlakien->pocetPrvkov - 1].planningTime);
    startDownload(&zoznamVlakien->vlakna[zoznamVlakien->pocetPrvkov - 1]);
}


URL_SLICED downloadInput() {
    URL_SLICED urlSliced;
    char urlConsole[100];
    char localFileConsole[100];

    printf( "------------------------------------------------------------------------------------------- \n");
    printf("Zadajte link(URL) pre stiahnutie suboru. Napr. http://xcal1.vodafone.co.uk/5MB.zip\n");
    printf("\n->  ");
    gets(urlConsole);
    if(strcmp(urlConsole, "") == 0) {
        split_url(&urlSliced, DEFAULT_HTTP_URL);
        printf( "Zadana adresa je: %s \n", DEFAULT_HTTP_URL);
    } else {
        split_url(&urlSliced, urlConsole);
        printf( "Zadana adresa je: %s \n", urlConsole);
    }
    if (atoi(urlSliced.port) > 65536 || atoi(urlSliced.port) < 0) {
        printf("Invalid Port Number!");
        exit(1);
    }
    printf( "------------------------------------------------------------------------------------------- \n");


    printf("Zadajte nazov noveho suboru. Pri nezadani nazvu sa nazov zachova totozny ako na serveri.\n");
    printf("\n->  ");
    gets(localFileConsole);
    if (strcmp(localFileConsole, "") != 0) {
        urlSliced.fileName = localFileConsole;
    }
    printf( "Zadany nazov lokalneho suboru je: %s \n", urlSliced.fileName);
    printf( "------------------------------------------------------------------------------------------- \n");


    printf("Protocol: %s\nSite: %s\nPort: %s\nPath: %s\nFileName: %s\n\n",
           urlSliced.protocol, urlSliced.domain, urlSliced.port, urlSliced.domainPath, urlSliced.fileName);
    printf( "------------------------------------------------------------------------------------------- \n");

    return urlSliced;
}

int imputPlanningTime() {
    char cas[100];
    char hodina[100];
    char minuta [100];
    char sekunda[100];
    printf( "Chcete naplánovať čas, kedy sa má sťahovanie začať? (a/n) \n");
    gets(cas);
    if (strcmp(cas, "a") == 0) {
        printf( "Zadajte pocet hodin \n");
        gets(hodina);
        printf( "Zadajte pocet minut \n");
        gets(minuta);
        printf( "Zadajte pocet sekund \n");
        gets(sekunda);
    } else if (strcmp(cas, "n") != 0) {

    } else {
        printf( "Zadali ste nespravy vstup ... budeme pokracovat v stahovani \n");
    }
    printf( "Download sa zacne o %d:%d:%d \n", atoi(hodina), atoi(minuta), atoi(sekunda));

    return (atoi(sekunda) + (atoi(minuta) * 60) + (atoi(hodina) * 60 * 60));
}