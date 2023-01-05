#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <dirent.h>
#include "functions.h"

/*Tento kód je zodpovedný za rozdelenie URL adresy na jednotlivé časti, ako napríklad protokol, doménu, port a cestu k súboru.
Tiež obsahuje funkcie na stiahnutie HTML obsahu z HTTP servera.
Prvá funkcia split_url rozdeľuje URL adresu na jednotlivé časti a ukladá ich do štruktúry URL_SLICED.
Štruktúra má nasledujúce polia:

protocol - protokol použitý v URL adrese, napríklad "http"
domain - doména URL adresy, napríklad "www.example.com"
port - port použitý v URL adrese
fullUrl - celé URL adresy
domainPath - cesta k súboru na serveri, napríklad "/index.html"
fileName - názov súboru na serveri, napríklad "index.html"*/
URL_SLICED *split_url(URL_SLICED *slicedURL, const char *url)
{
    if (!slicedURL || !url)
        return NULL;
    slicedURL->protocol = strtok(strcpy((char *)malloc(strlen(url) + 1), url), "://");
    slicedURL->domain = strstr(url, "://");
    if (slicedURL->domain)
    {
        slicedURL->domain += 3;
        char *site_port_path = strcpy((char *)calloc(1, strlen(slicedURL->domain) + 1), slicedURL->domain);
        slicedURL->domain = strtok(site_port_path, ":");
        slicedURL->domain = strtok(site_port_path, "/");
    }
    else
    {
        char *site_port_path = strcpy((char *)calloc(1, strlen(url) + 1), url);
        slicedURL->domain = strtok(site_port_path, ":");
        slicedURL->domain = strtok(site_port_path, "/");
    }
    slicedURL->fullUrl = strcpy((char *)malloc(strlen(url) + 1), url);
    slicedURL->port = strstr(slicedURL->fullUrl + 6, ":");
    char *port_path = 0;
    char *port_path_copy = 0;
    if (slicedURL->port && isdigit(*(port_path = (char *)slicedURL->port + 1)))
    {
        port_path_copy = strcpy((char *)malloc(strlen(port_path) + 1), port_path);
        char *r = strtok(port_path, "/");
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
        char *path = strstr(slicedURL->fullUrl + 8, "/");
        slicedURL->domainPath = path ? path : "/";
    }
    int r = strcmp(slicedURL->protocol, slicedURL->domain) == 0;
    if (r && slicedURL->port == "80")
        slicedURL->protocol = "http";
    else if (r)
        slicedURL->protocol = "tcp";

    char *fileName = strrchr(slicedURL->domainPath, '/') + 1;
    if (fileName && *(fileName)) {
        slicedURL->fileName = strcpy((char *)malloc(strlen(fileName) + 1), fileName);
        slicedURL->localFileName = strcpy((char *)malloc(strlen(fileName) + 1), fileName);
    }
    slicedURL->localDomainPath = strcpy((char *)malloc(strlen(slicedURL->domainPath) + 1), slicedURL->domainPath);

    return slicedURL;
}

/* Funkcia contentLength zisťuje veľkosť obsahu (v bajtoch) na základe hlavičky HTTP odpovede.
Funkcia vráti hodnotu -1, ak sa v hlavičke nenachádza informácia o veľkosti obsahu.*/
int contentLength(char *length)
{
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


/* Táto funkcia slúži na vymazanie obsahu zo súboru "log_file.txt".
Ak súbor neexistuje alebo nie je možné ho otvoriť na zápis, funkcia vypíše chybové hlásenie*/
void clear_log()
{
    FILE *fp = fopen("log_file.txt", "w");
    if (fp == NULL)
    {
        printf("Error: nemohol otvorit subor log_file na vymazanie historie\n");
    }
    else
    {
        fclose(fp);
    }
}

/*Táto funkcia slúži na výpis obsahu súboru "log_file.txt".
Ak súbor neexistuje alebo nie je možné ho otvoriť na čítanie, funkcia vypíše chybové hlásenie.*/
void printLog()
{
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
}

/* Táto funkcia slúži na prevod veľkosti súboru na ľudovo čitateľnú jednotku (napríklad B, KB, MB alebo GB).
Funkcia vráti ukazovateľ na reťazec s jednotkou ako návratovú hodnotu.*/
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

/* Funkcia write_to_log slúži na zápis informácií o sťahovanom súbore do log súboru a tiež na výpis týchto informácií na konzolu.
Funkcia otvorí súbor "log_file.txt" v režime pridávania (append), získa aktuálny čas, aktuálny pracovný adresár,
a jednotku pre veľkosť súboru. Potom zapíše záznam do súboru a vypíše ho aj na konzolu.
Na konci funkcie sa súbor zavrie a uvoľní sa alokovaná pamäť pre aktuálny čas.
Funkcia tiež vyšle signál pre ďalšie vlákno a odomkne zámok mutexu.*/
void write_to_log(char *filename, char *url, int size, char *sizeUnit, double elapsed_time, pthread_cond_t *start, pthread_mutex_t *mut)
{
    FILE *log_file;
    FILE *fp = fopen("log_file.txt", "a");
    if (fp == NULL)
    {
        perror("Error: nemohol otvorit subor log_file na zapis");
        return;
    }
    char *time_str = getCurrentTime();
    char *cwd = getCurrentDirectory();
    char *size_units = getSizeUnit(size);

    fprintf(fp, "[%s] Stiahnuty subor '%s' (%s za %.2lf sekund) z %s do %s\n", time_str, filename, size_units, elapsed_time, url, cwd);
    printf("[%s] Stiahnuty subor '%s' (%s za %.2lf sekund) z %s do %s\n", time_str, filename, size_units, elapsed_time, url, cwd);

    fclose(fp);
    free(time_str);
    pthread_cond_signal(start);
    pthread_mutex_unlock(mut);
}

/* Táto funkcia slúži na získanie aktuálneho pracovného adresára.
Funkcia vráti ukazovateľ na reťazec s názvom adresára ako návratovú hodnotu.
Ak sa nepodarí získať aktuálny adresár, funkcia vypíše chybové hlásenie.*/
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

/* Táto funkcia slúži na získanie aktuálneho dátumu a času v formáte "YYYY-MM-DD HH:MM:SS".
Funkcia vráti ukazovateľ na reťazec s aktuálnym dátumom a časom ako návratovú hodnotu.
Funkcia vyžaduje alokáciu pamäte pre reťazec s aktuálnym dátumom a časom, takže je potrebné pamäť po použití uvoľniť.*/
char *getCurrentTime()
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char *time_str = malloc(sizeof(char) * 20);
    strftime(time_str, 20, "%Y-%m-%d %H:%M:%S", tm);
    return time_str;
}

