#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include "configuration.h"

sqlite3 *db; //notre base sqlite
char *zErrMsg = 0; //pour récupérer les éventuelles erreurs SQLite
int rc; //pour récupérer la valeur de retour des fonctions sqlite
int nbFichTag=0; //pour récupérer le nombre de fichiers taggés

/* ======================================================= */
/* Vérifie l'existence d'une table dans la base de données */
/* ======================================================= */
int VerifierExistTable(char* table) {
	sqlite3_stmt* stmt;
	//on stock la requête dans stmt et on récupère la valeur de retour de la fonction dans rc
	rc = sqlite3_prepare(db, SELECT_TABLES, -1, &stmt, 0);
	if(rc != SQLITE_OK) {
		fprintf(stderr, "Erreur SQL: %s\n", zErrMsg);
      		sqlite3_free(zErrMsg);
    	}
	//on compte le nombre de colonnes retournées par la requête
	int cols = sqlite3_column_count(stmt);
	int i, vbool=0;
	do {
		rc = sqlite3_step(stmt);
        	//on regarde pour chaque colonne retournées (dans ce cas le nom des tables) si la table apparaît
		if (rc == SQLITE_ROW) {
			for (i=0; i<cols; i++)
				if (strcmp((char*)sqlite3_column_text(stmt,i),table) == 0)
					vbool = 1;
	  	}
	} while(rc == SQLITE_ROW); //tant qu'on a des enregistrements
	sqlite3_finalize(stmt);
	return vbool;
}

/* ============================== */
/* Récupère l'inoeud d'un fichier */
/* ============================== */
int RecupInoeud(char* fichier) {
	struct stat s;
	int res = stat(fichier,&s);
	if (res < 0) {
		perror("stat");
		printf("-> %s",fichier);
		exit(1);
	}
	return s.st_ino;
}

/* ============================================================== */
/* Vérifie l'existence d'un fichier taggé dans la base de données */
/* ============================================================== */
int VerifierExistFichTag(char* fichier, char* tag) {
	//récupère les infos du fichier (inoeud)
	int inoeud = RecupInoeud(fichier);
	sqlite3_stmt* stmt;
	//buffer qui contiendra la requête à exécuter
	char* sql_buf = malloc(200*sizeof(char));  
	if(sql_buf == NULL) {
		printf("\nECHEC ALLOCATION MEMOIRE !\n"); 
		exit(1); 
	}
	//on complète la requête avec les valeurs fichier et tag
	sprintf(sql_buf,SELECT_FICHTAG,inoeud,tag);	
	//on stock la requête dans stmt et on récupère la valeur de retour
	rc = sqlite3_prepare(db, sql_buf, -1, &stmt, 0);
	if(rc != SQLITE_OK) {
		fprintf(stderr, "Erreur SQL: %s\n", zErrMsg);
      		sqlite3_free(zErrMsg);
    	}
	int j=0, vbool=0;
	//on compte le nombre de lignes retournées par la requête
	rc = sqlite3_step(stmt);
	while(rc == SQLITE_ROW) {
		j++;
		rc = sqlite3_step(stmt);
	}
	//si la requête retourne une ligne, c'est que le fichier est déjà taggé
	if (j != 0) 
		vbool=1;
	free(sql_buf), sql_buf = NULL;
	sqlite3_finalize(stmt);
	return vbool;
}

