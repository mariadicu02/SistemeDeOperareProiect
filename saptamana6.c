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

#define BUFFSIZE 1024

void displayWidthAndHeight(int inputFile, int outputFile) {
    uint32_t width;
    uint32_t height;
    char buffer[BUFFSIZE];
    int bytesRead;

    if ((bytesRead = read(inputFile, buffer, BUFFSIZE)) != 0) {
        width = *(uint32_t *)(buffer + 18);
        sprintf(buffer, "Width: %d\n", width);
        write(outputFile, buffer, strlen(buffer));

        height = *(uint32_t *)(buffer + 22);
        sprintf(buffer, "Height: %d\n", height);
        write(outputFile, buffer, strlen(buffer));
    }
}

void displayFileInfo(int inputFile, int outputFile) {
    struct stat fileInfo;
    char buffer[BUFFSIZE];

    if (fstat(inputFile, &fileInfo) == -1) {
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

void displayPermission(char *permissionType, mode_t permission, int outputFile) {
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

void displayAllPermissions(int inputFile, int outputFile) {
    struct stat filePermission;

    if (fstat(inputFile, &filePermission) == -1) {
        perror("Error obtaining file information");
        exit(EXIT_FAILURE);
    }

    displayPermission("User Access Rights", (filePermission.st_mode & S_IRWXU), outputFile);
    displayPermission("Group Access Rights", (filePermission.st_mode & S_IRWXG), outputFile);
    displayPermission("Others Access Rights", filePermission.st_mode, outputFile);
}

int main(int argc, char *argv[]) {
    int inputFile;
    int outputFile;
    char buffer[BUFFSIZE];

    if (argc != 2) {
        perror("Invalid number of arguments");
        exit(EXIT_FAILURE);
    }

    if ((inputFile = open(argv[1], O_RDONLY)) < 0) {
        perror("Unable to open input file");
        exit(EXIT_FAILURE);
    }

    if ((outputFile = open("statistics.txt", O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU)) < 0) {
        perror("Unable to create output file");
        exit(EXIT_FAILURE);
    }

    char aux[100];
    strcpy(aux, argv[1]);
    char *extension;
    extension = strrchr(aux, '.');

    if (strcmp(extension, ".bmp") != 0) {
        perror("The file is not of the specified type!\nPlease load another file!");
        exit(EXIT_FAILURE);
    }

    sprintf(buffer, "File name: %s\n", aux);
    write(outputFile, buffer, strlen(buffer));

    displayWidthAndHeight(inputFile, outputFile);

    displayFileInfo(inputFile, outputFile);

    displayAllPermissions(inputFile, outputFile);

    close(inputFile);
    close(outputFile);

    return 0;
}
