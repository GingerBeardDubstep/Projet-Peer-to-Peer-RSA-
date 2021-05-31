#ifndef SERVEUR
#define SERVEUR
#include <sys/socket.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <fcntl.h>
#include <signal.h>
#include "MyP2P_util.h"

#define MAX_FILES 128
#define PATH_SIZE 72
#define NB_PORTS 64

#define SERVER_DEBUG 1
#define SERVER_INFO 1
#define SERVER_WARNING 1
#define SERVER_ERROR 1

void log_info(char* message);
void log_debug(char* message);
void log_warning(char* message);
void log_error(char* message);

struct file_report {
	int* ips;
	char** paths;
	int nbFiles;
};

typedef struct file_report* REPORT;

REPORT initialise_report();
void destroy_report(REPORT report);
REPORT import_report();
int delete_metadata(char* hash,int ip);
int is_hash_in_use(REPORT report, META meta);
void deletion(int serverSocket, struct sockaddr_in cli_addr, int clilen);
void display_report(REPORT report);
int add_file(REPORT report, META meta, int ip);
void serialize_report(REPORT report);
void serialize_report_without(REPORT report, int index);
void publish(int newSocket, struct sockaddr_in cli_addr, int clilen);
void search(int newSocket, struct sockaddr_in cli_addr, int clilen);
void communicate(int serverSocket);
int search_candidates(REPORT report, char* keywords[], META meta_list[], int ip_list[], int nbKeys);

#endif