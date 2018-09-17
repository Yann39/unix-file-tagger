#!/bin/bash
#si on a au moins 2 arguments
if [ $# -ge 2 ]
then
	#si l'option n'est pas --tag, on fait un ls normal
	if [ "$1" != "--tag" ]
	then
		if [ $# -eq 0 ]
		then
			ls $*
		else
			ls "$*"
		fi
	else
		#on met à jour les tags recent et ancien
		sqlite3 $chemin_base 'UPDATE tags SET nomtag="ANCIEN" WHERE nomtag="RECENT" AND datetag <= (CURRENT_DATE - CAST(2 AS INTERVAL MONTH));'
		chemin_base="/home/Yann/Projet/tag_base.db"
		ROUGE="\\033[37m"
		NORMAL="\\033[00m"
		if [ $# -eq 2 ]
		then
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
				ls "$line" | gawk '{printf("%s ",$0);}'
				echo -e $ROUGE$tag$NORMAL
			done < temp.txt
			#on efface le fichier
			rm temp.txt
		elif [ $# -eq 3 ]
		then
			tag=$3
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
				ls $2 "$line" | gawk '{printf("%s ",$0);}'
				echo -e $ROUGE$tag$NORMAL
			done < temp.txt
			#on efface le fichier
			rm temp.txt
		else
			echo -e "\033[04mUsage:\033[00m ls --tag [option] tag"
		fi
	fi
#si on a pas au moins 2 arguments
else
	if [ $# -eq 0 ]
	then
		ls $*
	else
		#si on a que l'option --tag sans paramètres
		if [ "$1" == "--tag" ]
		then
			echo -e "\033[04mUsage:\033[00m ls --tag [option] tag"
		#on fait un ls normal
		else
			ls "$*"
		fi
	fi
fi
