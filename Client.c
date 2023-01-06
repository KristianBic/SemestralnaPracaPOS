#include "Client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>


/* Táto funkcia slúži ako hlavný program a obsahuje interaktívny menu, v ktorom užívateľ môže zadať rôzne príkazy.
Možné príkazy sú "download" pre stiahnutie súboru, "historia" pre zobrazenie alebo vymazanie histórie stiahnutých súborov,
"pause" pre pozastavenie stiahnutia súboru, "resume" pre obnovenie stiahnutia súboru, "stop" pre zastavenie stiahnutia súboru
a "exit" pre ukončenie programu. Pri zadaní príkazu "download" sa vykonajú ďalšie funkcie pre spracovanie informácií
o stiahnutom súbore a spustenie vlákna pre stiahnutie súboru. Pri zadaní príkazu "historia" sa vykoná príkaz
"show" pre zobrazenie histórie stiahnutých súborov alebo "clear" pre vymazanie histórie.
Pri zadaní príkazu "pause", "resume" alebo "stop" sa vyberie ID procesu, ktorému sa má príkaz vykonať.
Pri zadaní príkazu "exit" sa program ukončí.*/
int main()
{
  pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t stop = PTHREAD_COND_INITIALIZER;
  pthread_cond_t start = PTHREAD_COND_INITIALIZER;

  MUTEX mutex = {&mut, &stop, &start, false};

  ZOZNAM_VLAKIEN zoznamVlakien;
  zoznamVlakien.pocetPrvkov = 0;
  char input[100];
  pthread_t threadDownload;

  printf("------------------------------------------------------------------------------------------- \n");
  while (1)
  {
    printf("Zadajte jeden z nasledujucich prikazov:  \n");
    printf("'download', 'historia', 'information', 'exit', 'pause', 'pauseALL', 'resume', 'resumeALL', 'stop', 'stopALL', 'deleteFile', 'deleteDir' \n");
    gets(input);
    if (strcmp(input, "download") == 0)
    {
      URL_SLICED urlSliced;
      urlSliced = downloadInput();
      int sock = serverConnection(&urlSliced);

       zoznamVlakien.vlakna[zoznamVlakien.pocetPrvkov] = download(&urlSliced, sock, zoznamVlakien.pocetPrvkov, &mutex);
       zoznamVlakien.vlakna[zoznamVlakien.pocetPrvkov].planningTime = imputPlanningTime();
       zoznamVlakien.pocetPrvkov++;

       pthread_create(&threadDownload, NULL, downloadThread, &zoznamVlakien);
       pthread_detach(threadDownload);
    }
    else if (strcmp(input, "information") == 0)
    {
      printInformations(&zoznamVlakien);
    }
    else if (strcmp(input, "priorita") == 0)
    {
      printInformations(&zoznamVlakien);
      printf("Zadajte id procesu, ktoremu chcete nastavit prioritu\n");
      char proces[2];
      gets(proces);
      char *command = strtok(proces, " ");
      for (int i = 0; i < zoznamVlakien.pocetPrvkov; ++i)
      {
        char str[2];
        sprintf(str, "%d", i);
        zoznamVlakien.vlakna[i].priority = 1; // vynulovat ostatne
        if (strcmp(command, str) == 0)
        {
          zoznamVlakien.vlakna[i].priority = 0; // nastavenie noveho
        }
      }
    }
    else if (strcmp(input, "historia") == 0)
    {
      printf("Zadajte co chcete robit - show , clear\n");
      char input[100];
      gets(input);
      if (strcmp(input, "show") == 0)
      {
        printLog();
      }
      else if (strcmp(input, "clear") == 0)
      {
        clear_log();
      }
      else
      {
        printf("Nespravny imput\n");
      }
    }
    else if (strcmp(input, "pause") == 0)
    {
      printInformations(&zoznamVlakien);
      printf("Zadajte id procesu\n");
      char proces[2];
      gets(proces);
      char *command = strtok(proces, " ");

      for (int i = 0; i < zoznamVlakien.pocetPrvkov; ++i)
      {
        char str[2];
        sprintf(str, "%d", i);
        if (strcmp(command, str) == 0)
        {
          pauseDownload(&zoznamVlakien.vlakna[i]);
        }
      }
    }
    else if (strcmp(input, "resume") == 0)
    {
      printInformations(&zoznamVlakien);
      printf("Zadajte id procesu\n");
      char proces[2];
      gets(proces);
      char *command = strtok(proces, " ");

      for (int i = 0; i < zoznamVlakien.pocetPrvkov; ++i)
      {
        char str[2];
        sprintf(str, "%d", i);
        if (strcmp(command, str) == 0)
        {
          resumeDownload(&zoznamVlakien.vlakna[i]);
        }
      }
    }
    else if (strcmp(input, "stop") == 0)
    {
      printInformations(&zoznamVlakien);
      printf("Zadajte id procesu\n");
      char proces[2];
      gets(proces);
      char *command = strtok(proces, " ");

      for (int i = 0; i < zoznamVlakien.pocetPrvkov; ++i)
      {
        char str[2];
        sprintf(str, "%d", i);
        if (strcmp(command, str) == 0)
        {
          stopDownload(&zoznamVlakien.vlakna[i]);
        }
      }
    }
    else if (strcmp(input, "deleteFile") == 0)
    {
      printInformations(&zoznamVlakien);
      printf("Zadajte nazov suboru\n");
      char proces[2];
      gets(proces);
      char *command = strtok(proces, " ");
      deleteFile(command);
    }
    else if (strcmp(input, "deleteDir") == 0)
    {
      printInformations(&zoznamVlakien);
      printf("Zadajte nazov suboru\n");
      char proces[2];
      gets(proces);
      char *command = strtok(proces, " ");
      if (strcmp(command, "") != 0)
      {
        deleteDirectory(command);
      }
    }
    else if (strcmp(input, "pauseALL") == 0)
    {
      for (int i = 0; i < zoznamVlakien.pocetPrvkov; ++i)
      {
        pauseDownload(&zoznamVlakien.vlakna[i]);
      }
    }
    else if (strcmp(input, "resumeALL") == 0)
    {
      for (int i = 0; i < zoznamVlakien.pocetPrvkov; ++i)
      {
        resumeDownload(&zoznamVlakien.vlakna[i]);
      }
    }
    else if (strcmp(input, "stopALL") == 0)
    {
      for (int i = 0; i < zoznamVlakien.pocetPrvkov; ++i)
      {
        stopDownload(&zoznamVlakien.vlakna[i]);
      }
    }
    else if (strcmp(input, "exit") == 0)
    {
      for (int i = 0; i < zoznamVlakien.pocetPrvkov; ++i)
      {
        if (zoznamVlakien.vlakna[i].downloading == true)
        {
          stopDownload(&zoznamVlakien.vlakna[i]);
        }
      }
      break;
    }
    else
    {
      printf("Zadali ste nespravny tvar. Skuste znovu. \n");
    }
    printf("\n");
  }
  printf("------------------------------------------------------------------------------------------- \n");


  for (int i = 0; i < zoznamVlakien.pocetPrvkov; ++i)
  {
    free((void *)zoznamVlakien.vlakna[i].slicedURL->domain);
    free((void *)zoznamVlakien.vlakna[i].slicedURL->protocol);
    free((void *)zoznamVlakien.vlakna[i].slicedURL->fullUrl);

      free((void *)zoznamVlakien.vlakna[i].slicedURL->fileName);
      free((void *)zoznamVlakien.vlakna[i].slicedURL->localFileName);
      free((void *)zoznamVlakien.vlakna[i].slicedURL->localDomainPath);

    pthread_mutex_destroy(zoznamVlakien.vlakna[i].mutex->mut);
    pthread_cond_destroy(zoznamVlakien.vlakna[i].mutex->stop);
    pthread_cond_destroy(zoznamVlakien.vlakna[i].mutex->start);
  }

  return 0;
}

