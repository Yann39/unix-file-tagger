#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <signal.h>
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
        	//on regarde pour chaque colonne retournées (dans notre cas le nom des tables) si la table apparaît
		if (rc == SQLITE_ROW) {
			for (i=0; i<cols; i++)
				if (strcmp((char*)sqlite3_column_text(stmt,i),table) == 0)
					vbool = 1;
	  	}
	} while(rc == SQLITE_ROW); //tant qu'on a des enregistrements
	sqlite3_finalize(stmt);
	return vbool;
}

/* ================================================== */
/* Vérifie l'existence de fichiers déjà incompatibles */
/* ================================================== */
int VerifierExistTagIncompatible(char* tag1, char* tag2) {
	sqlite3_stmt* stmt;
	//buffer qui contiendra la requête à exécuter
	char* sql_buf = malloc(200*sizeof(char));  
	if(sql_buf == NULL) {
		printf("\nECHEC ALLOCATION MEMOIRE !\n"); 
		exit(1); 
	}
	//on complète la requête avec les valeurs des tags
	sprintf(sql_buf,SELECT_INCTAG,tag1,tag2,tag1,tag2);	
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
	//si la requête retourne une ligne, c'est que les fichiers sont déjà incompatibles
	if (j != 0) 
		vbool=1;
	free(sql_buf), sql_buf = NULL;
	sqlite3_finalize(stmt);
	return vbool;
}

/* ====================================================== */
/* Ajoute deux tags incompatibles dans la base de données */
/* ====================================================== */
void AjouterTagIncompatible(char* tag1, char* tag2) {
	//buffer qui contiendra la requete à exécuter
	char* sql_buf=(char*)malloc(200*sizeof(char)); 
	if(sql_buf == NULL) {
		printf("\nECHEC ALLOCATION MEMOIRE !\n"); 
		exit(1); 
	}
	//on complète la requete avec les valeurs des tags
	sprintf(sql_buf,INSERT_INC,tag1,tag2);
	//on exécute la requête
	rc = sqlite3_exec(db, sql_buf, NULL, 0, &zErrMsg); 
	if(rc != SQLITE_OK) {
		fprintf(stderr, "Erreur SQL: %s\n", zErrMsg);
      		sqlite3_free(zErrMsg);
    	}
	printf("\nLes tags \033[33m%s\033[00m et \033[33m%s\033[00m sont maintenant incompatibles\n\n",tag1,tag2);
	free(sql_buf), sql_buf = NULL;
}

/* ============= */
/* Fonction Main */
/* ============= */
int main(int argc, char **argv) {
	//on vérifie si on a le bon nombre d'arguments
	if(argc != 3) {
		fprintf(stderr, "\033[04mUsage:\033[00m addincompatibletag nom_du_tag1 nom_du_tag2\n");
		exit(1);
	}
	//ouvre la base (ou créé la base si elle n'existe pas)
	rc = sqlite3_open(chemin_base, &db);
	if(rc) {
		fprintf(stderr, "Impossible d'ouvrir la base de données: %s\n", sqlite3_errmsg(db));
 		sqlite3_close(db);
		exit(1);
	}
	//créé la table incompatible_tags si elle n'existe pas
	int creerTable = VerifierExistTable("incompatible_tags");
	if(creerTable != 1) {
		rc = sqlite3_exec(db, CREATE_INC_TABLE, NULL, 0, &zErrMsg);
		if(rc != SQLITE_OK) {
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
	//si les tags ne sont pas déjà incompatibles, on les ajoute dans la base de données
	int exist = VerifierExistTagIncompatible(argv[1],argv[2]);
	if (exist == 0)
		AjouterTagIncompatible(argv[1],argv[2]);
	else
		printf("\nLes tags \033[33m%s\033[00m et \033[33m%s\033[00m sont déjà incompatibles\n\n",argv[1],argv[2]);
	sqlite3_close(db);
	return EXIT_SUCCESS;
}
