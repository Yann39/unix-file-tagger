#!/bin/bash
#Author : Yann <rockyracer@mailfence.com>
#Date : dec 2007

#si on a pas d'arguments
if [ $# -eq 0 ]
then
    #chemin d'installation, à remplacer si vous n'exécutez pas ce script depuis le répertoire d'installation
    chemin=`pwd`

    #on donne les droits pour pouvoir exécuter les fichiers
    chmod u+rwx $chemin/AfficherFichier.sh
    chmod u+rwx $chemin/SupprimerFichier.sh

    #on créé les alias
    alias addtag='"$chemin"/AjouterTag'
    alias addincompatibletags='"$chemin"/AjouterTagIncompatibles'
    alias delincompatibletags='"$chemin"/SupprimerTagIncompatibles'
    alias deltag='"$chemin"/SupprimerTag'
    alias delalltags='/"$chemin"/ViderTableTags'
    alias showtags='"$chemin"/AfficherTags tags'
    alias showincompatibletags='"$chemin"/AfficherTags incompatible_tags'
    alias ls='"$chemin"/AfficherFichier.sh'
    alias rm='"$chemin"/SupprimerFichier.sh'

    //on place également les alias dans le fichier /etc/profile
    aliases="\n#tag de fichiers\nalias addtag='"$chemin"/AjouterTag'\nalias addincompatibletags='"$chemin"/AjouterTagIncompatibles'\nalias delincompatibletags='"$chemin"/SupprimerTagIncompatibles'\nalias deltag='"$chemin"/SupprimerTag'\nalias delalltags='/"$chemin"/ViderTableTags'\nalias showtags='"$chemin"/AfficherTags tags'\nalias showincompatibletags='"$chemin"/AfficherTags incompatible_tags'\nalias ls='"$chemin"/AfficherFichier.sh'\nalias rm='"$chemin"/SupprimerFichier.sh'"

    #on récupère le nombre de lignes contenant la chaine $aliases dans le fichier /etc/profile
    nblignes=`grep -c SupprimerTagIncompatibles /etc/profile`
    if [ $nblignes -eq 0 ]
    then
        #on défini les commandes dans le fichier /etc/profile si elles n'y sont pas
        echo -e $aliases >> /etc/profile
    fi

#si on a des arguments, on affiche un message
else
	echo "configure n'attend aucun argument"
fi