/* Táto funkcia slúži na výpis informácií o vláknach pre stiahnutie súboru.
 Vstupom je ukazovateľ na štruktúru ZOZNAM_VLAKIEN, ktorá obsahuje pole štruktúr CLIENT_INFO s informáciami
 o jednotlivých vláknach. Pre každé vlákno sa vypíšu jeho ID, doména a priorita*/
void printInformations(ZOZNAM_VLAKIEN *ptr)
{
    printf("------------------------------------------------------------------------------------------- \n");
  for (int i = 0; i < ptr->pocetPrvkov; ++i)
  {
    double percentage = (double)ptr->vlakna[i].downloadedSize / ptr->vlakna[i].fileSize;
    int bar_length = 20;
    int progress = (int)(percentage * bar_length);
    printf("\r[%s] ID: %d, Priorita: %d, Stahovanie suboru: '%s', Prijate: %.2f/%.2f MB, Rychlost: (%.2f MB/s) [", getCurrentTime(), ptr->vlakna[i].id, ptr->vlakna[i].priority, ptr->vlakna[i].slicedURL->localFileName, (double)ptr->vlakna[i].downloadedSize / (1024 * 1024), (double)ptr->vlakna[i].fileSize / (1024 * 1024),  ptr->vlakna[i].currentSpeed / 1024.0 / 1024.0);
      for (int i = 0; i < bar_length; i++)
      {
           printf("%c", i <= progress ? '#' : ' ');
      }
       printf("] \n");
  }
    printf("------------------------------------------------------------------------------------------- \n");
}

