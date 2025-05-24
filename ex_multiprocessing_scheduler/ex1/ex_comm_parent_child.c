#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sched.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

int signals_to_ignore[] = {SIGHUP, SIGINT, SIGQUIT, SIGABRT, SIGTERM};

void catch_signal(int signo) {
    printf("[Signal capturé] Ignoré : %d\n", signo);
}

void set_cpu_affinity(int core_id) {
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(core_id, &set);
    if (sched_setaffinity(0, sizeof(set), &set) == -1) {
        perror("sched_setaffinity");
        exit(EXIT_FAILURE);
    }
}

void setup_signals() {
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = catch_signal;

    for (size_t i = 0; i < sizeof(signals_to_ignore) / sizeof(int); ++i) {
        if (sigaction(signals_to_ignore[i], &act, NULL) == -1) {
            perror("sigaction");
            exit(EXIT_FAILURE);
        }
    }
}

int main() {
    int sv[2];

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1) {
        perror("socketpair");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Processus enfant
        close(sv[0]);
        setup_signals();
        set_cpu_affinity(1); // core 1

        const char *message = "Hello parent";
        for(int i = 0; i < 30; i++)
        {
            write(sv[1], message, strlen(message) + 1);
            sleep(1);
        }

        const char *exit_msg = "exit";
        write(sv[1], exit_msg, strlen(exit_msg) + 1);

        close(sv[1]);
        exit(EXIT_SUCCESS);
    } else {
        // Processus parent
        close(sv[1]);
        setup_signals();
        set_cpu_affinity(0); // core 0

        char buffer[256];
        while (1) {
            ssize_t n = read(sv[0], buffer, sizeof(buffer));
            if (n <= 0)
            {
                if (errno == EINTR) continue;  // Signal reçu, mais on continue
                perror("read");
                break;
            }

            buffer[n] = '\0';
            printf("Parent reçoit : %s\n", buffer);

            if (strcmp(buffer, "exit") == 0) break;
        }

        close(sv[0]);
        wait(NULL);
        printf("Fin du programme parent.\n");
    }

    return 0;
}