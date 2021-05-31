#include "client.h"
#include "MyP2P_util.h"

int nbTry = 0;
META to_destroy_meta = NULL;
char* to_destroy_path = NULL;

void log_info(char* message){
	if(INFO){
		printf("\033[34mINFO : %s\033[39m\n",message);
	}
}
void log_debug(char* message){
	if(DEBUG){
		printf("\033[32mDEBUG : %s\033[39m\n", message);
	}
}
void log_warning(char* message){
	if(WARNING){
		printf("\033[33mWARNING : %s\033[39m\n",message);
	}
}
void log_error(char* message){
	if(ERROR){
		printf("\033[31mERROR : %s\033[39m\n",message);
	}
}


void init_dir(){
	
	/*
		Teste si les dossiers nécéssaires à l'utilisation de l'application en tant que client existent, sinon les crée.
	*/

	char *homedir;
	homedir = getenv("HOME");
	
	char* path = malloc(sizeof(char)*(strlen(homedir)+20));
	strcpy(path,homedir);
	DIR* dir = opendir(strcat(path,"/my_p2p/ava_files"));
	free(path);
	if (dir) {
		//existe
		closedir(dir); //Rien à faire
		if(DEBUG){
			printf("DEBUG init_dir : Directories already exist \n");
		}
	} else if (ENOENT == errno) {
		if (fork() == 0){ execlp("sh", "sh", "-c", "mkdir -p ~/my_p2p/ava_files",NULL);}
		if(DEBUG){
			printf("Required directories created\n");
		}
	} else {
		perror("Erreur test existence du dossier my_p2p\n");
		exit(1);
	}
}

void download(char* name, char* ip, char* dst){
	
	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	//établir connexion
	struct sockaddr_in serv_addr;
	bzero( (char*) &serv_addr, sizeof(serv_addr) );
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons((ushort) USER_PORT_DL);
	serv_addr.sin_addr.s_addr = inet_addr (ip);
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr) ) < 0){
		perror ("cliecho : erreur connect");
		exit (1);
	}
	
	send(sockfd, "GET\r\n", 6*sizeof(char),0);
	
	char* buff = malloc(BUFFER_SIZE*sizeof(char));
	memset(buff,0,BUFFER_SIZE*sizeof(char));
	recv(sockfd, buff, BUFFER_SIZE*sizeof(char),0);
	
	printf("[%s]\n",buff);
	
	FILE* file = fopen(dst,"wb+"); 
	
	if (strncmp(buff,"OK",2)){
		printf("Erreur header");
		free(buff);
		fclose(file);
		return;
	} else { 
		send(sockfd, name, strlen(name),0);
		for( ;; ){
			int number_received = recv(sockfd, buff, BUFFER_SIZE*sizeof(char),0);
			printf("%d\n",number_received);
			fwrite(buff, 1 , number_received, file);
			if ( number_received < BUFFER_SIZE ) {
				break;
			}

		}
		char* message = "OK\n";
		send(sockfd, message,4*sizeof(char),0);
	}
	free(buff);
	fclose(file);
}

int establish(int serverSocket, struct sockaddr* cli_addr);
void traiter(int socket);

void set_listening(){
	/*
		Lance l'écoute pour permettre le téléchargement de fichiers
	*/

		int listening_sock = socket(AF_INET,SOCK_STREAM,0);
		
		struct sockaddr_in  serv_addr;
		//initiliasation du sockaddr
		memset (&serv_addr, 0, sizeof(serv_addr) );
		serv_addr.sin_family = AF_INET ;
		serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		serv_addr.sin_port = htons(USER_PORT_DL);
		
		//exécution de bind
		if(bind(listening_sock,(struct sockaddr *)&serv_addr, sizeof (serv_addr) ) <0) {
			perror ("servecho: erreur bind\n");
			exit (1);
		}
		
		//lancement du service
		if(listen(listening_sock, MAX_CONN) <0) {
			perror ("servecho: erreur listen\n");
			exit (1);
		}
		
		//Ici on accepte une requête
		struct sockaddr_in cli_addr;
		int dialogSocket;
		
		for(;;){
			dialogSocket = establish(listening_sock,(struct sockaddr*) &cli_addr);
			switch ( fork()) {
				case -1 : perror("probleme fork");
				exit(1);
				case 0 : 
				close (listening_sock); /* fils */
				traiter(dialogSocket);
				close (dialogSocket);
				exit(0);
				default : close(dialogSocket); /* pere*/
			} 
		}
		printf("End of listening\n");
		return;
}