/* ======================================== */
/* Vérifie la compatibilité entre deux tags */
/* ======================================== */
char* VerifCompatibiliteTag(char* fichier, char* tag) {
	//pour stocker nos requêtes
	sqlite3_stmt* stmt;
	sqlite3_stmt* stmt2;
	//buffer qui contiendra les requêtes à exécuter
	char* sql_buf = malloc(200*sizeof(char));
	if(sql_buf == NULL) {
		printf("\nECHEC ALLOCATION MEMOIRE !\n"); 
		exit(1); 
	}
	//on complète la requête avec les valeurs du tag
	sprintf(sql_buf,SELECT_INCTAGFICH,tag,tag);
	//on sélectionne ainsi tous les couples incompatibles contenant le tag
	rc = sqlite3_prepare(db, sql_buf, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Erreur SQL: %s\n", zErrMsg);
      		sqlite3_free(zErrMsg);
    	}
	//on compte le nombre de lignes retournées par la requête
	int nbLigne = 0;
	rc = sqlite3_step(stmt);
	while(rc == SQLITE_ROW) {
		nbLigne++;
		rc = sqlite3_step(stmt);
	}
	if (nbLigne == 0) { //le tag n'a pas d'incompatibilités connues
		sqlite3_finalize(stmt);
		free(sql_buf), sql_buf = NULL;
		return "OK";
	}
	//on remet le sqlite3_step à zéro
	sqlite3_finalize(stmt);
	rc = sqlite3_prepare(db, sql_buf, -1, &stmt, 0);
	//récupère les infos du fichier (inoeud)
	int inoeud = RecupInoeud(fichier);
	//on complète la requête avec la valeur de l'inoeud du fichier
	sprintf(sql_buf,SELECT_FICHITAG,inoeud);
	//on sélectionne ainsi tous les tags du fichier
	int rc2 = sqlite3_prepare(db, sql_buf, -1, &stmt2, 0);
	if (rc2 != SQLITE_OK) {
		fprintf(stderr, "Erreur SQL: %s\n", zErrMsg);
      		sqlite3_free(zErrMsg);
    	}
	//on compte le nombre de lignes retournées par la requête
	nbLigne = 0;
	rc2 = sqlite3_step(stmt2);
	while(rc2 == SQLITE_ROW) {
		nbLigne++;
		rc2 = sqlite3_step(stmt2);
	}
	if (nbLigne == 0) { //le fichier n'est pas taggé
		sqlite3_finalize(stmt);
		sqlite3_finalize(stmt2);
		free(sql_buf), sql_buf = NULL;
		return "OK";
	}
	//on remet le sqlite3_step à zéro
	sqlite3_finalize(stmt2);
	rc2 = sqlite3_prepare(db, sql_buf, -1, &stmt2, 0); 
	//on regarde si le tag est compatible avec les tags déjà existants du fichier
	int doublebreak = 0;
	do {
		rc = sqlite3_step(stmt);
		if (rc == SQLITE_ROW) {
			do {
				rc2 = sqlite3_step(stmt2);
				if (rc2 == SQLITE_ROW) {
					//si notre tag est égal au 1er tag du couple et que le tag du fichier est égal au 2ème tag du couple 
					if (strcmp(tag,(char*)sqlite3_column_text(stmt,0))==0 && strcmp(sqlite3_column_text(stmt2,0),sqlite3_column_text(stmt,1))==0) {
						sqlite3_finalize(stmt2);
						char* tempchar = (char*)sqlite3_column_text(stmt,1); //pour pouvoir libérer stmt avant le return
						sqlite3_finalize(stmt);
						free(sql_buf), sql_buf = NULL;
						return tempchar;
					}
					//si notre tag est égal au 2ème tag du couple et que le tag du fichier est égal au 1er tag du couple
					if (strcmp(tag,(char*)sqlite3_column_text(stmt,1))==0 && strcmp(sqlite3_column_text(stmt2,0),sqlite3_column_text(stmt,0))==0) {
						sqlite3_finalize(stmt2);
						char* tempchar = (char*)sqlite3_column_text(stmt,0); //pour pouvoir libérer stmt avant le return
						sqlite3_finalize(stmt);
						free(sql_buf), sql_buf = NULL;
						return tempchar;
					}
	  			}
			} while (rc2 == SQLITE_ROW); //tant qu'on a des enregistrements
			sqlite3_finalize(stmt2);
			rc2 = sqlite3_prepare(db, sql_buf, -1, &stmt2, 0); //pour remettre le sqlite3_step à zéro
		}
	} while (rc == SQLITE_ROW); //tant qu'on a des enregistrements
	sqlite3_finalize(stmt);
	sqlite3_finalize(stmt2);
	free(sql_buf);
	return "OK";
}

/* ========================================================== */
/* Vérifie si l'argument passé et un fichier ou un répertoire */
/* ========================================================== */
int EstUnRep(char *fichier) {
	//structure qui acueillera les informations du fichier
	struct stat s;
	int res = stat(fichier,&s);
	if (res < 0) {
		perror("stat");
		printf("-> %s",fichier);
		exit(1);
	}
	//si c'est un répertoire on retourne vrai
	if (S_ISDIR(s.st_mode))
		return 1;
	else
		return 0;
}

