#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <pthread.h>

#include "download.h"

/*Vytvorí a inicializuje štruktúru CLIENT_INFO s údajmi o stiahnutom súbore.
Parametre:
slicedURL: ukazovateľ na štruktúru URL_SLICED obsahujúcu informácie o URL súboru na stiahnutie.
socked: soket, pomocou ktorého sa bude súbor stahovať.
id: ID stiahnutého súboru.
mut: ukazovateľ na mutex pre synchronizáciu vlákien.
Návratová hodnota: štruktúra CLIENT_INFO s inicializovanými údajmi.*/
CLIENT_INFO download(URL_SLICED *slicedURL, int socked, int id, MUTEX *mut)
{
  CLIENT_INFO clientA;
  clientA.mutex = mut;
  clientA.slicedURL = slicedURL;
  clientA.sockfd = socked;
  clientA.localFile = slicedURL->fileName;
  clientA.username = "";
  clientA.password = "";
  clientA.fileSize = 0;
  clientA.downloadedSize = 0;
  clientA.priority = 0;
  clientA.pause = false;
  clientA.id = id;
  printf("Data na stahovanie boli inicializovane\n");
  return clientA;
}

/*Funkcia spustí stahovanie súboru na základe informácií uložených v štruktúre CLIENT_INFO.
Parametre:
client: ukazovateľ na štruktúru CLIENT_INFO s údajmi o stiahnutom súbore.*/
void startDownload(CLIENT_INFO *client)
{
  client->downloading = true;
  if (client->pause)
  {
    resumeDownload(client);
    return;
  }
  if (strcmp(client->slicedURL->protocol, "http") == 0)
  {
    downloadHTTP(client);
  }
  else if (strcmp(client->slicedURL->protocol, "https") == 0)
  {
    downloadHTTP(client);
    // https();
  }
  else if (strcmp(client->slicedURL->protocol, "ftp") == 0)
  {
    downloadHTTP(client);
    // ftp();
  }
  else if (strcmp(client->slicedURL->protocol, "ftps") == 0)
  {
    // ftps();
  }
}

/*Funkcia pozastaví stahovanie súboru.
Parametre:
client: ukazovateľ na štruktúru CLIENT_INFO s údajmi o stiahnutom súbore.*/
void pauseDownload(CLIENT_INFO *client)
{
  printf("Stahovanie sa pozastavilo %s\n", client->slicedURL->domain);

  client->downloading = false;
  client->pause = true;
  client->resume = false;
}

/*Funkcia zruší stahovanie súboru.
Parametre:
client: ukazovateľ na štruktúru CLIENT_INFO s údajmi o stiahnutom súbore. */
void stopDownload(CLIENT_INFO *client)
{
  client->downloading = false;
  client->stop = true;
  client->pause = true;
  printf("Stahovanie sa zruselo\n");
}

/*Funkcia obnoví pozastavené stahovanie súboru.
Parametre:
client: ukazovateľ na štruktúru CLIENT_INFO s údajmi o stiahnutom súbore.*/
void resumeDownload(CLIENT_INFO *client)
{
  client->downloading = true;
  client->resume = true;
  client->pause = false;

  printf("Stahovanie sa znovu spustilo\n");
  /*
 if (client->stop) {
     return;
 }
 if (strcmp(client->slicedURL->protocol, "http") == 0) {
     http_download_file(client);
 } else if (strcmp(client->slicedURL->protocol, "https") == 0) {
     http_download_file(client);
     //https();
 } else if (strcmp(client->slicedURL->protocol, "ftp") == 0) {
     http_download_file(client);
     //ftp();
 } else if (strcmp(client->slicedURL->protocol, "ftps") == 0) {
     //ftps();
 }
  */
}

