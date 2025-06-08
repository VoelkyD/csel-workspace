#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define FIFO_PATH "/tmp/fan_ipc"

void usage() {
    printf("Usage:\n");
    printf("  ./fanctl mode [auto|manual]\n");
    printf("  ./fanctl freq [1-20]\n");
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc != 3) usage();

    int fifo_fd = open(FIFO_PATH, O_WRONLY);
    if (fifo_fd < 0) {
        perror("open fifo");
        return 1;
    }

    char cmd[64];
    snprintf(cmd, sizeof(cmd), "%s %s", argv[1], argv[2]);
    write(fifo_fd, cmd, strlen(cmd));
    close(fifo_fd);

    return 0;
}