/* Funkcia createDirectory vytvorí nový adresár s názvom path.
Funkcia vráti 0 ak bol adresár úspešne vytvorený, inak vráti -1.*/
int createDirectory(const char *path)
{
    int status = mkdir(path, 0777);
    if (status < 0)
    {
        perror("Error: vytvaranie adresara - mkdir");
        return -1;
    }
    chdir(path);
    return 0;
}

/* Funkcia directoryExists skontroluje, či adresár s názvom path existuje.
Vráti hodnotu true ak adresár existuje, false ak adresár neexistuje.*/
int directoryExists(const char *path)
{
    struct stat sb;
    return stat(path, &sb) == 0 && S_ISDIR(sb.st_mode);
}

/* Funkcia printDirectoryContents vypíše obsah adresára directory na konzolu.
Ak sa adresár otvorí úspešne, prejde sa pomocou cyklu cez jednotlivé položky v adresári a vypíšu sa ich názvy.*/
void printDirectoryContents(const char *directory)
{
    DIR *dir = opendir(directory);
    if (dir == NULL)
    {
        perror("Error: pri otvoreni adresara.");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        printf("%s\n", entry->d_name);
    }

    closedir(dir);
}

/* Funkcia printDirectory vypíše názvy podadresárov v adresári directory na konzolu.*/
void printDirectory(const char *directory)
{
    DIR *dir = opendir(directory);
    if (dir == NULL)
    {
        perror("Error: pri otvoreni adresara.");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_DIR)
        {
            printf("%s\t", entry->d_name);
        }
    }
    printf("\n");
    closedir(dir);
}

/* Funkcia deleteFile vymaže súbor s názvom filename.
Funkcia vráti hodnotu 0 ak bol súbor úspešne vymazaný, inak vráti chybové hlásenie.*/
void deleteFile(const char *filename)
{
    if (unlink(filename) != 0)
    {
        perror("Error: pri mazani suboru");
        return;
    }
}

/* Táto funkcia slúži na vymazanie adresára a všetkého, čo sa v ňom nachádza.
Funkcia otvorí adresár pomocou volania opendir, a následne prejde rekurzívne všetky položky,
ktoré sa v ňom nachádzajú. V prípade, že sa jedná o adresár, volá sa funkcia rekurzívne na tento adresár.
V prípade, že sa jedná o súbor, použije sa funkcia unlink na jeho vymazanie.
Po prejdení všetkých položiek sa adresár zavrie pomocou closedir a samotný adresár sa vymaže pomocou rmdir.
V prípade chyby sa vypíše chybové hlásenie pomocou perror.*/
void deleteDirectory(const char *directory)
{
    DIR *dir = opendir(directory);
    if (dir == NULL)
    {
        perror("Error: pri otvoreni adresara.");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name);
        if (entry->d_type == DT_DIR)
        {
            deleteDirectory(path);
        }
        else
        {
            if (unlink(path) != 0)
            {
                perror("Error: pri vymazavani suboru.");
                closedir(dir);
                return;
            }
        }
    }
    closedir(dir);
    if (rmdir(directory) != 0)
    {
        perror("Error: pri mazani adresara.");
        return;
    }
}

void  get_unique_filename(char *filename) {
    char *dot_pos = strrchr(filename, '.');

    if (dot_pos == NULL) {
        int index = 1;
        char new_filename[256];
        while (true) {
            sprintf(new_filename, "%s_(%d)", filename, index);
            if (access(new_filename, F_OK) == -1) {
                strcpy(filename, new_filename);
                break;
            }
            index++;
        }
    } else {

        int index = 1;
        char new_filename[256];
        char *file_extension = dot_pos + 1;
        *dot_pos = '\0';
        while (true) {
            sprintf(new_filename, "%s_(%d).%s", filename, index, file_extension);
            if (access(new_filename, F_OK) == -1) {
                strcpy(filename, new_filename);
                break;
            }
            index++;
        }
    }
}
