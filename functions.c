#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include "functions.h"

URL_SLICED* split_url(URL_SLICED* slicedURL, const char* url){
    if (!slicedURL || !url)
        return NULL;
    slicedURL->protocol = strtok(strcpy((char*)malloc(strlen(url) + 1), url), "://");
    slicedURL->domain = strstr(url, "://");
    if (slicedURL->domain)
    {
        slicedURL->domain += 3;
        char* site_port_path = strcpy((char*)calloc(1, strlen(slicedURL->domain) + 1), slicedURL->domain);
        slicedURL->domain = strtok(site_port_path, ":");
        slicedURL->domain = strtok(site_port_path, "/");
    }
    else
    {
        char* site_port_path = strcpy((char*)calloc(1, strlen(url) + 1), url);
        slicedURL->domain = strtok(site_port_path, ":");
        slicedURL->domain = strtok(site_port_path, "/");
    }
    char* URL = strcpy((char*)malloc(strlen(url) + 1), url);
    slicedURL->port = strstr(URL + 6, ":");
    char* port_path = 0;
    char* port_path_copy = 0;
    if (slicedURL->port && isdigit(*(port_path = (char*)slicedURL->port + 1)))
    {
        port_path_copy = strcpy((char*)malloc(strlen(port_path) + 1), port_path);
        char * r = strtok(port_path, "/");
        if (r)
            slicedURL->port = r;
        else
            slicedURL->port = port_path;
    }
    else
        slicedURL->port = "80";
    if (port_path_copy)
        slicedURL->domainPath = port_path_copy + strlen(slicedURL->port ? slicedURL->port : "");
    else
    {
        char* path = strstr(URL + 8, "/");
        slicedURL->domainPath = path ? path : "/";
    }
    int r = strcmp(slicedURL->protocol, slicedURL->domain) == 0;
    if (r && slicedURL->port == "80")
        slicedURL->protocol = "http";
    else if (r)
        slicedURL->protocol = "tcp";

    char *fileName = strrchr(slicedURL->domainPath, '/') + 1;
    if (fileName && *(fileName))
        slicedURL->fileName = fileName;

    //free(URL);
    //free(port_path_copy);
    return slicedURL;

}

int contentLength(char *length) {
    int content_length = -1;

    while (length != NULL)
    {
        if (strncmp(length, "Content-Length: ", 16) == 0)
        {
            content_length = atoi(length + 16);
            break;
        }
        length = strtok(NULL, "\r\n");
    }
    return content_length;
}

void downloadHTMLfromHTTP(int sockfd) {
    char buf[2056];
    int byte_count;

    char *header = "GET /index.html HTTP/1.1\r\nHost: www.example.com\r\n\r\n";
    send(sockfd,header,strlen(header),0);
    printf("GET Sent...\n");
    //all right ! now that we're connected, we can receive some data!
    byte_count = recv(sockfd,buf,sizeof(buf),0);
    printf("recv()'d %d bytes of data in buf\n",byte_count);
    printf("%.*s",byte_count,buf); // <-- give printf() the actual data size
}

// Clear log file
void clear_log()
{
    // pthread_mutex_lock(&log_lock);
    FILE *fp = fopen("log_file.txt", "w");
    if (fp == NULL)
    {
        printf("Error: nemohol otvorit subor log_file na vymazanie historie\n");
    }
    else
    {
        fclose(fp);
    }
    // pthread_mutex_unlock(&log_lock);
}

// Display log file
void printLog()
{
    // pthread_mutex_lock(&log_lock);
    FILE *fp = fopen("log_file.txt", "r");
    if (fp == NULL)
    {
        printf("Error: nemohol otvorit subor log_file na vypis\n");
    }
    else
    {
        char line[1024];
        while (fgets(line, 1024, fp) != NULL)
        {
            printf("%s", line);
        }
        fclose(fp);
    }
    // pthread_mutex_unlock(&log_lock);
}

char *getSizeUnit(double size)
{
    static char unit[4];
    if (size < 1024)
    {
        sprintf(unit, "%d B", size);
    }
    else if (size < 1024 * 1024)
    {
        sprintf(unit, "%.1f KB", (double)size / 1024.0);
    }
    else if (size < 1024 * 1024 * 1024)
    {
        sprintf(unit, "%.1f MB", (double)size / (1024.0 * 1024.0));
    }
    else
    {
        sprintf(unit, "%.1f GB", (double)size / (1024.0 * 1024.0 * 1024.0));
    }
    return unit;
}

void write_to_log(char *filename, char *url, int size, char *sizeUnit, double elapsed_time,   pthread_cond_t * start,  pthread_mutex_t* mut)
{

    FILE *log_file;
    // Open log file in append mode
    FILE *fp = fopen("log_file.txt", "a");
    if (fp == NULL)
    {
        perror("Error opening log file");
        return;
    }
    // Get current time
    char *time_str = getCurrentTime();
    // Get current working directory
    char *cwd = getCurrentDirectory();
    char *size_units = getSizeUnit(size);

    // Write log entry to file
    fprintf(fp, "[%s] Downloaded file '%s' (%s in %.2lf seconds) from %s to %s\n", time_str, filename, size_units, elapsed_time, url, cwd);

    // Print log entry to console
    printf("[%s] Downloaded file '%s' (%s in %.2lf seconds) from %s to %s\n", time_str, filename, size_units, elapsed_time, url, cwd);

    // Close log file
    fclose(fp);
    //sleep(20);

    pthread_cond_signal(start);
    pthread_mutex_unlock(mut);

}

char *getCurrentDirectory()
{
    static char buffer[1024];
    if (getcwd(buffer, 1024) == NULL)
    {
        printf("Error: ziskanie aktualneho pracovneho adresara");
        return NULL;
    }
    return buffer;
}

char *getCurrentTime()
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char *time_str = malloc(sizeof(char) * 20);
    strftime(time_str, 20, "%Y-%m-%d %H:%M:%S", tm);
    return time_str;
}