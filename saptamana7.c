#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>

#define BUFFSIZE 1024

void writeWidthAndHeight(const char *path, int outputFile) {
    uint32_t width;
    uint32_t height;
    char buffer[BUFFSIZE];
    int inputFile;
    int bytesRead;

    if ((inputFile = open(path, O_RDONLY)) < 0) {
        perror("Unable to open BMP file");
        exit(EXIT_FAILURE);
    }

    if ((bytesRead = read(inputFile, buffer, BUFFSIZE)) != 0) {
        width = *(uint32_t *)(buffer + 18);
        sprintf(buffer, "Width: %d\n", width);
        write(outputFile, buffer, strlen(buffer));

        height = *(uint32_t *)(buffer + 22);
        sprintf(buffer, "Height: %d\n", height);
        write(outputFile, buffer, strlen(buffer));
    }
    close(inputFile);
}

void writeFileInfo(int outputFile) {
    struct stat fileInfo;
    char buffer[BUFFSIZE];

    if (fstat(outputFile, &fileInfo) == -1) {
        perror("Error obtaining file information");
        exit(EXIT_FAILURE);
    }

    sprintf(buffer, "Size: %ld\n", fileInfo.st_size);
    write(outputFile, buffer, strlen(buffer));

    sprintf(buffer, "User ID: %d\n", fileInfo.st_uid);
    write(outputFile, buffer, strlen(buffer));

    sprintf(buffer, "Last Modification Time: %s\n", ctime(&fileInfo.st_mtime));
    write(outputFile, buffer, strlen(buffer));

    sprintf(buffer, "Number of Links: %ld\n", fileInfo.st_nlink);
    write(outputFile, buffer, strlen(buffer));
}

void writePermission(char *permissionType, mode_t permission, int outputFile) {
    char buffer[BUFFSIZE];

    sprintf(buffer, "%s: ", permissionType);
    write(outputFile, buffer, strlen(buffer));

    if (permission & S_IRUSR) {
        sprintf(buffer, "R");
        write(outputFile, buffer, strlen(buffer));
    } else {
        sprintf(buffer, "-");
        write(outputFile, buffer, strlen(buffer));
    }

    if (permission & S_IWUSR) {
        sprintf(buffer, "W");
        write(outputFile, buffer, strlen(buffer));
    } else {
        sprintf(buffer, "-");
        write(outputFile, buffer, strlen(buffer));
    }

    if (permission & S_IXUSR) {
        sprintf(buffer, "X\n");
        write(outputFile, buffer, strlen(buffer));
    } else {
        sprintf(buffer, "-\n");
        write(outputFile, buffer, strlen(buffer));
    }
}

void writeAllPermissions(int outputFile, struct stat filePermission) {
    writePermission("User Access Rights", (filePermission.st_mode & S_IRWXU), outputFile);
    writePermission("Group Access Rights", (filePermission.st_mode & S_IRWXG), outputFile);
    writePermission("Others Access Rights", filePermission.st_mode, outputFile);
}

void processFile(const char *filePath, int outputFile) {
    char aux[100];
    strcpy(aux, filePath);
    char *extension = strrchr(aux, '.');

    if (extension != NULL && strcmp(extension, ".bmp") == 0) {
        sprintf(aux, "\nFile Name: %s\n", filePath);
        write(outputFile, aux, strlen(aux));
        writeWidthAndHeight(filePath, outputFile);
        writeFileInfo(outputFile);
        writeAllPermissions(outputFile, (struct stat){});
    } else if (S_ISREG((struct stat){}.st_mode)) {
        sprintf(aux, "\nFile Name: %s\n", filePath);
        write(outputFile, aux, strlen(aux));
        writeFileInfo(outputFile);
        writeAllPermissions(outputFile, (struct stat){});
    }
}

void processDirectory(const char *directoryPath, int outputFile) {
    struct stat checkFile;
    struct dirent *entry;
    char buffer[BUFFSIZE];

    DIR *directory = opendir(directoryPath);

    if (directory == NULL) {
        perror("Invalid directory path!\n");
        exit(EXIT_FAILURE);
    }

    sprintf(buffer, "Directory Name: %s\n", directoryPath);
    write(outputFile, buffer, strlen(buffer));

    stat(directoryPath, &checkFile);
    sprintf(buffer, "User ID: %d\n", checkFile.st_uid);
    write(outputFile, buffer, strlen(buffer));

    writeAllPermissions(outputFile, checkFile);

    while ((entry = readdir(directory)) != 0) {
        char path[BUFFSIZE];
        sprintf(path, "%s/%s", directoryPath, entry->d_name);

        if (lstat(path, &checkFile) != 0) {
            perror("Invalid path\n");
            exit(EXIT_FAILURE);
        }

        if (S_ISREG(checkFile.st_mode)) {
            processFile(path, outputFile);
        } else if (S_ISDIR(checkFile.st_mode)) {
            processDirectory(path, outputFile);
        }
    }

    closedir(directory);
}

int main(int argc, char *argv[]) {
    int outputFile;
    char buffer[BUFFSIZE];

    if (argc != 2) {
        perror("Insufficient arguments");
        exit(EXIT_FAILURE);
    }

    if ((outputFile = open("statistics.txt", O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU)) < 0) {
        perror("Unable to create output file");
        exit(EXIT_FAILURE);
    }

    struct stat checkFile;

    if (lstat(argv[1], &checkFile) != 0) {
        perror("Unable to get file status!\n");
        exit(EXIT_FAILURE);
    }

    if (S_ISREG(checkFile.st_mode)) {
        processFile(argv[1], outputFile);
    } else if (S_ISDIR(checkFile.st_mode)) {
        processDirectory(argv[1], outputFile);
    } else if (S_ISLNK(checkFile.st_mode)) {
        // Handle symbolic link if needed
    }

    close(outputFile);

    return 0;
}
