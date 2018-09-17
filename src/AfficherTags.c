#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <sys/stat.h>
#include <signal.h>
#include "configuration.h" 

sqlite3 *db; //Notre base sqlite
char *zErrMsg = 0; //Erreur éventuelle des fonctions sqlite 
int rc; //Pour stocker la valeur de retour des fonctions sqlite
int nbEnreg = 0; //Pour récupérer le nombre d'enregistrements affichés

/* ================================= */
/* Affiche le résultat d'une requête */
/* ================================= */
//Fonction qui est appelée pour chaque ligne de la table
static int callback(void *NotUsed, int argc, char **argv, char **azColName){
	int i;
	for(i=0; i<argc; i++){
		printf("\033[31m%s\033[00m = %s \033[1;34m|\033[00m ", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	nbEnreg++;
	return 0;
}

/* ================================ */
/* Gestion du signal SIGINT capturé */
/* ================================ */
void gererSigint (int sig_num) {
	printf ("\n\033[31mProgramme interrompu : \033[00m");
	if (nbEnreg >= 2)
		printf("\n\033[1;31m%d fichiers sélectionnés\033[00m\n\n",nbEnreg); //'\033[31m' met le texte en rouge
	else
		printf("\n\033[1;31m%d fichier sélectionné\033[00m\n\n",nbEnreg);
	exit(0);
}

/* ============= */
/* Fonction Main */
/* ============= */
int main(int argc, char **argv){
	//permet de capturer un signal ctrl+c
	if (signal (SIGINT, gererSigint) == SIG_ERR)
		printf("\nProblème d'interception SIGINT");
	//on a en fait 1 paramètre mais ce n'est pas à l'utilisateur de le saisir
	if (argc != 2) {
		fprintf(stderr, "Cette fonction n'attend aucun paramètre\n");
		exit(1);
	}
	//ouvre la base (ou créé la base si elle n'existe pas)
	rc = sqlite3_open(chemin_base, &db);
	if(rc){
		fprintf(stderr, "Impossible d'ouvrir la base de données: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	}
	//on met à jour la table (pour le changement du tag recent en ancien au bout d'un certain temps)
	rc = sqlite3_exec(db, UPDATE_TAG, NULL, 0, &zErrMsg);
	if(rc != SQLITE_OK) {
		fprintf(stderr, "Erreur SQL: %s\n", zErrMsg);
      		sqlite3_free(zErrMsg);
    	}
	//on exécute la requête de sélection avec le bon nom de table
	char* requete = malloc(55*sizeof(char));
	if (strcmp(argv[1],"tags") == 0)
		requete = SELECT_ALLFICHTAG;
	else if (strcmp(argv[1],"incompatible_tags") == 0)
		requete = SELECT_ALLINCTAG;
	else {
		fprintf(stderr, "Table %s inconnue\n",argv[1]);
		exit(1);
	}
	rc = sqlite3_exec(db, requete, callback, 0, &zErrMsg);
	if(rc != SQLITE_OK) {
		fprintf(stderr, "Erreur SQL: %s\n", zErrMsg);
      		sqlite3_free(zErrMsg);
    	}
	if (strcmp(argv[1],"tags") == 0) {
		if (nbEnreg < 2)
			printf("\033[1;31m%d fichier sélectionné\033[00m\n",nbEnreg);
		else
			printf("\033[1;31m%d fichiers sélectionnés\033[00m\n",nbEnreg);
	}
	else if (strcmp(argv[1],"incompatible_tags") == 0) {
		if (nbEnreg < 2)
			printf("\033[1;31m%d couple sélectionné\033[00m\n",nbEnreg);
		else
			printf("\033[1;31m%d couples sélectionnés\033[00m\n",nbEnreg);
	}
	else {
		fprintf(stderr, "Table %s inconnue\n",argv[1]);
		exit(1);
	}
	sqlite3_close(db);
	requete = NULL, free(requete);
	return EXIT_SUCCESS;
}
