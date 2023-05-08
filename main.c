#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h> 

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

int is_c_file(char* file_name) {
    int len = strlen(file_name);
    if (strcmp(file_name + len - 2, ".c") == 0) {
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    struct stat file_stat;
    pid_t pid_switch, process_forkfile;


    if (argc == 1) {
        printf("Not multiple cmd agr");
        return EXIT_FAILURE;
    } else {
        for (int i = 1; i < argc; i++) {
            lstat(argv[i], &file_stat);
            pid_t pid = fork();
            if(pid<0){
                perror("Forking error... Exiting ...\n");
                exit(EXIT_FAILURE);
            }
            /*
            if (S_ISREG(file_stat.st_mode))
            {
                if(is_c_file(argv[i])){
                    printf("C file");
                }
                else{
                printf("%s is a regular file\n", argv[i]);
                print_access_rights(file_stat);
                printf("\n");
                }
             } else if (S_ISDIR(file_stat.st_mode)){
                printf("%s is a directory\n", argv[i]);
                count_c_files(argv[i]);
                DIR* dir = opendir(argv[i]);
                struct dirent* ent;
                            if (dir != NULL) {
                while ((ent = readdir(dir)) != NULL) {
                    char file_name[1024];
                    strcpy(file_name, argv[i]);
                    strcat(file_name, "/");
                    strcat(file_name, ent->d_name);
                    if (is_c_file(ent->d_name)) {
                        process_forkfile = fork();
                        if (process_forkfile == -1) {
                            printf("Fork failed.\n");
                            return EXIT_FAILURE;
                        } else if (process_forkfile == 0) {
                            printf("Child process %d: \n", getpid());
                            printf("%s is a C file.\n", file_name);
                            printf("The link name and size of %s are: \n", file_name);
                            get_link_name(file_name);
                            get_link_size(file_name);
                            printf("\n");
                            exit(EXIT_SUCCESS);
                        } else {
                            wait(100);
                        }
                    }
                }
                closedir(dir);
            } else {
                printf("Error opening dir\n");
            }
        } else if (S_ISLNK(file_stat.st_mode)) {
            printf("%s is a symbolic link\n", argv[i]);
            printf("The link name and size of %s are: \n", argv[i]);
            get_link_name(argv[i]);
            get_link_size(argv[i]);
            printf("\n");
        } else {
            printf("%s is neither a regular file, directory, nor a symbolic link\n", argv[i]);
        }
        */
    }
}
return EXIT_SUCCESS;
}