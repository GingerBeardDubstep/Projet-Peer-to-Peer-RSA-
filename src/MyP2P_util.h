#ifndef RSA2021_JACQMIN_RENAULT_P2P_UTIL
#define RSA2021_JACQMIN_RENAULT_P2P_UTIL
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

#define USER_PORT_DL 2222
#define SERVER_PORT 2223
#define BUFFER_SIZE 1024

//metadata defs
#define COMMAND_SIZE 128
#define NAME_SIZE 64
#define TYPE_SIZE 32
#define HASH_SIZE 128
#define KEY_SIZE 64
#define MAX_KEYS 32
#define MAX_SEARCH_KEYS 16
#define MAX_RETURN_META 16

//DEBUG
#define MAX_LOG_SIZE 128

struct metadata {
	char* name;
	char* type; //à définir
	char* hash; //SHA-1
	char** keywords; //char* keywords[]
	int nbKeywords;
};

typedef struct metadata* META;

void serialize_metadata(META meta, char* path);
void copy_file(char* path, META meta);
void display_meta(META meta);
void clear_stdin();
void metacpy(META dst, META src);
void import_metadata(char* path, META meta);
META initiate_meta();
void destroy_meta();
META get_meta_from_file(char* path);
void import_test();
void print_ip(unsigned int ip);




#endif //RSA2021_JACQMIN_RENAULT_P2P_UTIL
