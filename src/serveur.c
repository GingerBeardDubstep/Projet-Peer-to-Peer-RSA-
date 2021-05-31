#include "serveur.h"
sem_t* semaphore;


int main(int argc, char* argv[]){
	if(argc == 2 && ((strcmp(argv[1],"-r")) == 0 || (strcmp(argv[1],"--repo") == 0))){
		REPORT report = import_report();
		display_report(report);
		destroy_report(report);
	}else{
		log_debug("Initiating semaphore");
		sem_unlink("server_semaphore");
		semaphore = sem_open("server_semaphore",O_CREAT,S_IRWXU,1);
		log_debug("Initiating server listening socket");
		if (semaphore == SEM_FAILED) { 
			log_error("sem_open failed");
    		exit(EXIT_FAILURE); 
  		} 
		int serverSocket;

		if((serverSocket = socket(PF_INET, SOCK_DGRAM,0)) < 0){
			log_error("Impossible to create server listening socket");
			exit(1);
		}
		struct sockaddr_in serv_addr;


		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		serv_addr.sin_port = htons(SERVER_PORT);

		if( bind(serverSocket,(struct sockaddr *)&serv_addr, sizeof(serv_addr) ) < 0 ){
			log_error("ERROR while binding server listening socket");
			//perror();
			exit(1);
		}
		log_debug("Server listening socket initialised");
		communicate(serverSocket);
	}
}

void log_info(char* message){
	if(SERVER_INFO){
		printf("\033[34mINFO : %s\033[39m\n",message);
	}
}
void log_debug(char* message){
	if(SERVER_DEBUG){
		printf("\033[32mDEBUG : %s\033[39m\n", message);
	}
}
void log_warning(char* message){
	if(SERVER_WARNING){
		printf("\033[33mWARNING : %s\033[39m\n",message);
	}
}
void log_error(char* message){
	if(SERVER_ERROR){
		printf("\033[31mERROR : %s\033[39m\n",message);
	}
}

REPORT initialise_report(){
	REPORT report = malloc(sizeof(*report));
	report->ips = malloc(sizeof(int)*MAX_FILES);
	report->paths = malloc(sizeof(char)*PATH_SIZE*MAX_FILES);
	report->nbFiles = 0;
	return(report);
}

void destroy_report(REPORT report){
	free(report->ips);
	for(int i=0;i<report->nbFiles;i++){
		free(*(report->paths + i));
	}
	free(report->paths);
	free(report);
}

void display_report(REPORT report){
	printf("Displaying report_file\n");
	for(int i=0;i<report->nbFiles;i++){
		printf("Path : \"%s\", IP : %d\n", *(report->paths + i), *(report->ips + i));
	}
}

REPORT import_report(){
	FILE *fptr;
    fptr = fopen("serverSaves/file_report.sav", "r");
	if(fptr != NULL){
		int caractere = 0;
		int cpt = 0;
		int cptFiles = 0;
		REPORT report = initialise_report();

		//On recupere le path
		int tst = 1;
		while(tst){
			caractere = 0;
			cpt = 0;
			*(report->paths + cptFiles) = malloc(sizeof(char)*PATH_SIZE);
			do{
				caractere = fgetc(fptr);
				if(caractere == ';'){
					break;
				}
				if(caractere == EOF){
					free(*(report->paths + cptFiles));
					log_error("Unexpected EOF Path");
					exit(1);
				}else if(caractere == '\n'){
					free(*(report->paths + cptFiles));
					log_error("Unexpected \n Path");
					exit(1);
				}else{
					*(*(report->paths + cptFiles) + cpt) = caractere;
					cpt++;
				}
			}while((caractere != ';') && (cpt < PATH_SIZE - 1));
			*(*(report->paths + cptFiles) + cpt) = '\0';
			caractere = 0;
			cpt = 0;
			char* ipBuff = malloc(sizeof(char)*32);
			do{
				caractere = fgetc(fptr);
				if(caractere == '\n'){
					break;
				}else if(caractere == EOF){
					tst = 0;
					break;
				}else{
					*(ipBuff + cpt) = caractere;
					cpt++;
				}
			}while((caractere != EOF) && (cpt < 31));
			*(ipBuff + cpt) = '\0';
			*(report->ips + cptFiles) = atoi(ipBuff);
			free(ipBuff);
			cptFiles++;
			if(!tst){
				report->nbFiles = cptFiles;
				break;
			}
		}
    	fclose(fptr);
    	return(report);
    }else{
    	log_debug("No report save found");
    	REPORT report = initialise_report();
    	return(report);
    }
}

