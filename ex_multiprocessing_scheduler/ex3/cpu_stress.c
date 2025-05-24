#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

void consume_cpu() {
    volatile unsigned long long i = 0;
    while (1) {
        i++;
        if (i == 0) {
            printf("overflow\n");
        }
    }
}

int main() {
    printf("PID %d démarre une charge CPU\n", getpid());

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        printf("Processus enfant PID %d lancé.\n", getpid());
        consume_cpu();
    } else {
        printf("Processus parent PID %d lancé.\n", getpid());
        consume_cpu();
    }

    return 0;
}
