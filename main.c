#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>

void print_access_rights(struct stat file);
void get_link_name(char* file);
void get_link_size(char* file);
char get_file_type(struct stat);
void count_c_files(char* dir_name);
void reg_menu(struct stat file_stat,char *name);
void symbolic_menu(struct stat file_stat,char *name);
void directory_menu(struct stat file_stat,char *name);
void handle_non_c_file(char* name);
void handle_c_file(char* name);
void handle_symbolic_link_option(char *name);
void handle_directory(char *name);


int main(int argc, char* argv[]) {
    struct stat file_stat;
    int pfd[2];

    if (argc == 1) {
        printf("Not multiple cmd agr");
        return EXIT_FAILURE;
    } else {

        int status;

        for (int i = 1; i < argc; i++) {
            pid_t pid = fork();
            lstat(argv[i], &file_stat);

            if(pid<0){
                perror("Forking error... Exiting ...\n");
                exit(EXIT_FAILURE);
            }else if(pid==0){


                switch (get_file_type(file_stat)) {
                    case 'r':
                        //printf("\nWE HAVE REG FILE\n");
                        reg_menu(file_stat, argv[i]);
                        break;
                    case 'd':
                        directory_menu(file_stat,argv[i]);
                        break;
                    case 's':
                        symbolic_menu(file_stat,argv[i]);
                        break;
                    case 'u':
                        printf("Unknown file type");
                        exit(EXIT_FAILURE);
                    default:
                        printf("Unknown file type\n");
                        exit(EXIT_FAILURE);
                }
                exit(EXIT_SUCCESS);
            }else{
                // Create child process to handle C files or directories
                if(pipe(pfd)<0){
                    printf("The pipe was created");
                    exit(1);
                }
                pid_t pid2 = fork();
                if (pid2 < 0) {
                    perror("fork");
                    exit(EXIT_FAILURE);
                }else if(pid2==0){
                    switch (get_file_type(file_stat)) {
                        case 'r':
                            if (strstr(argv[i], ".c") != NULL) {
                                close(pfd[0]);
                                dup2(pfd[1], 1);
                               handle_c_file(argv[i]);
                            } else {
                                handle_non_c_file(argv[i]);
                            }
                            break;
                        case 'd':
                              handle_directory(argv[i]);
                            break;
                        case 's':
                             handle_symbolic_link_option(argv[i]);
                            break;
                        default:
                            exit(EXIT_SUCCESS);
                    }
                    exit(EXIT_SUCCESS);

                }else{
                    int status2;
                    waitpid(pid2, &status2, 0);
                    printf("The process with PID %d has ended with the exit code %d\n", pid2, status2);

                }

                switch (get_file_type(file_stat)){
                    case 'r':
                        if(strstr(argv[i], ".c") != NULL){
                            close(pfd[1]);
                            char ch[1];
                            char string[255];
                            int noOfErrors = 0, noOfWarnings = 0;
                            int nrBytesRead=0;
                            while(read(pfd[0],ch,1)>0){
                                string[nrBytesRead]=ch[0];
                                nrBytesRead++;
                            }
                            string[nrBytesRead]='\0';
                            char *ptr;
                            noOfErrors=strtol(string,&ptr,10);
                            noOfWarnings= strtol(ptr,NULL,10);
                            printf("\nNumber of Errors: %d\nNumber of Warnings: %d\n", noOfErrors, noOfWarnings);
                            double score;
                            if(noOfErrors > 0) {
                                score = 1;
                            } else{
                                if(noOfWarnings == 0) {
                                    score = 10;
                                } else if(noOfWarnings > 10) {
                                    score = 2;
                                } else{
                                    score = 2 + 8 * (10 - noOfWarnings) / 10.0;
                                }
                            }

                            char scoreString[10];
                            sprintf(scoreString, "%.2f", score);

                            char fileName[52];
                            sprintf(fileName, "%s:%s", argv[i], scoreString);

                            FILE *fp = fopen("grades.txt", "a");
                            if (fp == NULL) {
                                perror("\nError opening file grades.txt\n");
                                exit(1);
                            }
                            fprintf(fp, "%s\n", fileName);
                            fclose(fp);

                            printf("\nScore computed and saved in grades.txt.\n");
                            close(pfd[0]);

                        }break;
                    default:
                        exit(EXIT_SUCCESS);
                }

                waitpid(pid, &status, 0);
                printf("The process with PID %d has ended with the exit code %d\n", pid, status);


            }
        }
    }
    return EXIT_SUCCESS;
}


