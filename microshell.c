/*AUTHOR: Dominika Lesniewska*/
/*HOW TO COMPILE: gcc microshell.c -lreadline -> ./a.out*/

#define _CRT_SECURE_NO_WARNINGS
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <pwd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <ftw.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include <fcntl.h>
#include <dirent.h>

#define GREEN "\e[0;32m"
#define YELLOW "\e[0;33m"
#define GRBOLD "\e[1;32m"
#define YELBOLD "\e[1;33m"
#define WHTBOLD "\e[1;37m"
#define reset "\e[0m"

#define BUFFER_SIZE 1024
#define COMMAND_SIZE 64
#define BRACE 123
#define BRACECLOSED 125
#define SLASH 47
#define BACKSLASH 92
#define QUOTE 34
#define DOT 46

/*GLOBAL VARIABLES*/
char cwd[PATH_MAX];
char prevPath[PATH_MAX];

void help(){
    printf("\n*** "GREEN"MicroShell SOP"reset" ***\n");
    printf(YELLOW"Autor"reset": "WHTBOLD"Dominika Lesniewska\n");
    printf(YELLOW"Grupa"reset": "WHTBOLD"13"reset"\n");
    printf(YELLOW"Numer albumu"reset": "WHTBOLD"464957"reset"\n\n");
    printf(YELLOW"Powloka obsluguje nastepujace rzeczy"reset": \n");
    printf("- help\n");
    printf("- cd\n");
    printf(GRBOLD"================================\n"reset);
    printf(YELLOW"- mkdir:\n"reset);
    printf("\t* mkdir [name1] [name2]...\n");
    printf("\t* mkdir {[name1],[name2],[name3]}\n");
    printf("\t* mkdir -p [name1]/[name2]/[name3]\n");
    printf("\t* mkdir -p [name1]/[name2]/{[name3],[name4]}\n\n");
    printf(GRBOLD"================================\n"reset);
    printf(YELLOW"- rm/rmdir (usuwanie pustego pliku - rmdir):\n"reset);
    printf("\t* rmdir [dirname1]\n");
    printf("\t* rmdir [dirname1]/[dirname2]/...[dirname_x]/\n");
    printf("\t* rm [filename1]\n");
    printf("\t* rm [dirname1]/[dirname2]/[filename1]\n");
    printf("\t* rm -f [filename1]\n");
    printf("\t* rm -f [dirname1]/[dirname2]/[filename1]\n");
    printf("\t* rm -i [filename1]\n");
    printf("\t* rm -i [dirname1]/[dirname2]/[filename1]\n");
    printf("\t* rm -r [dirname1]/\n");
    printf("\t* rm -r [dirname1]/[dirname2]/[dirname3]/\n\n");
    printf(GRBOLD"================================\n"reset);
    printf(YELLOW"- tree:\n"reset);
    printf("\t* tree\n");
    printf("\t* tree [path]\n");
    printf(GRBOLD"================================\n"reset);
    printf("- polecenia z fork() i exec*()\n");
    printf("- obsluga historii polecen poprzez strzalki\n");
    printf(GRBOLD"================================\n"reset);
    printf(YELLOW"- obsluga sygnalow: \n"reset);
    printf("\t* CTRL + C\n");
    printf("\t* CTRL + Z\n");
    printf(GRBOLD"================================\n"reset);
    printf("- exit\n\n");
}/*end of help function*/

void handler(int sig){
    if(sig == SIGINT) printf("Interrupted the process\n");
    else if(sig == SIGTSTP) printf("Terminated the process. Hit enter to continue\n");
}/*end of handler function for signals*/

int gettingACommand(char *fullCommand, char **command, int size){
    char symbol[] = " \n";
    char *line;
    char *point = '\0';
    line = strtok(fullCommand, symbol);
    while(line != NULL){
        command[size] = line;
        line = strtok(NULL, symbol);
        size++;
    }
    command[size] = point;

    return size;
}/*end of gettingACommand function*/