/* ====================================================================== */
/* Récupère le chemin complet du fichier passé en argument et le vérifie  */
/* ====================================================================== */
char* RecupChemin(char* fichier) { 
	//cas ou l'utilisateur saisie un ../ mais pas au début du chemin
	int i;
	if (strlen(fichier) > 2)
		for (i=0;i<strlen(fichier)-3;i++)      
			if (fichier[i]=='.' && fichier[i+1]=='.' && fichier[i+2]=='/' && i!=0) {
				printf("\n'../' accepté seulement en début de chemin\n");
				exit(1);
			}	 
	if (fichier[0] == '/') //chemin absolu, on a plus rien à faire
		return fichier;
	else { //chemin relatif, il faut récupérer le chemin absolu
		char* chemin = NULL;
		char* fichmodif = NULL; //notre nouveau nom de fichier modifié
		size_t size = FILENAME_MAX;
		char* buffer = malloc(size); //contiendra le répertoire courant
		if(buffer == NULL) { 
			printf("\nECHEC ALLOCATION MEMOIRE !\n"); 
			exit(1); 
		}
		//on récupère le répertoire courant dans la variable buffer
		if (getcwd(buffer, size) != 0) {
			//cas ou l'utilisateur saisie un ./ au début
			if (fichier[0]=='.' && fichier[1]=='/') {
				fichmodif = malloc(strlen(fichier)-1);
				if(fichmodif==NULL) { 
					printf("\nECHEC ALLOCATION MEMOIRE !\n"); 
					exit(1); 
				}
				strcpy(fichmodif,fichier+2);
			}
			//cas ou l'utilisateur saisie un ../ au début
			else if (fichier[0]=='.' && fichier[1]=='.' && fichier[2]=='/') {
				int j, nbcarac=1;
				for (j=strlen(buffer)-1;j>=0;j--) {
					if(buffer[j] == '/')
						break;
					else
						nbcarac++;
				}
				//on enleve le ../ du nom de fichier
				fichmodif = malloc(strlen(fichier)-2);
				if(fichmodif == NULL) { 
					printf("\nECHEC ALLOCATION MEMOIRE !\n"); 
					exit(1); 
				}
				strcpy(fichmodif,fichier+3);
				//on modifie le chemin d'accès en enlevant le dernier répertoire du chemin
				char* buffer2 = malloc(strlen(buffer)-nbcarac+1);
				buffer2[strlen(buffer)-nbcarac] = '\0';
				strncpy(buffer2,buffer,strlen(buffer)-nbcarac);
				buffer = buffer2;
			}
			//sinon l'utilisateur n'a saisie ni ./ ni ../
			else {
				fichmodif = malloc(strlen(fichier)+1);
				if(fichmodif == NULL) { 
					printf("\nECHEC ALLOCATION MEMOIRE !\n"); 
					exit(1); 
				}
				strcpy(fichmodif,fichier);
			}		
			chemin = malloc(strlen(buffer)+strlen(fichier)+2);
			if(chemin == NULL) { 
				printf("\nECHEC ALLOCATION MEMOIRE !\n"); 
				exit(1); 
			}
			//on concatène le répertoire courant suivi d'un '/' puis du nom du fichier
			strcpy(chemin, buffer);
			strcat(chemin, "/");
			strcat(chemin, fichmodif);
		}
		free(buffer), buffer = NULL;
		free(fichmodif), fichmodif = NULL;
		return chemin;
		free(chemin), chemin = NULL;
	}
}

/* ========================================== */
/* Vérifie si un fichier existe sur le disque */
/* ========================================== */ 
int VerifierExistFichDisque(char* fichier) {
	//on tente d'ouvrir le fichier
	FILE* flux = fopen(fichier, "r");
	//si l'ouverture s'est bien passée, on retourne vrai 
	if(flux == NULL)   
		return 0;
	else 
		return 1;
}

/* ========================= */
/* Récupère la date courante */
/* ========================= */ 
char* DateAujourdhui(char* buffer) {
	time_t t;
	struct tm * timeinfo;
	time(&t);
	timeinfo = localtime(&t);
	strftime(buffer,11,"%Y-%m-%d",timeinfo);
	return buffer;
}

