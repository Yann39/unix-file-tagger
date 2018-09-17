#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <sys/stat.h>
#include "configuration.h" 

sqlite3 *db; //Notre base sqlite
char *zErrMsg = 0; //Erreur SQLite 

/* ============= */
/* Fonction Main */
/* ============= */
int main(int argc, char **argv){
	char c;
	printf("\nToutes les données seront éffacées, êtes-vous sûr de vouloir continuer ? (y/n) ");
	scanf("%c",&c);
	if (c=='y') { 
		//ouvre la base (ou créé la base si elle n'existe pas)
		int rc = sqlite3_open(chemin_base, &db);
		if(rc){
			fprintf(stderr, "Impossible d'ouvrir la base de données: %s\n", sqlite3_errmsg(db));
			sqlite3_close(db);
		}
		rc = sqlite3_exec(db, "DELETE FROM tags;", NULL, 0, &zErrMsg);
		if(rc != SQLITE_OK) {
			fprintf(stderr, "Erreur SQL: %s\n", zErrMsg);
      			sqlite3_free(zErrMsg);
    		}
		printf("\nTous les tags ont bien été supprimés\n\n");
		sqlite3_close(db);
	}
	return EXIT_SUCCESS;
}