void cd(char **command, int size){
    if((size == 1) || (strcmp(command[1],"~") == 0)){
        strcpy(prevPath, cwd);
        const char *home;
        if((home = getenv("HOME")) == NULL) home = getpwuid(getuid())->pw_dir;
        if(chdir(home) == -1) perror("An error has occurred - chdir() error");
        if(getcwd(cwd,sizeof(cwd)) == NULL) perror("An error has occurred - getcwd() error");
    }
    else if((size == 2) && (strcmp(command[1], "-") == 0)){
        if(chdir(prevPath) == -1) perror("An error has occurred - chdir() error");
        if(getcwd(cwd,sizeof(cwd)) == NULL) perror("An error has occurred - getcwd() error");
    }
    else {
        strcpy(prevPath, cwd);
        char *path;
        path = command[1];
        if(chdir(path) == -1) perror("An error has occurred - chdir() error");
        if(getcwd(cwd,sizeof(cwd)) == NULL) perror("An error has occurred - getcwd() error");
    }
}/*end of cd function*/

/*START OF MKDIR*/
void makeDirectory(char **command, int size){
    if(size < 2) printf("An error has occurred - wrong command - type 'help'\n");
    else{
        int brace = BRACE; /*{ */
        int clbrace = BRACECLOSED; /* }*/
        int slash = SLASH; /*/ */
        int bslash = BACKSLASH; /*\ */
        int quote = QUOTE; /* " " */
        int i, j, isBrace = 0, isClbrace = 0, isSlash = 0, isBslash = 0, isQuote = 0;

        for(i = 1; i < size; i++){
            for(j = 0; j < strlen(command[i]); j++){
                int code = (int)command[i][j];
                if(code == brace) isBrace++;
                if(code == clbrace) isClbrace++;
                if(code == slash) isSlash++;
                if(code == bslash) isBslash++;
                if(code == quote) isQuote++;
            }
        }

        if(size == 2){
            if(isBrace > 0){
                char *dirPaths[COMMAND_SIZE];
                char symbol[] = "{,}\n";
                char *line;
                int sizeOfDir = 0;
                line = strtok(command[1], symbol);

                while(line != NULL){
                    dirPaths[sizeOfDir] = line;
                    line = strtok(NULL, symbol);
                    sizeOfDir++;
                }
                for(i = 0; i < sizeOfDir; i++){
                    if(mkdir(dirPaths[i], 0777) == -1) perror("An error has occurred - mkdir() error");
                }
            }/*mkdir {dir1,dir2,dir3}*/
            else{
                if(mkdir(command[1], 0777) == -1) perror("An error has occurred - mkdir() error");
            }/*mkdir [name]*/
        }/*size == 2*/

        else if(size == 3){
            if(strcmp(command[1], "-p") == 0){
                char *dirPaths[COMMAND_SIZE];
                char symbol[] = "/ \n";
                char *line;
                char previous[PATH_MAX];
                int sizeOfDir = 0;
                line = strtok(command[2], symbol);

                while(line != NULL){
                    dirPaths[sizeOfDir] = line;
                    line = strtok(NULL, symbol);
                    sizeOfDir++;
                }

                if(isSlash > 0 && isBrace == 0){
                    if(getcwd(previous,sizeof(previous)) == NULL) perror("An error has occurred - getcwd() error");

                    for(i = 0; i < sizeOfDir; i++){
                        if(mkdir(dirPaths[i], 0777) == -1) perror("An error has occurred - mkdir() error");
                        if(chdir(dirPaths[i]) == -1) perror("An error has occurred - chdir() error");
                    }
                    if(chdir(previous) == -1) perror("An error has occurred - chdir() error");
                }/*mkdir -p dir1/dir2/dir3*/

                else if(isSlash > 0 && isBrace > 0){
                    char *dirPathsH[COMMAND_SIZE];
                    char symbolH[] = "{,} \n";
                    char *lineH;
                    int sizeOfDirH = 0;
                    lineH = strtok(dirPaths[sizeOfDir-1], symbolH);

                    while(lineH != NULL){
                        dirPathsH[sizeOfDirH] = lineH;
                        lineH = strtok(NULL, symbolH);
                        sizeOfDirH++;
                    }

                    if(getcwd(previous,sizeof(previous)) == NULL) perror("An error has occurred - getcwd() error");

                    for(i = 0; i < sizeOfDir-1; i++) {
                        if (mkdir(dirPaths[i], 0777) == -1) perror("An error has occurred - mkdir() error");
                        if (chdir(dirPaths[i]) == -1) perror("An error has occurred - chdir() error");
                    }

                    for(i = 0; i < sizeOfDirH; i++){
                        if (mkdir(dirPathsH[i], 0777) == -1) perror("An error has occurred - mkdir() error");
                    }

                    if(chdir(previous) == -1) perror("An error has occurred - chdir() error");
                }/*mkdir -p dir1/dir2/{dir3,dir4}*/
                else{
                    if(mkdir(command[2], 0777) == -1) perror("An error has occurred - mkdir() error");
                }
            }/*command[1] = -p*/
            else{
                for(i = 1; i < size; i++){
                    if (mkdir(command[i], 0777) == -1) perror("An error has occurred - mkdir() error");
                }
            }
        }/*size == 3*/
        else{
            for(i = 1; i < size; i++){
                if (mkdir(command[i], 0777) == -1) perror("An error has occurred - mkdir() error");
            }
        }/*size > 3*/
    }/*size > 2*/
}/*end of makeDirectory function*/
/*END OF MKDIR*/

