#!/bin/bash
#Author : Yann <rockyracer@mailfence.com>
#Date : dec 2007

#efface les alias existants
unalias addtag
unalias addincompatibletags
unalias delincompatibletags
unalias deltag
unalias delalltags
unalias showtags
unalias showincompatibletags
unalias ls
unalias rm

# retire les alias di fichier /etc/profile afin qu'ils ne soient pas recréés
sed -i '/#tag de fichiers/d' /etc/profile
sed -i '/alias addtag/d' /etc/profile
sed -i '/alias addincompatibletags/d' /etc/profile
sed -i '/alias delincompatibletags/d' /etc/profile
sed -i '/alias deltag/d' /etc/profile
sed -i '/alias delalltags/d' /etc/profile
sed -i '/alias showtags/d' /etc/profile
sed -i '/alias showincompatibletags/d' /etc/profile
sed -i '/alias ls/d' /etc/profile
sed -i '/alias rm/d' /etc/profile