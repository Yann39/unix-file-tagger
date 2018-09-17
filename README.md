# General

The purpose of this project was to build a program capable of adding semantic information to the Unix
file system through file tags. For example user will be able to tag files as `important`, `public`, `private`, `draft`, etc.

Application is in french and has been built on **December 2007** under **Linux Fedora Core 5**.
I tried to respect **POSIX** standards as much as possible so it should be compatible with other distributions as well.

It uses simple **C** programs and **Shell** scripts.

It was originally compiled with **GCC** for **32** bits systems. I recompiled it recently for **64** bits systems.

# Usage

## Install

1. Get the archive from this project (_bin/file-tagger.tar.gz_), then uncompress and extract it wherever you want (i.e. _/usr/local_)
    ```bash
    tar zxvf file-tagger.tar.gz -C /usr/local
    ```
2. From the application folder, execute the `configure.sh` file which will set permissions and create aliases (also in the _/etc/profile file_)
    ```bash
    cd /usr/local/file-tagger
    ./configure.sh
    ```
    You can also do it manually if you prefer, just inspect the file.
3. Run available commands, see [Execute](#Execute)

## Compile

1. Make sure you have the right libraries (SQLite) to be able to compile the **C** files :
    ```bash
    sudo apt-get install sqlite3
    sudo apt-get install libsqlite3-dev
    ```

2. Get the sources from this project (_src_ folder) then compile them :
    ```bash
    gcc src/AfficherTags.c -lsqlite3 -o AfficherTags
    gcc src/AjouterTag.c -lsqlite3 -o AjouterTag
    gcc src/AjouterTagIncompatibles.c -lsqlite3 -o AjouterTagIncompatibles
    gcc src/SupprimerTag.c -lsqlite3 -o SupprimerTag
    gcc src/SupprimerTagIncompatibles.c -lsqlite3 -o SupprimerTagIncompatibles
    gcc src/ViderTableTags.c -lsqlite3 -o ViderTableTags
    ```

## Execute

Simply use the available commands (once aliases have been created) :
- `addtag file/directory tag` → add a tag on one or multiple files
- `deltag file/directory tag` → remove a tag from one or multiple files
- `addincompatibletags tag1 tag2` → set 2 tags as incompatible
- `delincompatibletags tag1 tag2` → remove the incompatibility between 2 tags
- `showtags` → show all tags in all tagged files
- `delalltags` → remove all tags from all tagged files
- `ls --tag [option] tag` → list files tagged with the specified tag
- `rm --tag tag` → remove files tagged with the specified tag

## Uninstall

1. Remove the aliases by running the `uninstall.sh` script
    ```bash
    cd /usr/local/file-tagger
    ./uninstall.sh
    ```
2. Remove the application folder
    ```bash
    rm -rf /usr/local/file-tagger
    ```

# Technical details

## Application architecture :

![Application architecture schema](doc/architecture.png?raw=true "Architecture")

So we have 6 **C** files that allow us to interact with the **SQLite** database. These are the following files :
- `SupprimerTag.c`
- `AjouterTag.c`
- `AfficherTags.c`
- `ViderTableTags.c`
- `SupprimerTagIncompatibles.c`
- `AjouterTagIncompatibles.c`

And 2 **Shell** scripts that allow us to override the `ls` and `rm` commands when the `--tag` option is specified (it runs native `ls` and `rm` commands when the option is not specified). These are the files :
- `SupprimerFichier.sh`
- `AfficherFichier.sh`

The `configuration.h` file contains the **SQL queries** (definitions) and the **path** to the database file (by default in the current directory where the program will be placed). It it only used for compilation.

The `configure.sh` file is a helper file used to set permissions and create aliases (also in the _/etc/profile file_).

The `uninstall.sh` file is a helper file used to remove aliases.

## Functions

None of the functions modify the Unix file system itself, you only need to delete the file representing the database (`tag_base.db`) so that everything becomes normal again (no more tagged files, incompatible tags, etc ...).
Only the execution of `rm --tag tag` will permanently delete the tagged file and is therefore irreversible.

### Tag a file/directory

It is done by executing the `addtag` command which will :
1. check if the file or folder passed in parameter exists
2. create the database if it does not exist
3. if the argument passed as first parameter is a file :
    - check if the file is already tagged
    - if it already is, checks that there is no incompatibility of tags
    - get the inode of the file and store it with the filename and its tag in the `tags` table into the database
4. if the argument passed in 2nd parameter is a directory, we goes through it recursively, and for each element, we call the function to tag the files (step 3 above is executed for each of the files)

When tagging a directory, you can interrupt the process using `ctrl + c`;

### Remove a tag from a file/directory

It is done by executing the `deltag` command which will :
1. check if the file or folder passed in parameter exists
2. if the argument passed in 1st parameter is a file :
    - check if the file is tagged
    - if it is, deleted it from the `tags` table from the database
3. if the argument passed as second parameter is a directory, we go through it recursively, and for each element, we call the function to "untag" all the files (step 3 above is executed for each files)

### Set 2 tags as incompatible

It is done by executing the `addincompatibletag` command which will :
1. check that the tags are not already incompatible
2. add the two incompatible tags in the `incompatible_tags` table into the database (add a record)

### Remove incompatibility between 2 tags

It is done by executing the `delincompatibletag` command which will :
1. check that the tags are not already incompatible
2. delete the two incompatible tags from the `incompatible_tags` table from the database (remove the record)

### List all tagged files

It is done by executing the `delincompatibletag` command which will simply retrieve the content of the `tags` table from the database (user-friendly formatted).

### List all incompatible tags

It is done by executing the `showincompatibletags` command which will simply retrieve the content of the `incompatible_tags` table from the database (user-friendly formatted).

### Remove all tags on all files

It is done by executing the `delalltags` command which will simply remove the content of the `tags` table from the database (delete all records).

### List all file tagged with a certain tag

It is done by executing the `ls –tag [option] tag` command which will :
- select corresponding files (paths) from `tags` table from the database
- do a `ls` for each of the paths so they are displayed
- if the `--tag` option is not specified, we just run the native `ls` command with the specified parameters

It uses the `AfficherFichier.sh` script.

### Remove all files tagged with a certain tag

It is done by executing the `rm –tag [option] tag` command which will :
- select corresponding files (paths) from `tags` table from the database
- do a `rm` for each of the paths so they are displayed
- if the `--tag` option is not specified, we just run the native `rm` command with the specified parameters

It uses the `SupprimerFichier.sh` script.

## Database

The **SQLite** database is very light, and contains only 2 tables :
- `tags` (`id`, `inodeFile`, `fileName`, `tagName`);
- `incompatible_tags` (`id`, `tagName`, `tagName2`);

The `tag` table therefore contains all the files with their associated tag, and the `incompatibles_tags` table contains all incompatible tag pairs.

# Examples

## Tag a file

![Tag file](doc/tag_file.png?raw=true "Tag file")

## Tag a directory (containing a subdirectory)

![Tag directory](doc/tag_directory.png?raw=true "Tag directory")

## Interrupting the tag of a directory

![Interrupt directory tag](doc/tag_directory_interrupt.png?raw=true "Interrupt directory tag")

## Tag a directory containing already tagged files and some with incompatible tags

![Tag directory with already tagged file](doc/tag_directory_with_errors.png?raw=true "Tag directory with already tagged file")

## Set 2 tags as incompatible

![Add incompatible tags](doc/add_incompatible_tags.png?raw=true "Add incompatible tags")

## Add already incompatible tags

![Add already incompatible tags](doc/tags_already_incompatible.png?raw=true "Add already incompatible tags")

## Remove incompatibility between 2 tags

![Remove tag incompatibility](doc/remove_incompatible_tags.png?raw=true "Remove tag incompatibility")

## Remove incompatibility between 2 tags that are not incompatible

![Remove tag incompatibility fail](doc/remove_incompatible_tags_fail.png?raw=true "Remove tag incompatibility fail")

## Display all incompatibles tags

![Display all incompatibles tags](doc/show_incompatible_tags.png?raw=true "Display all incompatibles tags")

## Remove a tag from a file

![Remove file tag](doc/remove_tag.png?raw=true "Remove file tag")

## Display files tagged with a certain tag

![Display files tagged with a certain tag](doc/show_tagged_files_with_tag.png?raw=true "Display files tagged with a certain tag")

## Display all tagged files

![Display all tagged files](doc/show_all_tagged_files.png?raw=true "Display all tagged files")

## Delete a file tagged with a certain tag

![Delete tagged file](doc/delete_tagged_file.png?raw=true "Delete tagged file")

## Delete a file tagged with a certain tag which does not exist

![Delete non existing tagged file](doc/delete_tagged_file_not_exist.png?raw=true "Delete non existing tagged file")

## Remove all tags from all files

![Remove all tags](doc/remove_all_tags.png?raw=true "Remove all tags")

# Licence

WTFPL license : http://www.wtfpl.net/

But a mention is always appreciated :)