/*START OF RM*/
int removeDir(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf){
    int rv = remove(fpath);
    if(rv == -1) perror("An error has occurred - remove() error");
    return rv;
}/*end of removeDir function - for nftw()*/

void removeDirectory(char **command, int size){
    if(size < 2) printf("An error has occurred - wrong command - type 'help'\n");
    else{
        char previous[PATH_MAX];
        char *decision;
        char printPath[BUFFER_SIZE] = "";
        int slash = SLASH; /*/ */
        int dot = DOT; /* . */
        int i, j, isSlash = 0, isDot = 0;

        for(i = 1; i < size; i++){
            for(j = 0; j < strlen(command[i]); j++){
                int code = (int)command[i][j];
                if(code == slash) isSlash++;
                if(code == dot) isDot++;
            }
        }

        if(strcmp(command[0], "rmdir") == 0){
            char *dirPaths[COMMAND_SIZE];
            char symbol[] = "/ \n";
            char *line;
            int sizeOfDir = 0;
            line = strtok(command[1], symbol);

            while(line != NULL) {
                dirPaths[sizeOfDir] = line;
                line = strtok(NULL, symbol);
                sizeOfDir++;
            }

            if(getcwd(previous,sizeof(previous)) == NULL) perror("An error has occurred - getcwd() error");

            if(sizeOfDir > 1) {
                for (i = 0; i < sizeOfDir-1; i++) {
                    if (chdir(dirPaths[i]) == -1) perror("An error has occurred - chdir() error");
                }
            }
            if(rmdir(dirPaths[sizeOfDir-1]) == -1) perror("An error has occurred - rmdir() error");

            if(chdir(previous) == -1) perror("An error has occurred - chdir() error");
        }/*end of rmdir if*/
        else{
            if(size == 2){
                if(isDot > 0) {
                    if(isSlash > 0){
                        char *dirPaths[COMMAND_SIZE];
                        char symbol[] = "/ \n";
                        char *line;
                        int sizeOfDir = 0;
                        line = strtok(command[1], symbol);

                        while(line != NULL) {
                            dirPaths[sizeOfDir] = line;
                            line = strtok(NULL, symbol);
                            sizeOfDir++;
                        }

                        if(getcwd(previous,sizeof(previous)) == NULL) perror("An error has occurred - getcwd() error");

                        for (i = 0; i < sizeOfDir-1; i++) {
                            if (chdir(dirPaths[i]) == -1) perror("An error has occurred - chdir() error");
                        }

                        if (remove(dirPaths[sizeOfDir - 1]) == -1) perror("An error has occurred - rm() error");
                        if (chdir(previous) == -1) perror("An error has occurred - chdir() error");
                    }/*isSlash > 0*/
                    else{
                        if (remove(command[1]) == -1) perror("An error has occurred - rm() error");
                    }/*is only dot*/
                }
                else{
                    printf("You cannot delete directory. Enter 'help'\n");
                }
            }/*end of size == 2*/

            else if(size == 3){
                if(strcmp(command[1], "-f") == 0){
                    if(isDot > 0) {
                        if(isSlash > 0){
                            char *dirPaths[COMMAND_SIZE];
                            char symbol[] = "/ \n";
                            char *line;
                            int sizeOfDir = 0;
                            line = strtok(command[2], symbol);

                            while(line != NULL) {
                                dirPaths[sizeOfDir] = line;
                                line = strtok(NULL, symbol);
                                sizeOfDir++;
                            }

                            if(getcwd(previous,sizeof(previous)) == NULL) perror("An error has occurred - getcwd() error");

                            for (i = 0; i < sizeOfDir-1; i++) {
                                if (chdir(dirPaths[i]) == -1) perror("An error has occurred - chdir() error");
                            }

                            if (remove(dirPaths[sizeOfDir - 1]) == -1) perror("An error has occurred - rm() error");
                            if (chdir(previous) == -1) perror("An error has occurred - chdir() error");
                        }/*isSlash > 0*/
                        else{
                            if(remove(command[2]) == -1) perror("An error has occurred - rm() error");
                        }/*is only dot*/
                    }
                    else{
                        printf("You cannot delete directory. Enter 'help'\n");
                    }
                }/*end of -f force command*/

                else if(strcmp(command[1], "-i") == 0){
                    if(isDot > 0) {
                        if(isSlash > 0){
                            char *dirPaths[COMMAND_SIZE];
                            char symbol[] = "/ \n";
                            char *line;
                            int sizeOfDir = 0;
                            line = strtok(command[2], symbol);

                            while(line != NULL) {
                                dirPaths[sizeOfDir] = line;
                                line = strtok(NULL, symbol);
                                sizeOfDir++;
                            }

                            if(getcwd(previous,sizeof(previous)) == NULL) perror("An error has occurred - getcwd() error");

                            for (i = 0; i < sizeOfDir-1; i++) {
                                if (chdir(dirPaths[i]) == -1) perror("An error has occurred - chdir() error");
                            }

                            int ret = snprintf(printPath, sizeof(printPath), YELBOLD"Are you sure you want to delete %s?"reset" y/n: ", dirPaths[sizeOfDir-1]);
                            if(ret < 0) abort();

                            decision = readline(printPath);

                            if(strcmp(decision, "y") == 0) {
                                if (remove(dirPaths[sizeOfDir - 1]) == -1) perror("An error has occurred - rm() error");
                            }

                            if (chdir(previous) == -1) perror("An error has occurred - chdir() error");
                        }/*isSlash > 0*/
                        else{
                            int ret = snprintf(printPath, sizeof(printPath), YELBOLD"Are you sure you want to delete %s?"reset" y/n: ", command[2]);
                            if(ret < 0) abort();

                            decision = readline(printPath);
                            if(strcmp(decision, "y") == 0) {
                                if (remove(command[2]) == -1) perror("An error has occurred - rm() error");
                            }
                        }/*is only dot*/
                    }
                    else{
                        printf("You cannot delete directory. Enter 'help' for more information\n");
                    }
                }/*end of -i command*/

                else if(strcmp(command[1], "-r") == 0){
                    char *dirPaths[COMMAND_SIZE];
                    char symbol[] = "/ \n";
                    char *line;
                    int sizeOfDir = 0;
                    line = strtok(command[2], symbol);

                    while(line != NULL) {
                        dirPaths[sizeOfDir] = line;
                        line = strtok(NULL, symbol);
                        sizeOfDir++;
                    }

                    if(getcwd(previous,sizeof(previous)) == NULL) perror("An error has occurred - getcwd() error");

                    if(sizeOfDir > 1) {
                        for (i = 0; i < sizeOfDir - 1; i++) {
                            if (chdir(dirPaths[i]) == -1) perror("An error has occurred - chdir() error");
                        }
                    }

                    nftw(dirPaths[sizeOfDir-1], removeDir, 64, FTW_DEPTH | FTW_PHYS);

                    if (chdir(previous) == -1) perror("An error has occurred - chdir() error");
                }/*end of -r recursive command*/
            }/*end of size == 3*/
        }/*end of rm if*/
    }/*end of size >= 2*/
}/*end of removeDirectory function*/
/*END OF RM*/

