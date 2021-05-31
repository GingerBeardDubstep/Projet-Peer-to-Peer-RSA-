#include "client.h"


//Red: \e[0;31m
//Blue: \e[0;34m
//Reset: \e[0m
//Green: \e[0;32m
//Magenta: \e[0;37m

void help(){
	printf("\e[0;32mCommands:\n");
	printf("\e[0;34m\t Listening mode: \"\e[0;31mlisten\e[0;34m\"\n");
	printf("\e[0;34m\t Search a file: \"\e[0;31msearch\e[0;34m\"\n");
	printf("\e[0;34m\t Post a file: \"\e[0;31mpost\e[0;34m\"\n");
	printf("\e[0;34m\t Quit: \"\e[0;31mquit\e[0;34m\"\n");
}

void interface(){
	
	{
		 printf(" __  __         _____ ___  _____\n") ; 
		 printf("|  \\/  |       |  __ \\__ \\|  __ \\\n") ;
		 printf("| \\  / |_   _  | |__) | ) | |__) |\n");
		 printf("| |\\/| | | | | |  ___/ / /|  ___/\n") ;
		 printf("| |  | | |_| | | |    / /_| |\n")   ;  
		 printf("|_|  |_|\\__, | |_|   |____|_|\n") ;    
		 printf("	 __/ |\n")  ;                  
		 printf("        |___/\n")  ;                   
	}
	
	help();
	
	for(;;){
		
		char commande[201];
		printf("\e[0;32mMyP2P$\e[0;37m");
		fgets(commande,200,stdin);
		commande[strlen(commande)-1] = 0;
		if( strcmp(commande,"listen") == 0){
			printf("\e[0;34mlistening...\n");
			set_listening();
		} else if (strcmp(commande, "search") == 0){
			int serverSocket;

			if((serverSocket = socket(PF_INET, SOCK_DGRAM,0)) < 0){
				perror("ERROR while creating server socket\n");
				exit(1);
			}
			struct sockaddr_in serv_addr;

			memset(&serv_addr, 0, sizeof(serv_addr));
			serv_addr.sin_family = PF_INET;
			serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
			serv_addr.sin_port = htons(SERVER_PORT);

			search(serverSocket,serv_addr);
		} else if (strcmp(commande,"post") == 0){
			log_debug("Initiating server socket connexion");
			int serverSocket;

			if((serverSocket = socket(PF_INET, SOCK_DGRAM,0)) < 0){
				perror("ERROR while creating server socket\n");
				exit(1);
			}
			struct sockaddr_in serv_addr;

			memset(&serv_addr, 0, sizeof(serv_addr));
			serv_addr.sin_family = PF_INET;
			serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
			serv_addr.sin_port = htons(SERVER_PORT);

			//import_metadata("../saves/client.sav",meta);
			publish(serverSocket,serv_addr);
		} else if (strcmp("quit",commande) == 0){
			printf("\e[0;34mclosing MyP2P\n");
			break;
		} else if (strcmp("help",commande) == 0){	
			help();
		} else {
			printf("\e[0;34mUnknown command: \"\e[0;31m%s\e[0;34m\". Type \"\e[0;31mhelp\e[0;34m\" to get the command list \n", commande);
		}
		
	}

}

int main(int argc, char* argv[]){
	
	init_dir();
	//printf("%d \n", argc);
	if(argc == 3){
		download(argv[1], argv[2], argv[1]);
	}else if(argc == 2 && ( (strcmp(argv[1],"-p") == 0) || (strcmp(argv[1],"--publish") == 0))){
		log_debug("Initiating server socket connexion");
		int serverSocket;

		if((serverSocket = socket(PF_INET, SOCK_DGRAM,0)) < 0){
			perror("ERROR while creating server socket\n");
			exit(1);
		}
		struct sockaddr_in serv_addr;

		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = PF_INET;
		serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		serv_addr.sin_port = htons(SERVER_PORT);

		//import_metadata("../saves/client.sav",meta);
		publish(serverSocket,serv_addr);
	}else if(argc == 2 && ( (strcmp(argv[1],"-s") == 0) || (strcmp(argv[1],"--search") == 0))){
		int serverSocket;

		if((serverSocket = socket(PF_INET, SOCK_DGRAM,0)) < 0){
			perror("ERROR while creating server socket\n");
			exit(1);
		}
		struct sockaddr_in serv_addr;

		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = PF_INET;
		serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		serv_addr.sin_port = htons(SERVER_PORT);

		search(serverSocket,serv_addr);
	} else {
		interface();
	}
	
	return 0;
}