int establish(int serverSocket, struct sockaddr* cli_addr){
	int clilen = sizeof(*cli_addr);
	int dialogSocket = accept(serverSocket, cli_addr, (socklen_t *)&clilen);
	if(dialogSocket< 0) {
		perror("servecho : erreur accept\n");
		exit (1);
	}
	return dialogSocket;
}

void upload(int socket){
	//réception du nom du fichier
	char* buff = malloc(BUFFER_SIZE*sizeof(char));
	memset(buff,0,BUFFER_SIZE*sizeof(char));
	recv(socket, buff, BUFFER_SIZE*sizeof(char),0);
	
	//buff[strlen(buff)-2] = 0; //Si usage avec telnet
	printf("DEBUG DL : |%s|\n",buff);
	
	//On vérifie que le nom du fichier n'est pas malicieux
	for(int i = 0;i <(int)strlen(buff);i++){
		if (buff[i] == '/'){
			char* message = "Invalid file\r\n";
			send(socket, message,15*sizeof(char),0);
			return;
		}
	}
	
	//On récupère le fichier
	char *homedir;
	homedir = getenv("HOME");
	char* path = malloc(sizeof(char)*(BUFFER_SIZE+strlen(homedir)+20));
	strcpy(path,homedir);
	strcat(path,"/my_p2p/ava_files/");
	strcat(path,buff);
	
	printf("DEBUG DL : [%s]\n",path);
	
	FILE* file = fopen(path,"rb");
	
	if (file == NULL){
		char* message = "name not found\r\n";
		send(socket, message,17*sizeof(char),0);
		return;
	}
	
	/*
		Remplacer par le téléchargement
	*/
	int c = 0;
	while(1){
		int nb_carac = fread(buff,sizeof(char),BUFFER_SIZE,file); c++;
		send(socket, buff , nb_carac*sizeof(char), 0);
		if (feof(file)){
			break;
		}
	}
	recv(socket, buff, BUFFER_SIZE*sizeof(char),0);
	if(strncmp(buff,"OK\n",4)){
		printf("PB TRANSMISSION\n");
	}
	char* message = "OK\n";
	send(socket, message,4*sizeof(char),0);
	return;
	
}

void traiter(int socket){
	char* buff = malloc(BUFFER_SIZE*sizeof(char));
	memset(buff,0,BUFFER_SIZE*sizeof(char));
	recv(socket, buff, BUFFER_SIZE*sizeof(char),0);
	if(strcmp("GET\r\n",buff)){ // \r\n pour que ça marche avec mon telnet
		char* message = "Invalid header\r\n";
		send(socket, message,16*sizeof(char),0);
	} else {
		char* message = "OK\n";
		send(socket, message,4*sizeof(char),0);
		upload(socket);
	}
	free(buff);
}

void alarm_handler(int signum){
	if(nbTry >= NB_TRY){
		printf("SERVER UNREACHABLE\n");
		if(to_destroy_meta != NULL){
			destroy_meta(to_destroy_meta);
		}
		if(to_destroy_path != NULL){
			free(to_destroy_path);
		}
 		exit(signum + 10);
	}else{
		printf("NO RESPONSE, TRYING %d\n",++nbTry );
		alarm(TRY_GAP);
	}
}