/*START OF TREE*/
void treeRecursive(char *path, int start){
    char nPath[PATH_MAX];
    struct dirent *entry;
    DIR *dir = opendir(path);

    if (!dir) return;

    while ((entry = readdir(dir)) != NULL){
        if ((strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0)){
            int i;
            for (i=0; i < start; i++){
                if ((i == 0) || (i%2 == 0)) printf("|");
                else printf(" ");
            }

            printf("+-%s\n", entry->d_name);

            strcpy(nPath, path);
            strcat(nPath, "/");
            strcat(nPath, entry->d_name);
            treeRecursive(nPath, start + 2);
        }
    }
    closedir(dir);
}/*end of treeRecursive function*/

void tree(char **command, int size){
    if(size == 1){
        int start = 0;
        if(getcwd(cwd,sizeof(cwd)) == NULL) perror("An error has occurred - getcwd() error");
        treeRecursive(cwd, start);
    }
    else if(size == 2){
        char path[PATH_MAX];
        strcpy(path, command[1]);
        int start = 0;
        treeRecursive(path, start);
    }
    else printf("Type 'help' for getting more information about command tree");
}/*end of tree function*/
/*END OF TREE*/

int main(int argc, char *argv[]){
    /*VARIABLES*/
    char *name;
    char hostname[BUFFER_SIZE];
    char *command[COMMAND_SIZE];
    char printPath[BUFFER_SIZE] = "";
    char *fullCommand;

    name = getlogin();
    if(name == NULL) perror("An error has occurred - getlogin() error");
    if(gethostname(hostname, BUFFER_SIZE) == -1) perror("An error has occurred - gethostname() error");

    /*CTRL C*/
    if(signal(SIGINT, handler) == SIG_ERR) perror("An error has occurred - signal() error");

    /*CTRL Z*/
    struct sigaction sa;
    sa.sa_handler = &handler;
    sa.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &sa, NULL);

    if(getcwd(cwd,sizeof(cwd))==NULL) perror ("An error has occurred - getcwd() error");
    else {
        strcpy(prevPath, cwd);
        /*main shell*/
        while (1) {
            /*VARIABLES*/
            int size = 0;
            int pid;
            int ret = snprintf(printPath, sizeof(printPath),"["GRBOLD"%s@%s"reset":"YELBOLD"%s"reset"]"reset"$ ", name, hostname, cwd);

            if(ret < 0) abort();

            fullCommand = readline(printPath);
            add_history(fullCommand);

            size = gettingACommand(fullCommand, command, size);

            if(size == 0) continue;
            else {
                /*help command*/
                if (strcmp(command[0], "help") == 0) help();

                /*cd command*/
                else if (strcmp(command[0], "cd") == 0) cd(command, size);

                /*mkdir command*/
                else if (strcmp(command[0], "mkdir") == 0) makeDirectory(command, size);

                /*rmdir command*/
                else if (strcmp(command[0], "rm") == 0 || strcmp(command[0], "rmdir") == 0) removeDirectory(command, size);

                /*cat command*/
                else if (strcmp(command[0], "tree") == 0) tree(command, size);

                /*exit command*/
                else if (strcmp(command[0], "exit") == 0) exit(0);

                /*executing command using fork and exec*()*/
                else {
                    pid = fork();
                    if (pid < 0) perror("An error has occurred - fork() error");
                    else if (pid == 0) {
                        if (size == 1) {
                            if (execvp(command[0], argv) == -1) perror("An error has occurred - execvp() error");
                            exit(0);
                        }
                        else {
                            if (execvp(command[0], command) == -1) perror("An error has occurred - execvp() error");
                            exit(0);
                        }
                    } else wait(NULL);
                }
            }
            /*free an array*/
            free(fullCommand);
        }/*end of while*/
    }/*end of else of getcwd error*/
    return 0;
}/*end of main function*/
