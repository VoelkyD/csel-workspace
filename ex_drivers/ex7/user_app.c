#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>

#define DEV "/dev/irq_block"

int main() {
    int fd = open(DEV, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    struct pollfd pfd = {
        .fd = fd,
        .events = POLLIN
    };

    int count = 0;
    printf("Waiting for button press (Ctrl+C to stop)...\n");

    while (1) {
        int ret = poll(&pfd, 1, -1);
        if (ret > 0 && (pfd.revents & POLLIN)) {
            read(fd, NULL, 0); // Just consume event
            printf("Interrupt received: %d\n", ++count);
        }
    }

    close(fd);
    return 0;
}