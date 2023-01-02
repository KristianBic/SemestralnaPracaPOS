#include "Client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>




int main()
{
    ZOZNAM_VLAKIEN zoznamVlakien;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    char input[100];
    zoznamVlakien.pocetPrvkov = 0;
    pthread_t threadDownload;

    printf( "------------------------------------------------------------------------------------------- \n");
    while (1) {
        printf("Zadajte jeden z nasledujucich prikazov: download ; history ; exit \n");
        printf("\n->  ");
        gets(input);
        if(strcmp(input, "download") == 0) {
            //najprv implementovat veci a az potom dat do vlakna stahovanie
            URL_SLICED urlSliced;
            urlSliced = downloadInput();
            int sock = serverConnection(&urlSliced);
            zoznamVlakien.vlakna[zoznamVlakien.pocetPrvkov] = download(&urlSliced, sock);
            zoznamVlakien.pocetPrvkov++;

            pthread_create(&threadDownload, NULL, downloadThread, &zoznamVlakien);
            pthread_detach(threadDownload);

        } else if (strcmp(input, "history") == 0) {
            printf( "historia");
        } else if (strcmp(input, "pause") == 0) {
            pauseDownload(zoznamVlakien.vlakna[zoznamVlakien.pocetPrvkov - 1]);
        }else if (strcmp(input, "resume") == 0) {
            resumeDownload(zoznamVlakien.vlakna[zoznamVlakien.pocetPrvkov - 1]);
        }else if (strcmp(input, "stop") == 0) {
            stopDownload(zoznamVlakien.vlakna[zoznamVlakien.pocetPrvkov - 1]);
        }else if (strcmp(input, "start") == 0) {
            printf("************************************ %s\n",  zoznamVlakien.vlakna[zoznamVlakien.pocetPrvkov - 1].slicedURL->domain);

        }else if (strcmp(input, "exit") == 0) {
            break;
        } else {
            printf( "Zadali ste nespravny tvar. Skuste znovu. \n");
        }
    }
    printf( "------------------------------------------------------------------------------------------- \n");

    return 0;
}

void* downloadThread(void* vlaknaPar) {
    ZOZNAM_VLAKIEN* zoznamVlakien = vlaknaPar;
    startDownload(zoznamVlakien->vlakna[zoznamVlakien->pocetPrvkov - 1]);
    //tu dat startDownload
    printf("************************************ %s\n",  zoznamVlakien->vlakna[zoznamVlakien->pocetPrvkov - 1].slicedURL->domain);
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