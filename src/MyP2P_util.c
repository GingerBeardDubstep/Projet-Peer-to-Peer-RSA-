#include "MyP2P_util.h"

void serialize_metadata(META meta, char* path){
	FILE *fptr = NULL;
    fptr = fopen(path, "w");
    if(fptr != NULL){
    	fprintf(fptr, "%s\n%s\n%s\n",meta->name,meta->type,meta->hash);

    	for(int i=0;i<meta->nbKeywords;i++){
    		if(*(meta->keywords + i) == NULL || *(meta->keywords + i) == 0){
    			break;
    		}
    		if(i != 0){
    			fprintf(fptr,";");
    		}
    		fprintf(fptr,"%s",*(meta->keywords + i));
    	}
    	fclose(fptr);
    }else{
    	printf("\033[31mERROR : ");
    	printf("Impossible to open file %s\n",path);
    	printf("\033[39m");
    	//printf("ERROR : Impossible to open file %s\n",path);
    	exit(1);
    }
}

void print_ip(unsigned int ip){
    unsigned char bytes[4];
    bytes[0] = ip & 0xFF;
    bytes[1] = (ip >> 8) & 0xFF;
    bytes[2] = (ip >> 16) & 0xFF;
    bytes[3] = (ip >> 24) & 0xFF;   
    printf("%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], bytes[3]);        
}

void import_metadata(char* path, META meta){
	FILE *fptr;
    fptr = fopen(path, "r");
	if(fptr != NULL){
		int caractere = 0;
		int cpt = 0;

		//On recupere le nom
		do{
			caractere = fgetc(fptr);
			if(caractere == '\n'){
				break;
			}
			if(caractere == EOF){
				printf("ERROR : unexpected EOF Name\n");
				exit(1);
			}else{

			}
			*(meta->name + cpt) = caractere;
			cpt++;
		}while(caractere != '\n');
		*(meta->name + cpt) = '\0';

		//On récupère le type
		caractere = 0;
		cpt = 0;
		do{
			caractere = fgetc(fptr);
			if(caractere == '\n'){
				break;
			}
			if(caractere == EOF){
				printf("ERROR : unexpected EOF : read Type\n");
				exit(1);
			}else{

			}
			*(meta->type + cpt) = caractere;
			cpt++;
		}while(caractere != '\n');
		*(meta->type + cpt) = '\0';

		//On recupere le hash
		caractere = 0;
		cpt = 0;
		do{
			caractere = fgetc(fptr);
			if(caractere == '\n'){
				break;
			}
			if(caractere == EOF){
				printf("ERROR : unexpected EOF : read Hash\n");
				exit(1);
			}else{

			}
			*(meta->hash + cpt) = caractere;
			cpt++;
		}while(caractere != '\n');
		*(meta->hash + cpt) = '\0';

		//On récupère les keywords
		caractere = 0;
		cpt = 0;
		int cptKey = 0;
		do{
			caractere = fgetc(fptr);
			if(caractere == '\n'){
				printf("ERROR : unexpected \\n : read Keywords\n");
				exit(1);
			}
			if(caractere == EOF){
				break;
			}
			if(caractere == ';'){
				*(*(meta->keywords + cptKey) + cpt) = '\0';
				cptKey++;
				cpt = 0;
				continue;
			}
			*(*(meta->keywords + cptKey) + cpt) = caractere;
			cpt++;
		}while(caractere != EOF);
		*(*(meta->keywords + cptKey) + cpt) = '\0';
		meta->nbKeywords = cptKey+1;
    	fclose(fptr);
    }else{
    	printf("\033[31mERROR : ");
    	printf("Impossible to open file %s\n",path);
    	printf("\033[39m\n");
    	exit(1);
    }
}

META initiate_meta(){
	META meta = NULL;
    meta = malloc(sizeof(*meta));
    meta->name = malloc(sizeof(char)*NAME_SIZE);
    meta->type = malloc(sizeof(char)*TYPE_SIZE);
    meta->hash = malloc(sizeof(char)*HASH_SIZE);
    meta->keywords = NULL;
    meta->keywords = malloc(sizeof(char)*(KEY_SIZE)*MAX_KEYS);
    for(int i=0;i<MAX_KEYS;i++){
    	*(meta->keywords + i) = malloc(sizeof(char)*KEY_SIZE);
    }
    meta->nbKeywords = 0;
    return(meta);
}

void destroy_meta(META meta){
	free(meta->name);
	for(int i=0;i<MAX_KEYS;i++){
		if(*(meta->keywords + i) == NULL || *(meta->keywords + i) == 0){
			break;
		}
    	free(*(meta->keywords + i));
    }
	free(meta->keywords);
    free(meta->hash);
    free(meta->type);
   	
	free(meta);
}

/*int main(int argc, char** argv){
	//serialize_test();
	import_test();
}*/

void display_meta(META meta){
	printf("Nom : %s\nType : %s\nHash : %s\n",meta->name,meta->type,meta->hash);
	printf("Mots clefs : ");
	for(int i=0;i<meta->nbKeywords;i++){
		if(i!=0){
			printf("; ");
		}
		printf("%s",*(meta->keywords + i));
	}
	printf("\n");
}

void import_test(){
	META meta = NULL;
	meta = initiate_meta();
	import_metadata("../saves/Makefile.sav",meta);
	display_meta(meta);
	destroy_meta(meta);
}

void metacpy(META dst, META src){
	strcpy(dst->name,src->name);
	strcpy(dst->type,src->type);
	strcpy(dst->hash,src->hash);
	for(int i=0;i<src->nbKeywords;i++){
		strcpy(*(dst->keywords + i),*(src->keywords + i));
	}
	dst->nbKeywords = src->nbKeywords;
}

/*void clear_stdin(){
	int c;
	while ((c = getchar()) != EOF) { }
}*/

int get_keywords(char* keywords[]){
	printf("Rentrez les mots clefs de votre recherche : \n");
	int tst = 1;
	int cpt = 0;
	while(tst){
		if(cpt == MAX_SEARCH_KEYS){
			printf("Nombre maximum de mots clefs atteint : %d\n", MAX_SEARCH_KEYS);
			break;
		}

		if(cpt != 0){
			while(1){
				printf("Voulez vous ajouter un mot clef ? (y/n) ");
				char* res = malloc(sizeof(char)*8);
				//clear_stdin();
				if(fgets(res,8,stdin) == NULL){
					perror("ERROR getting confirmation from stdin\n");
					
					exit(1);
				}
				if(*res == 'n'){
					tst = 0;
					free(res);
					break;
				}else if(*res == 'y'){
					free(res);
					break;
				}
				free(res);
			}
			if(!tst){
				break;
			}
		}
		printf("Entrez le mot clef : ");
		//*(meta->keywords + KEY_SIZE*cpt)
		//clear_stdin();
		keywords[cpt] = malloc(sizeof(char)*KEY_SIZE);
		if(fgets(keywords[cpt], KEY_SIZE, stdin) == NULL){
			perror("ERROR getting key from stdin\n");
			exit(1);
		}
		*(keywords[cpt] + strcspn(keywords[cpt], "\n")) = '\0';
		//*(*(meta->keywords + KEY_SIZE*cpt) + strcspn(*(meta->keywords + KEY_SIZE*cpt), "\n")) = '\0';
		cpt++;
	}
	return(cpt);
}

void copy_file(char* path, META meta){
	int tst_fork = fork();
	if(tst_fork < 0){
		perror("Erreur fork\n");
		exit(1);
	}else if(tst_fork == 0){
		char *homedir;
		homedir = getenv("HOME");
		char comd_fork[BUFFER_SIZE] = "cp ";
		strcat(comd_fork,path);
		strcat(comd_fork," ");
		strcat(comd_fork,homedir);
		strcat(comd_fork,"/my_p2p/ava_files/");
		strcat(comd_fork,meta->hash);
		free(path);
		destroy_meta(meta);
		execlp("sh" ,"sh","-c",comd_fork,NULL); 
		exit(0);
	}
}

META get_meta_from_file(char* path){
	META meta = NULL;
	meta = initiate_meta();
	if(meta == NULL){
		perror("ERROR initiating metadata\n");
		exit(1);
	}

	printf("Entrez le chemin du fichier : ");
	if(fgets(path, NAME_SIZE, stdin) == NULL){
		perror("ERROR getting name from stdin\n");
		exit(1);
	}
	*(path + strcspn(path,"\n")) = '\0';
	int cpt = 0;
	int cptBuff = 0;
	for(int i=0;i<(int)strlen(path);i++){
		if(*(path+cpt) == '/'){
			cptBuff = 0;
			cpt++;
			continue;
		}else{
			*(meta->name + cptBuff) = *(path+cpt);
			cpt++;
			cptBuff++;
		}
	}
	*(meta->name + cptBuff) = '\0';

	//On récupère le sha1
	int entree[2];
	int sortie[2];
	pipe(entree);
	pipe(sortie);
	pid_t pid = fork();
	if(pid == -1){
		perror("Erreur fork : exit 1\n");
		exit(1);
	}else if(pid == 0){
		close(entree[1]);
		close(sortie[0]);
		close(entree[0]);
		dup2(sortie[1],1); // On duplique la sortie du tube avec stdout
		close(sortie[1]);
		char comd[NAME_SIZE] = "shasum ";
		strcat(comd,path);
		execlp("sh" ,"sh","-c",comd,NULL); 
		exit(0);
	}else{
		close(entree[0]);
		close(entree[1]);
		close(sortie[1]);
		int status;
		waitpid(-1,&status,0);
		int ret;
		do{
			ret = read(sortie[0],meta->hash,HASH_SIZE);
			assert(ret != -1);
		}while(ret != 0);
		*(meta->hash + strcspn(meta->hash," ")) = '\0';
		close(sortie[0]);
	}
	if(meta->hash == NULL || strcmp(meta->hash,"") == 0){
		printf("\033[33m");
		printf("WARNING : couldn't generate sha1 digest : the file %s doesn't exist\033[39m\n", path);
		return(NULL);
	}

	//On récupère le type
	int entree_type[2];
	int sortie_type[2];
	pipe(entree_type);
	pipe(sortie_type);
	pid_t pid_type = fork();
	if(pid_type == -1){
		perror("Erreur fork : exit 1\n");
		exit(1);
	}else if(pid_type == 0){
		close(entree_type[1]);
		close(sortie_type[0]);
		close(entree_type[0]);
		dup2(sortie_type[1],1); // On duplique la sortie du tube avec stdout
		close(sortie_type[1]);
		char comd[NAME_SIZE] = "file --mime-type ";
		strcat(comd,path);
		execlp("sh" ,"sh","-c",comd,NULL); 
		exit(0);
	}else{
		close(entree_type[0]);
		close(entree_type[1]);
		close(sortie_type[1]);
		int status;
		waitpid(-1,&status,0);
		char type[128];
		int ret;
		do{
			ret = read(sortie[0],type,128);
			assert(ret != -1);
		}while(ret != 0);
		*(type + strcspn(type,"\n"))  ='\0';
		strcpy(meta->type,type + strcspn(type," ") + 1);
		close(sortie_type[0]);
	}




	printf("Vous avez rentré le chemin : %s\nNom du fichier : %s\nType du fichier : %s\n", path, meta->name, meta->type);
	printf("Hash du fichier : %s\n", meta->hash);

	printf("Entrez les mots clefs : \n");
	int tst = 1;
	cpt = 0;
	while(tst){
		if(cpt == MAX_KEYS){
			printf("Désolé, nous ne pouvons pas avoir plus de %d mots clefs", MAX_KEYS);
			break;
		}
		if(cpt != 0){
			while(1){
				printf("Voulez vous ajouter un mot clef ? (y/n) ");
				char* res = malloc(sizeof(char)*8);
				if(fgets(res,8,stdin) == NULL){
					printf("ERROR getting confirmation from stdin\n");
					exit(1);
				}
				if(*res == 'n'){
					tst = 0;
					free(res);
					break;
				}else if(*res == 'y'){
					free(res);
					break;
				}
				free(res);
			}
			if(!tst){
				break;
			}
		}
		printf("Entrez le mot clef : ");
		if(fgets(*(meta->keywords + cpt), KEY_SIZE, stdin) == NULL){
			printf("ERROR getting key from stdin\n");
			exit(1);
		}
		*(*(meta->keywords + cpt) + strcspn(*(meta->keywords + cpt), "\n")) = '\0';
		cpt++;
	}
	meta->nbKeywords = cpt;
	display_meta(meta);
	//free(path);
	wait(NULL);
	/*char* filename = malloc(sizeof(char)*32);
	strcpy(filename, "../saves/");
	strcat(filename, meta->name);
	strcat(filename,".sav");

	serialize_metadata(meta,filename);
	free(filename);
	destroy_meta(meta);*/
	return(meta);
}