int add_file(REPORT report, META meta, int ip){
	if(report->nbFiles == MAX_FILES){
		printf("Report is full : metadata not added\n");
		return(0);
	}else{
		*(report->ips + report->nbFiles) = ip;
		char path[PATH_SIZE] = "serverSaves/";
		strcat(path,meta->hash);
		strcat(path,".sav");
		*(report->paths + report->nbFiles) = malloc(sizeof(char)*PATH_SIZE);
		strcpy(*(report->paths + (report->nbFiles)),path);
		serialize_metadata(meta,path);
		report->nbFiles++;
		return(1);
	}
}

void serialize_report(REPORT report){
	FILE *fptr = NULL;
    fptr = fopen("serverSaves/file_report.sav", "w");
    if(fptr != NULL){
    	for(int i=0;i<report->nbFiles;i++){
    		if(i != 0){
    			fprintf(fptr,"\n");
    		}
    		fprintf(fptr,"%s;%d",*(report->paths + i),*(report->ips + i));
    	}
    	fclose(fptr);
    }else{
    	log_error("Impossible to open file_report.sav file");
    	exit(1);
    }
}

void serialize_report_without(REPORT report, int index){
	FILE *fptr = NULL;
    fptr = fopen("serverSaves/file_report.sav", "w");
    if(fptr != NULL){
    	int firstindex = 0;
    	for(int i=0;i<report->nbFiles;i++){
    		if(i == index){
    			if(!i){
    				firstindex = 1;
    			}
    			break;
    		}
    		if(i != firstindex){
    			fprintf(fptr,"\n");
    		}
    		fprintf(fptr,"%s;%d",*(report->paths + i),*(report->ips + i));
    	}
    	fclose(fptr);
    }else{
    	log_error("Impossible to open file_report.sav file");
    	exit(1);
    }
}

int is_hash_in_use(REPORT report, META meta){
	META currentMeta;
	for(int i=0;i<report->nbFiles;i++){
		currentMeta = initiate_meta();
		import_metadata(*(report->paths + i),currentMeta);
		
		if(strcmp(currentMeta->hash,meta->hash) == 0){
			destroy_meta(currentMeta);
			return(1);
		}
		destroy_meta(currentMeta);
	}
	return(0);
}

int search_candidates(REPORT report,char* keywords[],META meta_list[], int ip_list[], int nbKeys){
	int nbMeta = 0;
	META meta;
	int tst = 0;
	for(int i=0;i<report->nbFiles;i++){
		meta = initiate_meta();
		import_metadata(*(report->paths + i),meta);
		for(int j=0;j<nbKeys;j++){
			if(strcmp(meta->name,keywords[j]) == 0){
				meta_list[nbMeta] = initiate_meta();
				metacpy(meta_list[nbMeta],meta);
				ip_list[nbMeta] = *(report->ips + i);
				nbMeta++;
				break;
			}
			for(int k=0;k<meta->nbKeywords;k++){
				if(strcmp(*(meta->keywords + k),keywords[j]) == 0){
					meta_list[nbMeta] = initiate_meta();
					metacpy(meta_list[nbMeta],meta);
					ip_list[nbMeta] = *(report->ips + i);
					nbMeta++;
					tst = 1;
					break;
				}
			}
			if(tst){
				tst = 0;
				break;
			}
		}
		destroy_meta(meta);
	}
	return(nbMeta);
}