void handle_non_c_file(char* name){
    execlp("wc", "wc", "-l", name, (char *)0);
}

void handle_directory(char *name) {
    char dirPath[4096];
    snprintf(dirPath, sizeof(dirPath), "%s/", name);

    // create the directory with the specified permissions
    if (mkdir(dirPath, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_ISVTX | S_IXOTH) < 0) {
        perror("mkdir");
        exit(1);
    }
}
void handle_symbolic_link_option(char* name){
    // set user permissions to read, write, and execute
    chmod(name, S_IRWXU);

// set group permissions to read and write (no execute)
    chmod(name, S_IRGRP | S_IWGRP);

// remove all permissions for other users
    chmod(name, 0);
}

void print_access_rights(struct stat file) {
    printf("USER:\n");
    if (file.st_mode & S_IRUSR) {
        printf("Read:YES\n");
    } else {
        printf("Read:NO\n");
    }
    if (file.st_mode & S_IWUSR) {
        printf("Write:YES\n");
    } else {
        printf("Write:NO\n");
    }
    if (file.st_mode & S_IXUSR) {
        printf("Execution:YES\n");
    } else {
        printf("Execution:NO\n");
    }

    printf("\nGROUP:\n");
    if (file.st_mode & S_IRGRP) {
        printf("Read:YES\n");
    } else {
        printf("Read:NO\n");
    }
    if (file.st_mode & S_IWGRP) {
        printf("Write:YES\n");
    } else {
        printf("Write:NO\n");
    }
    if (file.st_mode & S_IXGRP) {
        printf("Execution:YES\n");
    } else {
        printf("Execution:NO\n");
    }

    printf("\nOTHERS:\n");
    if (file.st_mode & S_IROTH) {
        printf("Read:YES\n");
    } else {
        printf("Read:NO\n");
    }
    if (file.st_mode & S_IWOTH) {
        printf("Write:YES\n");
    } else {
        printf("Write:NO\n");
    }
    if (file.st_mode & S_IXOTH) {
        printf("Execution:YES\n");
    } else {
        printf("Execution:NO\n");
    }
}

void get_link_name(char* file) {
    char buffer[1024];
    readlink(file, buffer, sizeof(buffer) - 1);
    printf("The link name of %s is %s\n ", file, buffer);
}

void get_link_size(char* file) {
    char buffer[1024];
    readlink(file, buffer, sizeof(buffer) - 1);
    struct stat target;
    lstat(buffer, &target);
    printf("The link size of %s is %ld\n ", file, target.st_size);
}

void count_c_files(char* dir_name) {
    DIR* dir;
    struct dirent* ent;
    int count = 0;
    int len;
    dir = opendir(dir_name);

    if (dir != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            len = strlen(ent->d_name);
            if (strcmp(ent->d_name + len - 2, ".c") == 0) {
                count++;
            }
        }
        closedir(dir);
    } else {
        printf("Error opening dir\n");
    }

    printf("The number of c files is %d\n", count);
}


