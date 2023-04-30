#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>

#define MAX_OPTIONS 10
#define BUFFER_SIZE 1024

void print_access_rights(mode_t mode) {
    printf("User:\n");
    printf("Read - %s\n", (mode & S_IRUSR) ? "yes" : "no");
    printf("Write - %s\n", (mode & S_IWUSR) ? "yes" : "no");
    printf("Exec - %s\n", (mode & S_IXUSR) ? "yes" : "no");
    printf("Group:\n");
    printf("Read - %s\n", (mode & S_IRGRP) ? "yes" : "no");
    printf("Write - %s\n", (mode & S_IWGRP) ? "yes" : "no");
    printf("Exec - %s\n", (mode & S_IXGRP) ? "yes" : "no");
    printf("Others:\n");
    printf("Read - %s\n", (mode & S_IROTH) ? "yes" : "no");
    printf("Write - %s\n", (mode & S_IWOTH) ? "yes" : "no");
    printf("Exec - %s\n", (mode & S_IXOTH) ? "yes" : "no");
}

void handle_regular_file(char* path) {
    struct stat st;
    if (lstat(path, &st) == -1) {
        perror("lstat");
        exit(EXIT_FAILURE);
    }
    printf("Regular file: %s (%c)\n", path, (S_ISLNK(st.st_mode)) ? 'l' : '-');
    char options[MAX_OPTIONS];
    printf("Options: ");
    scanf("%s", options);
    int len = strlen(options);
    if (strchr(options, '-') == NULL || len < 2) {
        printf("Error: invalid option\n");
        handle_regular_file(path);
        return;
    }
    if (strchr(options, 'n') != NULL) {
        printf("Name: %s\n", path);
    }
    if (strchr(options, 'd') != NULL) {
        printf("Size: %ld bytes\n", st.st_size);
    }
    if (strchr(options, 'h') != NULL) {
        printf("Hard link count: %ld\n", st.st_nlink);
    }
    if (strchr(options, 'm') != NULL) {
        printf("Time of last modification: %s", ctime(&st.st_mtime));
    }
    if (strchr(options, 'a') != NULL) {
        printf("Access rights:\n");
        print_access_rights(st.st_mode);
    }
    if (strchr(options, 'l') != NULL && S_ISREG(st.st_mode)) {
        printf("Enter symbolic link name: ");
        char linkname[BUFFER_SIZE];
        scanf("%s", linkname);
        if (symlink(path, linkname) == -1) {
            perror("symlink");
            exit(EXIT_FAILURE);
        }
        printf("Symbolic link created: %s -> %s\n", linkname, path);
    }
    if (strchr(options, 'l') != NULL && S_ISLNK(st.st_mode)) {
        char linkname[256];
        ssize_t len = readlink(path, linkname, sizeof(linkname)-1);
        if (len == -1) {
            perror("readlink");
            exit(EXIT_FAILURE);
        }
        linkname[len] = '\0';
        printf("Name: %s\n", path);
        printf("Symbolic Link: %s\n", linkname);
        return;
    }

    printf("Invalid option for symbolic link\n");
}

void handle_directory(char *path, char *options) {
    struct stat st;
    if (stat(path, &st) == -1) {
        perror("stat");
        exit(EXIT_FAILURE);
    }

    printf("Name: %s\n", path);
    printf("Size: %ld bytes\n", st.st_size);
    print_access_rights(path);

    DIR *dirp;
    struct dirent *dp;
    int count = 0;

    dirp = opendir(path);
    if (dirp == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    while ((dp = readdir(dirp)) != NULL) {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
            char filename[256];
            sprintf(filename, "%s/%s", path, dp->d_name);
            struct stat st;
            if (stat(filename, &st) == -1) {
                perror("stat");
                exit(EXIT_FAILURE);
            }

            if (S_ISREG(st.st_mode)) {
                if (strstr(dp->d_name, ".c") != NULL) {
                    count++;
                }
            } else if (S_ISDIR(st.st_mode)) {
                handle_directory(filename, "");
            }
        }
    }

    closedir(dirp);
    printf("Total number of files with the .c extension: %d\n", count);
}

int main(int argc, char *argv[]) {
    pid_t pid;
    int i, status;

    if (argc < 2) {
        printf("Usage: %s <file/directory/symbolic_link> ...\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    for (i = 1; i < argc; i++) {
        pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Child process
            struct stat st;
            if (lstat(argv[i], &st) == -1) {
                perror("lstat");
                exit(EXIT_FAILURE);
            }

            // Check file type and call appropriate function
            if (S_ISREG(st.st_mode) && has_c_extension(argv[i])) {
                handle_regular_file_with_c_extension(argv[i]);
            } else if (S_ISREG(st.st_mode)) {
                handle_regular_file(argv[i]);
            } else if (S_ISDIR(st.st_mode)) {
                handle_directory(argv[i]);
                pid = fork();
                if (pid == -1) {
                    perror("fork");
                    exit(EXIT_FAILURE);
                } else if (pid == 0) {
                    // Second child process for creating a file
                    char filename[MAX_PATH_LEN];
                    snprintf(filename, MAX_PATH_LEN, "%s_file.txt", argv[i]);
                    char *args[] = {"touch", filename, NULL};
                    execvp(args[0], args);
                    perror("execvp");
                    exit(EXIT_FAILURE);
                } else {
                    wait(&status);
                    if (WIFEXITED(status)) {
                        printf("File creation status: %d\n", WEXITSTATUS(status));
                    } else {
                        printf("File creation failed\n");
                    }
                }
            } else if (S_ISLNK(st.st_mode)) {
                handle_symbolic_link(argv[i]);
            } else {
                printf("Unknown file type\n");
                exit(EXIT_FAILURE);
            }

            exit(EXIT_SUCCESS);
        }
    }

    // Parent process waits for child processes to finish
    for (i = 1; i < argc; i++) {
        wait(&status);
        if (WIFEXITED(status)) {
            printf("Child process %d exited with status %d\n", i, WEXITSTATUS(status));
        } else {
            printf("Child process %d failed\n", i);
        }
    }

    return 0;
}