/*Táto funkcia sa používa na stiahnutie súboru cez protokol HTTP.
Jej vstupom je ukazovateľ na štruktúru CLIENT_INFO s informáciami o sťahovaní.
Funkcia otvára lokálny súbor na zapisovanie a vysiela požiadavku na sťahovanie súboru pomocou funkcie send.
Potom z HTTP odpovede získava dĺžku sťahovaného súboru a zavolá funkciu http_download_file,
ktorá sa postará o samotné sťahovanie. Na konci sa zatvára lokálny súbor.*/
void downloadHTTP(CLIENT_INFO *client)
{
  printf("Starting sownload %s\n", client->slicedURL->domain);

  int fd;
  if ((fd = open(client->localFile, O_WRONLY | O_CREAT, 0666)) == -1)
  { // open file
    printf("Error. Subor sa neda otvorit\n");
    return;
  }
  // nasledujuci kod nefunguje ... neviem preco
  /*
  if(access(client.localFile, F_OK) == 0) { //if file exists
      printf("Subor s rovnakym nazvom uz existuje. Chcete ho prepisat? (a/n): ");
      char answer[256];
      fgets(answer, sizeof(answer), stdin);
      answer[0] = tolower(answer[0]);
      if(answer[0] == 'a') {
          printf("Prepisujem subor. Pokracujeme v stahovani.\n");
      } else if(answer[0] == 'n') {
          printf("Canceling download\n");
          return;
      } else {
          printf("Error: Zly vstup. Rusi sa stahovanie.\n");
          return;
      }
  } else {
      printf("Pokracujeme v stahovani.\n");
  }
  */
  printf("Stahovanie suboru %s...\n", client->localFile);

  char recv_data[BUFFER_SIZE];
  char send_data[BUFFER_SIZE];

  sprintf(send_data, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", client->slicedURL->domainPath, client->slicedURL->domain);
  if (send(client->sockfd, send_data, strlen(send_data), 0) < 0)
  {
    perror("Error sending HTTP request - Send data");
    exit(1);
  }
  printf("Data send...\n");

  // recv(sockfd,recv_data,sizeof(recv_data),0);
  if (recv(client->sockfd, recv_data, BUFFER_SIZE - 1, 0) < 0)
  {
    perror("Error reading HTTP response - send data");
    exit(2);
  }
  recv_data[BUFFER_SIZE - 1] = '\0';
  printf("Received HTTP response\n");

  // Extract content length from HTTP response
  char *length = strtok(recv_data, "\r\n");
  client->fileSize = contentLength(length);
  printf("Content length of file is: %s\n", getSizeUnit(client->fileSize));

  if (client->fileSize < 0)
  {
    fprintf(stderr, "Error: content length not found in HTTP response\n");
    exit(3);
  }
  http_download_file(client);
  close(fd);
}

/*Táto funkcia slúži na samotné sťahovanie súboru cez protokol HTTP.
Jej vstupom je ukazovateľ na štruktúru CLIENT_INFO s informáciami o sťahovaní.
Funkcia otvára lokálny súbor na zapisovanie a pomocou funkcie recv číta dáta zo sťahovaného súboru
a zapisuje ich do lokálneho súboru. Sťahovanie prebieha v cykle, kým sa stále niečo číta zo sťahovaného súbor.
Po skončení sťahovania sa zavrie miestny súbor a ukončí sa vlákno.*/
void *http_download_file(CLIENT_INFO *client)
{
  printf("Zacina sa stahovanie\n");

  // Open local file for writing
  FILE *fp = fopen(client->localFile, "w+");
  if (fp == NULL)
  {
    perror("Error opening local file for writing");
    pthread_exit(NULL);
  }

  // Read file data from HTTP response
  char buffer[BUFFER_SIZE];
  int bytes_read;
  struct timeval start, end;
  double elapsed;
  gettimeofday(&start, NULL);
  double speedLimit;

  while ((bytes_read = recv(client->sockfd, buffer, BUFFER_SIZE, 0)) > 0)
  {
    if (client->stop)
    {
      printf("\nStahovanie sa ukoncilo a subor sa vymazal! STOPPED: %s\n", client->localFile);
      fclose(fp);
      deleteFile(client->slicedURL->fileName);
      close(client->sockfd);
      pthread_exit(NULL);
    }
    if (client->pause)
    {
      sleep(1);
    }
    else
    {
      gettimeofday(&end, NULL);
      elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
      double currentSpeed = (double)client->downloadedSize / elapsed;
      double percentage = (double)client->downloadedSize / client->fileSize;

      // Set initial speed limit based on priority
      switch (client->priority)
      {
      case 1:
        speedLimit = 3.0; // drzi opkolo 2.0 - 2.08 MB/s
        break;
      default:
        speedLimit = 0.0; // Unlimited
        break;
      }
      // riesenie  1
      if (currentSpeed > speedLimit)
      {
        double delay = (((double)bytes_read / (1024 * 1024)) / speedLimit) * 1000000.0;
        usleep(delay);
      }

      fwrite(buffer, 1, bytes_read, fp);
      client->downloadedSize += bytes_read;
      // Display progress bar
      int bar_length = 20;
      int progress = (int)(percentage * bar_length);
      char *time_str = getCurrentTime();
      // printf("\r%s Downloading... %.2f/%.2f M bytes (%.2f%%) received (%.2f MB/s) [", time_str, (double)client->downloadedSize / (1024 * 1024), (double)client->fileSize / (1024 * 1024), percentage * 100, currentSpeed / 1024.0 / 1024.0);
      for (int i = 0; i < bar_length; i++)
      {
        // printf("%c", i <= progress ? '#' : ' ');
      }
      // printf("] (Time: %.2f seconds)", elapsed);
      fflush(stdout);
      free(time_str);
    }
  }
  // Close local file
  fclose(fp);
  // Close socket

  close(client->sockfd);
  printf("\nStahovanie dokoncene: %s. Pthread: %s CLOSED!\n", client->localFile, client->slicedURL->domain);

  while (client->mutex->logging == true)
  {
    printf("Download musi cakat \n");
    pthread_cond_wait(client->mutex->start, client->mutex->mut);
  }
  client->mutex->logging = true;
  write_to_log(client->localFile, (char *)client->slicedURL->domain, client->downloadedSize, getSizeUnit(client->downloadedSize), elapsed, client->mutex->start, client->mutex->mut);
  client->mutex->logging = false;

  pthread_cond_signal(client->mutex->stop);
  pthread_mutex_unlock(client->mutex->mut);

  pthread_exit(NULL);
}