int delete_metadata(char* hash,int ip){
	REPORT report = import_report();
	META meta;
	for(int i=0;i<report->nbFiles;i++){
		meta = initiate_meta();
		import_metadata(*(report->paths + i),meta);
		if(strcmp(meta->hash,hash) == 0){
			if(*(report->ips + i) == ip){
				//Tout est bon : on delete
				serialize_report_without(report,i);
				destroy_meta(meta);
				destroy_report(report);
				return(0);
			}else{
				//Mauvaise IP
				destroy_meta(meta);
				destroy_report(report);
				return(2);
			}
		}
		destroy_meta(meta);
	}
	//Fichier n'existe pas
	destroy_report(report);
	return(1);
}

void INThandler(int sig){
	sem_close(semaphore);
	sem_unlink("server_semaphore");
	printf("\n");
	log_info("Closing server");
    exit(sig + 10);
}

void communicate(int serverSocket){
	signal(SIGINT,INThandler);
	while(1){
		int clilen;
		struct sockaddr_in cli_addr;

		clilen = sizeof(cli_addr);

		char buff[BUFFER_SIZE];
		log_info("Waiting for request");
		recvfrom(serverSocket,(void *)buff, BUFFER_SIZE*sizeof(char), 0, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen);
		if(strcmp(buff,"PUBLISH") == 0){
			printf("\033[34mINFO : ");
			printf("%s from ",buff);
			print_ip(cli_addr.sin_addr.s_addr);
			printf("\033[39m\n");
			sendto(serverSocket,"ACCEPTED", BUFFER_SIZE*sizeof(char), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
			publish(serverSocket,cli_addr,clilen);
		}else if(strcmp(buff,"SEARCH") == 0){
			printf("\033[34mINFO : ");
			printf("%s from ",buff);
			print_ip(cli_addr.sin_addr.s_addr);
			printf("\033[39m\n");
			sendto(serverSocket,"ACCEPTED", BUFFER_SIZE*sizeof(char), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
			search(serverSocket,cli_addr,clilen);
		}else if(strcmp(buff,"DELETE") == 0){
			printf("\033[34mINFO : ");
			printf("%s from ",buff);
			print_ip(cli_addr.sin_addr.s_addr);
			printf("\033[39m\n");
			sendto(serverSocket,"ACCEPTED", BUFFER_SIZE*sizeof(char), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
			deletion(serverSocket,cli_addr,clilen);
		}else{
			printf("\033[34mINFO : ");
			printf("Unknown request from ");
			print_ip(cli_addr.sin_addr.s_addr);
			printf(" : %s\033[39m\n",buff);
			sendto(serverSocket,(void *)"ERROR : unknown request", BUFFER_SIZE*sizeof(char), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
		}
	}
}

void publish(int serverSocket,struct sockaddr_in cli_addr, int clilen){
	int fo = fork();
	if(fo < 0){
		log_error("fork UDP didn't work");
		sem_close(semaphore);
		exit(1);
	}else if(fo > 0){
		return;
	}
	log_info("Begin PUBLISH");
	char log_buff[MAX_LOG_SIZE];

	log_debug("Generating new socket");
	int newSocket;

	if((newSocket = socket(PF_INET, SOCK_DGRAM,0)) < 0){
		sem_close(semaphore);
		log_error("impossible to create server communication socket");
		exit(1);
	}
	struct sockaddr_in serv_addr2;
	int cpt = 1;
	int has_port = 0;
	int port = 0;
	log_debug("Searching available port");
	while(port == 0 && (cpt <= NB_PORTS)){
		memset(&serv_addr2, 0, sizeof(serv_addr2));
		serv_addr2.sin_family = AF_INET;
		serv_addr2.sin_addr.s_addr = htonl(INADDR_ANY);
		serv_addr2.sin_port = htons(SERVER_PORT + cpt);
		if( bind(newSocket,(struct sockaddr *)&serv_addr2, sizeof(serv_addr2) ) < 0 ){
			sprintf(log_buff,"%d ALREADY BINDED",cpt + SERVER_PORT);
			log_debug(log_buff);
		}else{
			has_port = 1;
			port = SERVER_PORT + cpt;
			break;
		}
		cpt++;
	}
	
	if(!has_port){
		sendto(serverSocket,(void *)"PUBLISH IMPOSSIBLE : NO PORTS LEFT", BUFFER_SIZE*sizeof(char), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
		log_warning("No port left");
		sem_close(semaphore);
		exit(1);
	}else{
		sprintf(log_buff,"Changing to port %d",port);
		log_debug(log_buff);
		char port_string[BUFFER_SIZE];
		sprintf(port_string,"%d",port);
		sendto(serverSocket,(void *)port_string, strlen(port_string)*sizeof(char), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
	}
	log_debug("Client sent to new port");
	log_debug("Begin metadata reception");
	META meta = initiate_meta();
	char buff[BUFFER_SIZE];
	recvfrom(newSocket,(void *)meta->name, NAME_SIZE*sizeof(char), 0, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen);
	sprintf(log_buff, "Name received : %s", meta->name);
	log_debug(log_buff);
	int ip = cli_addr.sin_addr.s_addr;
	recvfrom(newSocket,(void *)meta->type, TYPE_SIZE*sizeof(char), 0, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen);
	sprintf(log_buff, "Type received : %s", meta->type);
	log_debug(log_buff);
	recvfrom(newSocket,(void *)meta->hash, HASH_SIZE*sizeof(char), 0, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen);
	sprintf(log_buff, "Hash received : %s", meta->hash);
	log_debug(log_buff);
	for(int i=0;i<MAX_KEYS;i++){
		recvfrom(newSocket,(void *)buff, KEY_SIZE*sizeof(char), 0, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen);
		if(strcmp(buff,"END") == 0){
			meta->nbKeywords = i;
			break;
		}
		strcpy(*(meta->keywords + i), buff);
		sprintf(log_buff,"Key %d received : %s",i + 1, *(meta->keywords + i));
		log_debug(log_buff);
	}
	log_debug("Metadata has been received");
	//Debut section critique
	log_debug("Semaphore activated");
	sem_wait(semaphore);
	log_debug("Importing report");
	REPORT report = import_report();
	log_debug("Report imported");
	if(is_hash_in_use(report,meta)){
		log_info("Impossible to publish : name already in use");
		sendto(newSocket,(void *)"PUBLISH IMPOSSIBLE : FILE ALREADY UPLOADED",BUFFER_SIZE*sizeof(char), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
		log_debug("Semaphore desactivated");
		sem_post(semaphore);
		log_debug("Cleaning heap and exit");
		sem_close(semaphore);
		destroy_meta(meta);
		destroy_report(report);
		close(newSocket);
		exit(0);
	}else if(add_file(report,meta,ip)== 0){
		log_warning("Impossible to publish : metadata list is full");
		sendto(newSocket,(void *)"PUBLISH IMPOSSIBLE : REPORT IS FULL",BUFFER_SIZE*sizeof(char), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
		sem_post(semaphore);
		log_debug("Semaphore desactivated");
		sem_close(semaphore);
		log_debug("Cleaning heap and exit");
		destroy_meta(meta);
		destroy_report(report);
		close(newSocket);
		exit(2);
	}else{
		log_debug("Publish correct");
		sendto(newSocket,(void *)"PUBLISH_ACK", BUFFER_SIZE*sizeof(char), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
	}

	if(SERVER_DEBUG){
		log_debug("Meta : ");
		printf("\033[35m");
		display_meta(meta);
		log_debug("Meta Report : ");
		printf("\033[36m");
		display_report(report);
	}
	log_debug("Serializing report");
	serialize_report(report);
	log_debug("Report Serialised");
	sem_post(semaphore);
	log_debug("Semaphore desactivated");
	//Fin section critique

	log_debug("Cleaning heap and exit");
	destroy_meta(meta);
	destroy_report(report);
	//sleep(60); pour tester le NO PORT LEFT avec NB_PORTS = 2
	close(newSocket);
	sem_close(semaphore);
	log_info("PUBLISH finished");
	exit(0);
}

void search(int serverSocket, struct sockaddr_in cli_addr, int clilen){
	int fo = fork();
	if(fo < 0){
		log_error("impossible to fork UDP");
		exit(1);
	}else if(fo > 0){
		return;
	}
	log_debug("Begin SEARCH");
	char log_buff[MAX_LOG_SIZE];

	int newSocket;

	if((newSocket = socket(PF_INET, SOCK_DGRAM,0)) < 0){
		log_error("ERROR while creating server communication socket");
		exit(1);
	}
	struct sockaddr_in serv_addr2;
	int cpt = 1;
	int has_port = 0;
	int port = 0;
	while(port == 0 && (cpt <= NB_PORTS)){
		memset(&serv_addr2, 0, sizeof(serv_addr2));
		serv_addr2.sin_family = AF_INET;
		serv_addr2.sin_addr.s_addr = htonl(INADDR_ANY);
		serv_addr2.sin_port = htons(SERVER_PORT + cpt);
		if( bind(newSocket,(struct sockaddr *)&serv_addr2, sizeof(serv_addr2) ) < 0 ){
			sprintf(log_buff,"%d ALREADY BINDED",cpt + SERVER_PORT);
			log_debug(log_buff);
		}else{
			has_port = 1;
			port = SERVER_PORT + cpt;
			break;
		}
		cpt++;
	}
	if(!has_port){
		sendto(serverSocket,(void *)"SEARCH IMPOSSIBLE : NO PORTS LEFT", BUFFER_SIZE*sizeof(char), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
		log_warning("No port left");
		exit(1);
	}else{
		char port_string[BUFFER_SIZE];
		sprintf(port_string,"%d",port);
		sendto(serverSocket,(void *)port_string, strlen(port_string)*sizeof(char), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
	}


	char* keywords[MAX_SEARCH_KEYS];
	char key[KEY_SIZE];
	int nbKeys = 0;

	log_debug("Receiving keywords");
	for(int i=0;i<MAX_SEARCH_KEYS;i++){
		recvfrom(newSocket,(void *)key, KEY_SIZE*sizeof(char), 0, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen);
		if(strcmp(key,"END_KEYS") == 0){
			nbKeys = i;
			break;
		}
		keywords[i] = malloc(sizeof(char)*KEY_SIZE);
		strcpy(keywords[i], key);
		nbKeys++;
	}

	META meta_list[MAX_RETURN_META];
	int ip_list[MAX_RETURN_META];

	//Debut section critique
	
	for(int i=0;i<nbKeys;i++){
		sprintf(log_buff,"Keyword : %s",keywords[i]);
		log_debug(log_buff);
	}
	log_debug("Activationg semaphore");
	sem_wait(semaphore);
	log_debug("Importing report");
	REPORT report = import_report();
	log_debug("Searching candidates");
	int nbMeta = search_candidates(report, keywords, meta_list, ip_list, nbKeys);
	sem_post(semaphore);
	log_debug("Releasing semaphore");
	//Fin section critique

	if(nbMeta == 0){
		log_info("No answers");
	}else{
		sprintf(log_buff,"Found %d answers",nbMeta);
		log_info(log_buff);
	}
	for(int i=0;i<nbMeta;i++){
		sprintf(log_buff,"%s from %d ip",meta_list[i]->name,ip_list[i]);
		log_debug(log_buff);
		log_debug("Displaying metadata :");
		if(SERVER_DEBUG){
			display_meta(meta_list[i]);
		}
	}

	for(int i=0;i<nbKeys;i++){
		free(keywords[i]);
	}

	//RENVOYER la SEARCH_RESP
	sprintf(log_buff,"Sending %d metadatas :",nbMeta);
	log_debug(log_buff);
	sendto(newSocket,(void *)"SEARCH_RESP", BUFFER_SIZE*sizeof(char), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
	for(int i=0;i<nbMeta;i++){
		sleep(0.1);
		sendto(newSocket,(void *) meta_list[i]->name, (strlen(meta_list[i]->name) + 1)*sizeof(char), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
		sleep(0.1);
		sendto(newSocket,(void *) meta_list[i]->type, strlen(meta_list[i]->type)*sizeof(char), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
		sleep(0.1);
		sendto(newSocket,(void *) meta_list[i]->hash, strlen(meta_list[i]->hash)*sizeof(char), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
		sleep(0.1);
		char ip[BUFFER_SIZE];
		sprintf(ip,"%d",ip_list[i]);
		sendto(newSocket,(void *) ip, strlen(ip)*sizeof(char), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
		for(int j=0;j<meta_list[i]->nbKeywords;j++){
			sleep(0.1);
			sendto(newSocket,(void *) *(meta_list[i]->keywords + j), (strlen(*(meta_list[i]->keywords + j)) + 1)*sizeof(char), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
		}
		sleep(0.1);
		sendto(newSocket,(void *)"END", BUFFER_SIZE*sizeof(char), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
		sprintf(log_buff,"%s.%s sent",meta_list[i]->name,meta_list[i]->type);
		log_debug(log_buff);
	}
	sendto(newSocket,(void *)"END_RESP", BUFFER_SIZE*sizeof(char), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
	log_debug("All metadata sent");


	log_debug("Cleaning heap and exit");
	close(newSocket);
	sem_close(semaphore);
	
	for(int i=0;i<nbMeta;i++){
		destroy_meta(meta_list[i]);
	}
	destroy_report(report);

	log_info("SEARCH finished");
	exit(0);
}

void deletion(int serverSocket, struct sockaddr_in cli_addr, int clilen){
	int fo = fork();
	if(fo < 0){
		log_error("impossible to fork UDP");
		exit(1);
	}else if(fo > 0){
		return;
	}
	log_debug("Begin DELETE");
	char log_buff[MAX_LOG_SIZE];

	int newSocket;

	if((newSocket = socket(PF_INET, SOCK_DGRAM,0)) < 0){
		log_error("ERROR while creating server communication socket");
		exit(1);
	}
	struct sockaddr_in serv_addr2;
	int cpt = 1;
	int has_port = 0;
	int port = 0;
	while(port == 0 && (cpt <= NB_PORTS)){
		memset(&serv_addr2, 0, sizeof(serv_addr2));
		serv_addr2.sin_family = AF_INET;
		serv_addr2.sin_addr.s_addr = htonl(INADDR_ANY);
		serv_addr2.sin_port = htons(SERVER_PORT + cpt);
		if( bind(newSocket,(struct sockaddr *)&serv_addr2, sizeof(serv_addr2) ) < 0 ){
			sprintf(log_buff,"%d ALREADY BINDED",cpt + SERVER_PORT);
			log_debug(log_buff);
		}else{
			has_port = 1;
			port = SERVER_PORT + cpt;
			break;
		}
		cpt++;
	}
	if(!has_port){
		sendto(serverSocket,(void *)"DELETE IMPOSSIBLE : NO PORTS LEFT", BUFFER_SIZE*sizeof(char), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
		log_warning("No port left");
		exit(1);
	}else{
		char port_string[BUFFER_SIZE];
		sprintf(port_string,"%d",port);
		sendto(serverSocket,(void *)port_string, strlen(port_string)*sizeof(char), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
	}
	log_debug("Client sent to new port");
	log_debug("Begin hash reception");
	char hash[HASH_SIZE];
	recvfrom(newSocket,(void *)hash, HASH_SIZE*sizeof(char), 0, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen);


	//Debut section critique
	log_debug("Activating semaphore");
	sem_wait(semaphore);
	int tstDelete = delete_metadata(hash,cli_addr.sin_addr.s_addr);
	sem_post(semaphore);
	log_debug("Semaphore desactivated");
	//Fin section critique

	if(tstDelete == 0){
		//Metadata deleted
		log_info("Metadata successfully deleted");
		sendto(newSocket,(void *)"DELETE_ACK", BUFFER_SIZE*sizeof(char), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
	}else if(tstDelete == 1){
		//Doesn't exist
		log_info("Impossible to delete metadata : it doesn't exist");
		sendto(newSocket,(void *)"DELETE IMPOSSIBLE : THIS FILE DOESN'T EXIST", BUFFER_SIZE*sizeof(char), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
	}else{
		//Not good IP
		log_info("Impossible to delete metadata : the person doesn't own the file");
		sendto(newSocket,(void *)"DELETE IMPOSSIBLE : THIS FILE BELONGS TO SOMEONE ELSE", BUFFER_SIZE*sizeof(char), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
	}


	log_debug("Cleaning heap and exit");
	sem_close(semaphore);
	exit(0);
}