void symbolic_menu(struct stat file_stat,char* name) {
    char input[32];
    printf("B) Symbolic file:\n");
    printf("\t-n: name\n");
    printf("\t-d: size of symbolic link\n");
    printf("\t-l: delete symbolic link\n");
    printf("\t-t: size of target file\n");
    printf("\t-a: access rights\n");

    printf("Enter your choice: \n");
    fgets(input,sizeof(input),stdin);

    printf("\nYOUR OPTION: %s\n", input);
    for(int i = 0; i < strlen(input); i++){
        switch (input[i]) {
            case 'n':
                get_link_name(name);
                break;
            case 'd':
                get_link_size(name);
                break;
            case 'a':
                print_access_rights(file_stat);
                break;
            case 'l':
                printf("Unlinking...\n");
                if(unlink(name)==-1){
                    printf("Error deleting...\n");
                    exit(EXIT_FAILURE);
                }
                printf("Unlinked !");
                break;
            case 't':
                printf("Size is : %ld \n ",file_stat.st_size);
                break;
            default:
                //printf("Invalid input. Please try again.\n");
                break;
        }
    }
}
void reg_menu(struct stat file_stat,char* name) {
    char link_name[100];
    char input[32];
    printf("A) Regular file:\n");
    printf("\t-n: name\n");
    printf("\t-d: size\n");
    printf("\t-h: hard link count\n");
    printf("\t-m: time of last modification\n");
    printf("\t-a: access rights\n");
    printf("\t-l: create symbolic link\n");
    printf("Enter your choice: \n");
    fgets(input,sizeof(input),stdin);

    printf("\nYOUR OPTION: %s\n", input);
    time_t mtime = file_stat.st_mtime;
    struct tm* mtime_tm = localtime(&mtime);
    char mtime_str[100];
    strftime(mtime_str, 100, "%Y-%m-%d %H:%M:%S", mtime_tm);
    for(int i = 0; i < strlen(input); i++){
        switch (input[i]) {
            case 'n':
                printf("Name: %s\n", name);
                break;
            case 'd':
                printf("Size: %ld bytes\n", file_stat.st_size);
                break;
            case 'h':
                printf("Hard link count: %ld\n", file_stat.st_nlink);
                break;
            case 'm':
                // print time of last modification
                printf("Last modified: %s\n", mtime_str);


                break;
            case 'a':
                print_access_rights(file_stat);
                break;
            case 'l':
                printf("Enter name of symbolic link: ");
                scanf("%s", link_name);
                symlink(name, link_name);
                printf("Symbolic link created.\n");
                break;
            default:
                //printf("Invalid input. Please try again.\n");
                break;
        }
    }
}


void directory_menu(struct stat file_stat,char* name) {
    char input[32];
    printf("C) Directory:\n");
    printf("\t-n: name\n");
    printf("\t-d: size of directory\n");
    printf("\t-c: total number of files with the .c extension\n");
    printf("\t-a: access rights\n");

    printf("Enter your choice: \n");
    fgets(input,sizeof(input),stdin);

    printf("\nYOUR OPTION: %s\n", input);
    for(int i = 0; i < strlen(input); i++){
        switch (input[i]) {
            case 'n':
                printf("Directory name is : %s \n",name);
                break;
            case 'd':
                printf("Size of directory is : %ld \n",file_stat.st_size);
                break;
            case 'a':
                print_access_rights(file_stat);
                break;
            case 'c':
                count_c_files(name);
                break;
            default:
                //printf("Invalid input. Please try again.\n");
                break;
        }
    }
}
char get_file_type(struct stat file_stat) {
    /*struct stat sb;

    if (stat(path, &sb) == -1) {
        perror("stat");
        exit(EXIT_FAILURE);
    }*/

    if (S_ISLNK(file_stat.st_mode)) {
        return 's'; // symbolic link
    } else if (S_ISDIR(file_stat.st_mode)) {
        return 'd'; // directory
    } else if (S_ISREG((file_stat.st_mode))) {
        return 'r'; // regular file
    } else {
        return 'u'; // unknown file type
    }
}

void handle_c_file(char* name){
    execlp("bash", "bash", "script_c_file.sh", name, (char *)0);

}