/* Funkcia downloadThread() slúži ako vlákno pre stiahnutie súboru. Vstupom je ukazovateľ na štruktúru ZOZNAM_VLAKIEN,
ktorá obsahuje pole štruktúr CLIENT_INFO s informáciami o jednotlivých vláknach.
Počká sa určitý čas, ktorý je určený v poli planningTime v poslednom prvku v poli štruktúr CLIENT_INFO
a následne sa zavolá funkcia startDownload() s ukazovateľom na posledné vlákno v zozname.*/
void *downloadThread(void *vlaknaPar)
{
  ZOZNAM_VLAKIEN *zoznamVlakien = vlaknaPar;
  sleep(zoznamVlakien->vlakna[zoznamVlakien->pocetPrvkov - 1].planningTime);
  printf("------------------------------------------------------------------------------------------- \n");
  startDownload(&zoznamVlakien->vlakna[zoznamVlakien->pocetPrvkov - 1]);
}

/* Funkcia downloadInput() slúži na načítanie vstupu pre stiahnutie súboru od používateľa.
Vráti štruktúru URL_SLICED, ktorá obsahuje informácie o URL súboru, názve lokálneho súboru
a ceste k lokálnemu adresáru. Používateľ môže zadať URL súboru, názov lokálneho súboru a adresára, kam*/
URL_SLICED downloadInput()
{
  URL_SLICED urlSliced;
  char urlConsole[100];
  char localFileConsole[100];
  char localDirectory[100];
  char urlConsoleDefault[100] = "ftp://ftp.cs.brown.edu/pub/Effective_C++_errata.txt";

  printf("------------------------------------------------------------------------------------------- \n");
  printf("Zadajte link(URL) alebo stlacte enter pre stiahnutie suboru. DEFAULT: http://xcal1.vodafone.co.uk/5MB.zip\n");
  gets(urlConsole);
  if (strcmp(urlConsole, "") == 0)
  {
    split_url(&urlSliced, urlConsoleDefault);
    printf("Zadana adresa je: %s \n", urlConsoleDefault);
  }
  else
  {
    split_url(&urlSliced, urlConsole);
    printf("Zadana adresa je: %s \n", urlConsole);
  }
  if (atoi(urlSliced.port) > 65536 || atoi(urlSliced.port) < 0)
  {
    printf("Neplatne cislo portu!\n");
    exit(1);
  }
  printf("------------------------------------------------------------------------------------------- \n");
  printf("Zadajte nazov noveho suboru alebo stlacte enter a nazov sa zachova totozny s URL\n");
  gets(localFileConsole);
  char *localFileName;
  localFileName = localFileConsole;
  if (strcmp(localFileConsole, "") != 0)
  {
      urlSliced.localFileName = strcpy((char *)malloc(strlen(localFileConsole) + 1), localFileConsole);
  }

    if(access(urlSliced.localFileName, F_OK) == 0) { //if file exists
        printf("Subor s rovnakym nazvom uz existuje. Chcete z neho spravit kopiu? Pokial nie tak sa subor prepise. (a/n): \n");
        char answer[100];
        gets(answer);
        if(strcmp(answer, "a") == 0) {
            get_unique_filename(urlSliced.localFileName);
        } else if(strcmp(answer, "n") == 0) {
            //nerobime nic
        } else {
            printf("Error: Zly vstup.\n");
        }
    }

    printf("Zadany nazov serv suboru je: %s \n", urlSliced.fileName);
  printf("Zadany nazov lokalneho suboru je: %s \n", urlSliced.localFileName);
  printf("------------------------------------------------------------------------------------------- \n");

  printf("Existujuce adresare: ");
  printDirectory(getCurrentDirectory());

  //treba prerobit lebo do downloadu musime davat dobry path aby sa to stiahlo z url
  printf("Zadajte nazov adresara alebo stlacte enter a nazov sa zachova podla povodneho adresara.\n");
  gets(localDirectory);
  if (strcmp(localDirectory, "") == 0)
  {
    printf("Adresar ostava nezmeneny\n");
  }
  else if (!directoryExists(localDirectory))
  {
    createDirectory(localDirectory);
    strcat(localDirectory, "/");
    strcat(localDirectory, urlSliced.localFileName);
    urlSliced.localDomainPath = localDirectory;
  }
  else if (directoryExists(localDirectory))
  {
    chdir(localDirectory);
    strcat(localDirectory, "/");
    strcat(localDirectory, urlSliced.localFileName);
    urlSliced.localDomainPath = localDirectory;
  }

  if (strcmp(urlSliced.protocol, "ftp") == 0 || strcmp(urlSliced.protocol, "ftps") == 0)
  {
      char port[256], username[256], password[256];
      printf("Enter FTP/FTPS username: ");
      gets(username);
      if (strcmp(username, "") == 0)
      {
          urlSliced.username = "";
      } else {
          urlSliced.username = strcpy((char *)malloc(strlen(username) + 1), username);
      }

      printf("Enter FTP/FTPS password: ");
      gets(password);
      if (strcmp(password, "") == 0)
      {
          urlSliced.password = "";
      } else {
          urlSliced.password = strcpy((char *)malloc(strlen(password) + 1), password);
      }

      printf("Enter FTP/FTPS port number: \n");
      gets(port);
      if (!isdigit(*port))
      {
          printf("Error: Invalid port number.\n");
      }
      if(strcmp(port, "") == 0) {
          urlSliced.port = "21"; //treba este skontrolovat ci uz nahodou nebolo v ipecke port ... v spliceURL to robi ale iba pri ipcke a nie pri urlcke
      } else {
          urlSliced.port = strcpy((char *)malloc(strlen(port) + 1), port);
      }
  }


  printf("------------------------------------------------------------------------------------------- \n");
  printf("Aktualny adresar: %s\n", getCurrentDirectory());
  printf("Protocol: %s\nSite: %s\nPort: %s\nPath: %s\nFileName: %s\nLocal Path: %s\nLocal FileName: %s\n",
         urlSliced.protocol, urlSliced.domain, urlSliced.port, urlSliced.domainPath, urlSliced.fileName, urlSliced.localDomainPath, urlSliced.localFileName);
  if (strcmp(urlSliced.protocol, "ftp") == 0 || strcmp(urlSliced.protocol, "ftps") == 0) {
      printf("Username: %s\nPassword: %s\n",
             urlSliced.username, urlSliced.password);
  }
  printf("------------------------------------------------------------------------------------------- \n");

  return urlSliced;
}

int imputPlanningTime()
{
  char cas[100];
  char hodiny[100], minuty[100], sekundy[100];

  printf("Chcete naplanovať cas, kedy sa mm stahovanie zacat? (a/n)\n");
  gets(cas);
  if (strcmp(cas, "a") == 0)
  {
    printf("Zadajte pocet hodin \n");
    gets(hodiny);
    printf("Zadajte pocet minut \n");
    gets(minuty);
    printf("Zadajte pocet sekund \n");
    gets(sekundy);
    printf("Stahovanie sa uskutocni o %d:%d:%d \n", atoi(hodiny), atoi(minuty), atoi(sekundy));
  }
  else if (strcmp(cas, "n") == 0)
  {
  }
  else
  {
    printf("Zadali ste nespravy vstup. Budeme pokracovat v stahovani bez nacasovania. \n");
  }
  printf("------------------------------------------------------------------------------------------- \n");
  return (atoi(sekundy) + (atoi(minuty) * 60) + (atoi(hodiny) * 60 * 60));
}
