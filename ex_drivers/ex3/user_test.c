#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define DEVICE_PATH_FORMAT "/dev/mymodule%d"
#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    if (argc < 3) {
        printf("Usage: %s <device_number> <message>\n", argv[0]);
        return 1;
    }

    int dev_num = atoi(argv[1]);
    const char *message = argv[2];
    char device_path[64];
    snprintf(device_path, sizeof(device_path), DEVICE_PATH_FORMAT, dev_num);

    int fd = open(device_path, O_RDWR);
    if (fd < 0) {
        perror("Erreur open");
        return 1;
    }

    // Réinitialiser l’offset AVANT écriture
    if (lseek(fd, 0, SEEK_SET) < 0) {
        perror("Erreur lseek avant write");
        close(fd);
        return 1;
    }

    if (write(fd, message, strlen(message)) < 0) {
        perror("Erreur write");
        close(fd);
        return 1;
    }

    // Réinitialiser l’offset AVANT lecture
    if (lseek(fd, 0, SEEK_SET) < 0) {
        perror("Erreur lseek avant read");
        close(fd);
        return 1;
    }

    char buffer[BUFFER_SIZE] = {0};
    ssize_t n = read(fd, buffer, sizeof(buffer) - 1);
    if (n < 0) {
        perror("Erreur read");
        close(fd);
        return 1;
    }

    buffer[n] = '\0';
    printf("Texte lu depuis %s : \"%s\"\n", device_path, buffer);

    close(fd);
    return 0;
}