/* ========================================= */
/* Ajoute un fichier et son tag dans la base */
/* ========================================= */
void AjouterTag(char* fichier, char* tag) {
	//récupère les infos du fichier (inoeud)
	int inoeud = RecupInoeud(fichier);
	//buffer qui contiendra la requete à exécuter
	char* sql_buf=(char*)malloc(200*sizeof(char)); 
	if(sql_buf == NULL) {
		printf("\nECHEC ALLOCATION MEMOIRE !\n"); 
		exit(1); 
	}
	char buffer[11];
	//on complète la requete avec les valeurs fichier et tag
	sprintf(sql_buf,INSERT_TAG,inoeud,fichier,tag,DateAujourdhui(buffer));
	//on exécute la requête
	rc = sqlite3_exec(db, sql_buf, NULL, 0, &zErrMsg); 
	if(rc != SQLITE_OK) {
		fprintf(stderr, "Erreur SQL: %s\n", zErrMsg);
      		sqlite3_free(zErrMsg);
    	}
	printf("\n Fichier \033[33m%s\033[00m taggé",fichier);
	free(sql_buf), sql_buf = NULL;
}

/* ================================= */
/* Met à jour le nom d'un répertoire */
/* ================================= */
char* MajSlashs(char* repertoire) {
	char* repert = NULL;	
	//on parcours le nom du répertoire depuis la fin pour voir si il y a un ou plusieur slash
	int i,nbSlashs = 0, queDesSlashs=1;
	for (i=strlen(repertoire)-1;i>=0;i--) {
		if (repertoire[i] == '/')
			nbSlashs++;
		else {
			queDesSlashs=0;
			break;
		}
	}
	//si le nom du répertoire ne finit pas par '/', on le rajoute
	if (nbSlashs == 0) {
		repert = malloc(strlen(repertoire)+2);
		if(repert == NULL) {
			printf("\nECHEC ALLOCATION MEMOIRE !\n"); 
			exit(1); 
		}
		strcpy(repert,repertoire);
		strcat(repert,"/");
		return repert;
	}
	//si il n'y a que des slashs, le répertoire est '/'
	if (queDesSlashs == 1) {
		repert = malloc(2);
		if(repert == NULL) {
			printf("\nECHEC ALLOCATION MEMOIRE !\n"); 
			exit(1); 
		}
		repert="/";
		return repert;
	}
	//si il n'y a qu'un seul '/', on ne touche rien
	if (nbSlashs==1 && queDesSlashs==0)
		return repertoire;
	//si il y a plusieurs '/' à la fin, on en garde un seul
	if (nbSlashs > 1) {
		repert = malloc(strlen(repertoire)-nbSlashs+1);
		repert[strlen(repertoire)-nbSlashs] = '\0';
		if(repert == NULL) {
			printf("\nECHEC ALLOCATION MEMOIRE !\n"); 
			exit(1); 
		}
		strncpy(repert,repertoire,strlen(repertoire)-nbSlashs);
		strcat(repert,"/");
		return repert;
	}
	free(repert), repert = NULL;
}

/* ======================================================== */
/* Tag un fichier seul ou tous les fichiers d'un répertoire */
/* ======================================================== */
int TaggerFichier(char* fichOUrep, char* tag) {
	//on regarde si c'est est un fichier ou un répertoire
	int estrep = EstUnRep(fichOUrep);
	if (estrep == 0) { //c'est un fichier
		//on récupère le chemin d'accès complet du fichier
		char* chem = RecupChemin(fichOUrep);
  		//on vérifie si le fichier est déjà taggé ou non (avec le même tag)
		if (VerifierExistFichTag(chem,tag) == 1)
			printf("\n Le fichier \033[33m%s\033[00m contient déjà le tag \033[33m%s\033[00m",chem,tag);
		else {
			char* comp = NULL;
			//si la table incompatible_tags existe, on vérifie si il n'y a pas d'incompatibilité avec le tag
			int existTable = VerifierExistTable("incompatible_tags");
			if (existTable == 1)
				comp = VerifCompatibiliteTag(chem,tag);
			else
				comp = "OK";
			//si les tags sont incompatibles, on affiche un message et on ne tag pas le fichier
			if (strcmp(comp,"OK") != 0)
					printf("\n Fichier \033[33m%s\033[00m : les tags \033[33m%s\033[00m et \033[33m%s\033[00m sont incompatibles",chem,comp,tag);
			else {
				//ajoute le fichier et son tag dans la base
				AjouterTag(chem,tag); 
				nbFichTag++;
			}
		}
	}
	else { //c'est un répertoire
		char* chemincomplet = NULL;
		//on met à jour le nom du dossier pour qu'il se termine bien par '/'
		char* dossier = MajSlashs(fichOUrep);
		//on ouvre le dossier
		struct dirent *lecture;
		DIR *rep;
		rep = opendir(dossier);
		if (rep == NULL) {
			if (nbFichTag < 2)
				printf("\nImpossible d'ouvrir le répertoire \033[33m%s\033[00m\n\033[1;31m%d fichier taggé\033[00m\n\n",dossier,nbFichTag);
			else
				printf("\nImpossible d'ouvrir le répertoire \033[33m%s\033[00m\n\033[1;31m%d fichiers taggés\033[00m\n\n",dossier,nbFichTag);
			exit(1);
		}
		//on récupère le chemin d'accès du dossier
		char* cheminrep = RecupChemin(dossier);
		printf("\n\033[35mDébut répertoire %s :\033[00m",cheminrep);
		//on lit chacun des fichiers contenu dans le dossier
		while ((lecture = readdir(rep))) {
			if (strcmp(lecture->d_name,".")!=0 && strcmp(lecture->d_name,"..")!= 0 && strcmp(lecture->d_name+strlen(lecture->d_name)-1,"~") != 0) {
				//on récupère le chemin d'accès complet du fichier
				chemincomplet = malloc(strlen(cheminrep)+strlen(lecture->d_name)+1);
				if(chemincomplet == NULL) {
					printf("\nECHEC ALLOCATION MEMOIRE !\n"); 
					exit(1); 
				}
				strcpy(chemincomplet,cheminrep);
				strcat(chemincomplet,lecture->d_name);
				//on relance la fonction pour tagger le fichier
				TaggerFichier(chemincomplet,tag);
				free(chemincomplet), chemincomplet = NULL;
			}
		}
		printf("\n\033[35mFin répertoire %s\033[00m",cheminrep);
		closedir(rep);
	}
	return nbFichTag;
}

