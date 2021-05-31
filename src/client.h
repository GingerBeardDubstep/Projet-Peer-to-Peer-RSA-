#ifndef RSA2021_JACQMIN_RENAULT_CLIENT
#define RSA2021_JACQMIN_RENAULT_CLIENT
#include <sys/socket.h> //TCP -> Utiliser SOCK_STREAM 
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/signal.h>
#include "MyP2P_util.h"

#define MAX_CONN 10
#define MAX_WAIT 5
#define NB_TRY 10
#define TRY_GAP 1

#define DEBUG 0
#define INFO 0
#define WARNING 1
#define ERROR 1

void log_info(char* message);
void log_debug(char* message);
void log_warning(char* message);
void log_error(char* message);

void set_listening();

void init_dir();

void download(char* name, char* ip, char* dst);

void publish(int serverSocket,struct sockaddr_in serv_addr);
int search(int serverSocket, struct sockaddr_in serv_addr);
int get_keywords(char* keywords[]);
//void communicate(int serverSocket);

#endif //RSA2021_JACQMIN_RENAULT_CLIENT