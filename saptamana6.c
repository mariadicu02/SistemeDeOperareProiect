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

void printWidthAndHeight(int fisierIntrare, int fisierIesire) {
    uint32_t width;
    uint32_t height;
    char buffer[BUFFSIZE];
    int rd;

    if ((rd = read(fisierIntrare, buffer, BUFFSIZE)) != 0) {
        width = *(uint32_t *)(buffer + 18);
        sprintf(buffer, "Latimea este: %d\n", width);
        write(fisierIesire, buffer, strlen(buffer));

        height = *(uint32_t *)(buffer + 22);
        sprintf(buffer, "Inaltimea este: %d\n", height);
        write(fisierIesire, buffer, strlen(buffer));
    }
}

void printFileInfo(int fisierIntrare, int fisierIesire) {
    struct stat fileInfo;
    char buffer[BUFFSIZE];

    if (fstat(fisierIntrare, &fileInfo) == -1) {
        perror("Eroare la obtinerea informatiilor despre fisier");
        exit(EXIT_FAILURE);
    }

    sprintf(buffer, "Dimensiune: %ld\n", fileInfo.st_size);
    write(fisierIesire, buffer, strlen(buffer));

    sprintf(buffer, "Identificatorul utilizatorului: %d\n", fileInfo.st_uid);
    write(fisierIesire, buffer, strlen(buffer));

    sprintf(buffer, "Timpul ultimei modificari: %s\n", ctime(&fileInfo.st_mtime));
    write(fisierIesire, buffer, strlen(buffer));

    sprintf(buffer, "Numarul de legaturi: %ld\n", fileInfo.st_nlink);
    write(fisierIesire, buffer, strlen(buffer));
}

void printPermission(char *permissionType, mode_t permission, int fisierIesire) {
    char buffer[BUFFSIZE];

    sprintf(buffer, "%s: ", permissionType);
    write(fisierIesire, buffer, strlen(buffer));

    if (permission & S_IRUSR) {
        sprintf(buffer, "R");
        write(fisierIesire, buffer, strlen(buffer));
    } else {
        sprintf(buffer, "-");
        write(fisierIesire, buffer, strlen(buffer));
    }

    if (permission & S_IWUSR) {
        sprintf(buffer, "W");
        write(fisierIesire, buffer, strlen(buffer));
    } else {
        sprintf(buffer, "-");
        write(fisierIesire, buffer, strlen(buffer));
    }

    if (permission & S_IXUSR) {
        sprintf(buffer, "X\n");
        write(fisierIesire, buffer, strlen(buffer));
    } else {
        sprintf(buffer, "-\n");
        write(fisierIesire, buffer, strlen(buffer));
    }
}

void printAllPermissions(int fisierIntrare, int fisierIesire) {
    struct stat filePermission;

    if (fstat(fisierIntrare, &filePermission) == -1) {
        perror("Eroare la obtinerea informatiilor despre fisier");
        exit(EXIT_FAILURE);
    }

    printPermission("Drepturi de acces user", (filePermission.st_mode & S_IRWXU), fisierIesire);
    printPermission("Drepturi de acces grup", (filePermission.st_mode & S_IRWXG), fisierIesire);
    printPermission("Drepturi de acces pentru altii", filePermission.st_mode, fisierIesire);
}

int main(int argc, char *argv[]) {
    int fisierIntrare;
    int fisierIesire;
    char buffer[BUFFSIZE];

    if (argc != 2) {
        perror("Numar de argumente invalid");
        exit(EXIT_FAILURE);
    }

    if ((fisierIntrare = open(argv[1], O_RDONLY)) < 0) {
        perror("Nu se poate deschide fisierul de intrare");
        exit(EXIT_FAILURE);
    }

    if ((fisierIesire = open("statistica.txt", O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU)) < 0) {
        perror("Nu se poate crea fisierul de iesire");
        exit(EXIT_FAILURE);
    }

    char aux[100];
    strcpy(aux, argv[1]);
    char *trunc;
    trunc = strrchr(aux, '.');

    if (strcmp(trunc, ".bmp") != 0) {
        perror("Fisierul nostru nu este de tipul indicat!\nIncarcati alt fisier!");
        exit(EXIT_FAILURE);
    }

    sprintf(buffer, "File name: %s\n", aux);
    write(fisierIesire, buffer, strlen(buffer));

    printWidthAndHeight(fisierIntrare, fisierIesire);

    printFileInfo(fisierIntrare, fisierIesire);

    printAllPermissions(fisierIntrare, fisierIesire);

    close(fisierIntrare);
    close(fisierIesire);

    return 0;
}