/* ================================ */
/* Gestion du signal SIGINT capturé */
/* ================================ */
void gererSigint (int sig_num) {
	printf ("\n\n\033[31mProgramme interrompu : \033[00m");
	if (nbFichTag != 1)
		printf("\n\033[1;31m%d fichiers taggés\033[00m\n\n",nbFichTag); //'\033[31m' met le texte en rouge
	else
		printf("\n\033[1;31m%d fichier taggé\033[00m\n\n",nbFichTag);
	exit(0);
}

/* ============= */
/* Fonction Main */
/* ============= */
int main(int argc, char **argv) {
	//permet de capturer un signal ctrl+c
	if (signal (SIGINT, gererSigint) == SIG_ERR)
		printf("\nProblème d'interception SIGINT");
	//on vérifie si on a le bon nombre d'arguments
	if(argc != 3) {
		fprintf(stderr, "\033[04mUsage:\033[00m addtag fichier nom_du_tag\n");
		exit(1);
	}
	//on vérifie si le fichier existe sur le disque
	int exist = VerifierExistFichDisque(argv[1]);
	if (exist == 0) {
		printf("\nLe fichier %s n'a pas été trouvé.\n\n",argv[1]);
		exit(1);
	}
	//ouvre la base (ou créé la base si elle n'existe pas)
	rc = sqlite3_open(chemin_base, &db);
	if(rc) {
		fprintf(stderr, "Impossible d'ouvrir la base de données: %s\n", sqlite3_errmsg(db));
 		sqlite3_close(db);
		exit(1);
	}
	//créé la table tags si elle n'existe pas
	int creerTable = VerifierExistTable("tags");
	if(creerTable != 1) {
		rc = sqlite3_exec(db, CREATE_TAG_TABLE, NULL, 0, &zErrMsg);
		if (rc != SQLITE_OK) {
			fprintf(stderr, "Erreur SQL: %s\n", zErrMsg);
      			sqlite3_free(zErrMsg);
    		}
  	}
	else {
		//on met à jour la table (pour le changement du tag recent en ancien au bout d'un certain temps)
		rc = sqlite3_exec(db, UPDATE_TAG, NULL, 0, &zErrMsg);
		if(rc != SQLITE_OK) {
			fprintf(stderr, "Erreur SQL: %s\n", zErrMsg);
      			sqlite3_free(zErrMsg);
    		}
	}
	//on tag le fichier ou le dossier et on affiche le nombre de fichiers taggés.
	int nbTag = TaggerFichier(argv[1],argv[2]);
	if (nbTag >= 2)
		printf("\n\033[1;31m%d fichiers taggés\033[00m\n\n",nbTag); //'\033[31m' met le texte en rouge
	else
		printf("\n\033[1;31m%d fichier taggé\033[00m\n\n",nbTag);

	sqlite3_close(db);
	return EXIT_SUCCESS;
}
