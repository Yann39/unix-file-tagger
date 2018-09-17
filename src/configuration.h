/* ============ */
/* Requêtes SQL */
/* ============ */
#define CREATE_TAG_TABLE "CREATE TABLE tags(id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, inoeudfichier INTEGER NOT NULL, nomfichier VARCHAR NOT NULL, nomtag VARCHAR NOT NULL, datetag DATE NOT NULL, UNIQUE(inoeudfichier,nomtag));"
#define CREATE_INC_TABLE "CREATE TABLE incompatible_tags(id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, nomtag1 VARCHAR NOT NULL, nomtag2 VARCHAR NOT NULL, UNIQUE(nomtag1,nomtag2));"
#define INSERT_TAG "INSERT INTO tags(inoeudfichier,nomfichier,nomtag,datetag) VALUES (%d, '%s', '%s', '%s');"
#define INSERT_INC "INSERT INTO incompatible_tags(nomtag1,nomtag2) VALUES ('%s', '%s');"
#define SELECT_FICHTAG "SELECT nomtag FROM tags WHERE inoeudfichier='%d' AND nomtag='%s';"
#define SELECT_FICHITAG "SELECT nomtag FROM tags WHERE inoeudfichier='%d';"
#define SELECT_INCTAG "SELECT id FROM incompatible_tags WHERE (nomtag1='%s' AND nomtag2='%s') OR (nomtag2='%s' AND nomtag1='%s');"
#define SELECT_TABLES "SELECT name FROM sqlite_master WHERE type='table';"
#define SELECT_ALLFICHTAG "SELECT nomfichier AS 'nom du fichier', nomtag AS 'nom du tag', datetag AS 'date du tag' FROM tags;"
#define SELECT_ALLINCTAG "SELECT nomtag1 AS 'nom du tag1', nomtag2 AS 'nom du tag2' FROM incompatible_tags;"
#define DELETE_TAG "DELETE FROM tags WHERE inoeudfichier='%d' AND nomtag='%s';"
#define DELETE_INCTAG "DELETE FROM incompatible_tags WHERE (nomtag1='%s' AND nomtag2='%s') OR (nomtag2='%s' AND nomtag1='%s');"
#define SELECT_INCTAGFICH "SELECT nomtag1, nomtag2 FROM incompatible_tags WHERE nomtag1='%s' OR nomtag2='%s';"
#define UPDATE_TAG "UPDATE tags SET nomtag='ANCIEN' WHERE nomtag='RECENT' AND datetag <= (CURRENT_DATE - CAST(2 AS INTERVAL MONTH));"

char* chemin_base = "tag_base.db";