void publish(int serverSocket,struct sockaddr_in serv_addr){
	char log_buff[MAX_LOG_SIZE];
	char log_buff_2[BUFFER_SIZE + MAX_LOG_SIZE + 35];
	char* path = malloc(sizeof(char)*NAME_SIZE);
	log_info("Getting metadata from user");
	META meta = get_meta_from_file(path);
	if(meta == NULL){
		log_error("Couldn't generate metadata");
		free(path);
		exit(1);
	}
	log_info("Begin PUBLISH");
	to_destroy_meta = meta;
	to_destroy_path = path;
	signal(SIGALRM, alarm_handler);

	int servlen = 0;

	char pub[BUFFER_SIZE] = "PUBLISH";
	char buff[BUFFER_SIZE];

	time_t begin = time( NULL );

    time_t end; 

    log_debug("Trying to connect to server");
	do{
		sleep(0.1);
		alarm(TRY_GAP);
		end = time( NULL);
		if((end - begin) > NB_TRY){
			log_warning("Publish impossible, server reachable but full");
			//printf("PUBLISH IMPOSSIBLE : SERVER REACHABLE BUT FULL \n");
			free(path);
			destroy_meta(meta);
			alarm(0);
			return;
		}
		sendto(serverSocket,(void *)pub, BUFFER_SIZE*sizeof(char), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
		recvfrom(serverSocket,(void *)buff, BUFFER_SIZE*sizeof(char), 0, (struct sockaddr *)&serv_addr,(socklen_t *) &servlen);
	}while(strcmp(buff,"ACCEPTED") != 0);
	alarm(0);
	log_debug("Connexion made");
	nbTry = 0;

	log_debug("Looking for new port");
	recvfrom(serverSocket,(void *)buff, BUFFER_SIZE*sizeof(char), 0, (struct sockaddr *)&serv_addr,(socklen_t *) &servlen);
	int port = atoi(buff);
	if(port == 0){
		sprintf(log_buff_2,"Unvalid port : %s",buff);
		log_error(log_buff);
		log_debug("Cleaning heap and exit");
		destroy_meta(meta);
		exit(1);
	}

	close(serverSocket);

	int newSocket;

	if((newSocket = socket(PF_INET, SOCK_DGRAM,0)) < 0){
		log_error("Impossible to create server communication socket");
		exit(1);
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = PF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);

	log_debug("Sending metadata");
	sendto(newSocket,(void *)meta->name,strlen(meta->name) *sizeof(char), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	sleep(0.1);
	sendto(newSocket,(void *)meta->type,strlen(meta->type)*sizeof(char), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	sleep(0.1);
	sendto(newSocket,(void *)meta->hash,strlen(meta->hash) *sizeof(char), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	for(int i=0;i<meta->nbKeywords;i++){
		sleep(0.1);
		sendto(newSocket,(void *)*(meta->keywords + i),(strlen(*(meta->keywords + i)) + 1) *sizeof(char), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	}
	sleep(0.1);
	sendto(newSocket,(void *)"END", 4*sizeof(char), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	log_debug("Metadata sent");

	log_debug("Waiting for ACK"); 
	recvfrom(newSocket,(void *)buff, BUFFER_SIZE*sizeof(char), 0, (struct sockaddr *)&serv_addr,(socklen_t *) &servlen);
	if(strcmp(buff,"PUBLISH_ACK") == 0){
		log_info("The metadata have been successfully uploaded");
		printf("The metadata have been successfully uploaded\n");
		copy_file(path,meta);
	}else{
		sprintf(log_buff_2,"Impossible to upload metadata\nAnswer : \"%s\"",buff);
		log_warning(log_buff_2);
	}
	log_debug("Cleaning heap and exit");
	close(newSocket);
	destroy_meta(meta);
	free(path);
	log_info("End PUBLISH");
}

int search(int serverSocket, struct sockaddr_in serv_addr){
	signal(SIGALRM, alarm_handler);
	char* keywords[MAX_SEARCH_KEYS];
	int nbKeys = get_keywords(keywords);
	int servlen = sizeof(serv_addr);

	char* sea = "SEARCH";

	char buff[BUFFER_SIZE];

	time_t begin = time( NULL );
    
    time_t end; 
    printf("Votre recherche :");
    for(int i=0;i<nbKeys;i++){
    	if(i!=0){
    		printf(";");
    	}
    	printf(" %s",keywords[i] );
    }
    printf("\n");

	do{
		sleep(0.1);
		alarm(TRY_GAP);
		end = time( NULL);
		if((end - begin) > NB_TRY){
			printf("SEARCH IMPOSSIBLE : SERVER REACHABLE BUT FULL \n");
			alarm(0);
			return(3);
		}
		sendto(serverSocket,(void *)sea, BUFFER_SIZE*sizeof(char), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
		recvfrom(serverSocket,(void *)buff, BUFFER_SIZE*sizeof(char), 0, (struct sockaddr *)&serv_addr,(socklen_t *) &servlen);
	}while(strcmp(buff,"ACCEPTED") != 0);
	alarm(0);
	nbTry = 0;

	recvfrom(serverSocket,(void *)buff, BUFFER_SIZE*sizeof(char), 0, (struct sockaddr *)&serv_addr,(socklen_t *) &servlen);
	int port = atoi(buff);
	if(port == 0){
		printf("ERROR : %s\n", buff);
		return(2);
	}
	close(serverSocket);

	int newSocket;

	if((newSocket = socket(PF_INET, SOCK_DGRAM,0)) < 0){
		perror("ERROR while creating client socket\n");
		return(2);
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = PF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);
	
	for(int i=0;i<nbKeys;i++){
		sendto(newSocket,(void *)keywords[i], (strlen(keywords[i]) + 1)*sizeof(char), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
		sleep(0.1);
	}
	sendto(newSocket,(void *)"END_KEYS", KEY_SIZE*sizeof(char), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

	for(int i=0;i<nbKeys;i++){
		free(keywords[i]);
	}


	//On récupère les meta-données de SEARCH_RESP
	recvfrom(newSocket,(void *)buff, BUFFER_SIZE*sizeof(char), 0, (struct sockaddr *)&serv_addr,(socklen_t *) &servlen);
	if(strcmp(buff,"SEARCH_RESP") != 0){
		printf("Mesage : %s\n", buff);
		close(newSocket);
		return(1);
	}

	int cptMeta = 0;
	META meta_list[MAX_RETURN_META];
	int ip_list[MAX_RETURN_META];
	META meta;
	//int cpt
	for(int i=0;i<MAX_RETURN_META;i++){
		meta = initiate_meta();
		recvfrom(newSocket,(void *)meta->name, NAME_SIZE*sizeof(char), 0, (struct sockaddr *)&serv_addr,(socklen_t *) &servlen);
		if(strcmp(meta->name,"END_RESP") == 0){
			destroy_meta(meta);
			break;
		}
		recvfrom(newSocket,(void *)meta->type, TYPE_SIZE*sizeof(char), 0, (struct sockaddr *)&serv_addr,(socklen_t *) &servlen);
		recvfrom(newSocket,(void *)meta->hash, HASH_SIZE*sizeof(char), 0, (struct sockaddr *)&serv_addr,(socklen_t *) &servlen);
		char buff[BUFFER_SIZE];
		recvfrom(newSocket,(void *)buff, BUFFER_SIZE*sizeof(char), 0, (struct sockaddr *)&serv_addr,(socklen_t *) &servlen);
		ip_list[i] = atoi(buff);
		char key[KEY_SIZE];
		for(int j=0;j<MAX_KEYS;j++){
			recvfrom(newSocket,(void *)key, KEY_SIZE*sizeof(char), 0, (struct sockaddr *)&serv_addr,(socklen_t *) &servlen);
			if(strcmp(key,"END") == 0){
				meta->nbKeywords = j;
				break;
			}
			meta->nbKeywords = j;
			strcpy(*(meta->keywords + j),key);
		}
		meta_list[i] = initiate_meta();
		metacpy(meta_list[i],meta);
		//strcpy(meta->name,buff);
		destroy_meta(meta);
		cptMeta++;
	}
	close(newSocket);
	
	//recvfrom(newSocket,(void *)buff, BUFFER_SIZE*sizeof(char), 0, (struct sockaddr *)&serv_addr,(socklen_t *) &servlen);



	
	//Affichage + selection de la donnée qui nous intéresse

	if(cptMeta == 0){
		printf("Aucune réponse n'a été trouvée à votre recherche\n");
		while(1){
			printf("Voulez vous en refaire une autre ? (y/n) ");
			char* res = malloc(sizeof(char)*8);
			//clear_stdin();
			if(fgets(res,8,stdin) == NULL){
				printf("ERROR getting confirmation from stdin\n");
				exit(1);
			}
			if(*res == 'n'){
				//tst = 0;
				free(res);
				return(0);
			}else if(*res == 'y'){
				free(res);
				return(5);
			}
			free(res);
			printf("\n");
		}
	}
	printf("%d réponses ont été trouvées\n", cptMeta);
	for (int i = 0; i < cptMeta; i++){
		printf("Réponse %d : (IP = ",i + 1);
		print_ip(ip_list[i]);
		printf(")\n");
		display_meta(meta_list[i]);
	}

	printf("Voulez-vous télécharger un de ces fichiers ?\nEntrez le numéro de la réponse correspondante (0 ou tout autre entrée sinon) ");
	char* resp_number = malloc(sizeof(char)*3);
	fgets(resp_number,3,stdin);
	int num = atoi(resp_number);
	free(resp_number);
	if(num  && (num <= cptMeta)){
		char real_ip[16];
		unsigned char bytes[4];
    	bytes[0] = ip_list[num-1] & 0xFF;
    	bytes[1] = (ip_list[num-1] >> 8) & 0xFF;
    	bytes[2] = (ip_list[num-1] >> 16) & 0xFF;
    	bytes[3] = (ip_list[num-1] >> 24) & 0xFF;   
    	sprintf(real_ip,"%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], bytes[3]);  
		printf("Téléchargement du fichier %s\n",meta_list[num-1]->name);
		download(meta_list[num-1]->hash,real_ip,meta_list[num-1]->name);
		// Vérifier que sha1(new_file) == meta->hash
	}else{
		printf("Pas de téléchargement\n");
	}
	for(int i=0;i<cptMeta;i++){
		destroy_meta(meta_list[i]);
	}
	return(0);
}

