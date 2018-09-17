#!/bin/bash
#si on a bien 2 arguments
if [ $# -eq 2 ]
then
	#si l'option n'est pas --tag, on fait un rm normal
	if [ "$1" != "--tag" ]
	then
		rm "$*"
	else
		#on met à jour les tags recent et ancien
		sqlite3 $chemin_base 'UPDATE tags SET nomtag="ANCIEN" WHERE nomtag="RECENT" AND datetag <= (CURRENT_DATE - CAST(2 AS INTERVAL MONTH));'
		chemin_base="/home/Yann/Projet/tag_base.db"
		ROUGE="\\033[31m"
		NORMAL="\\033[00m"
		tag=$2
		sqlite3 $chemin_base 'SELECT nomfichier FROM tags WHERE nomtag="'$tag'"' >temp.txt
		#on compte le nombre de lignes du fichier
		nbLigne=$(cat temp.txt | wc -l)
		#si le fichier est vide, c'est qu'il n'y a pas de fichier comportant le tag indiqué
		if [ $nbLigne -eq 0 ]  
		then
			echo "Auncun fichier ne comporte le tag "$tag
		fi
		#on affiche chaque ligne du fichier à l'aide de ls suivi du tag
		while read line
		do 
			sqlite3 $chemin_base 'DELETE FROM tags WHERE nomtag="'$tag'"'
			rm "$line"
			echo -e "Le fichier "$ROUGE$line$NORMAL" a été supprimé"
		done < temp.txt
		#on efface le fichier
		rm temp.txt
	fi
#si on a pas 2 arguments, on fait un rm normal
else
	rm "$*"
fi
