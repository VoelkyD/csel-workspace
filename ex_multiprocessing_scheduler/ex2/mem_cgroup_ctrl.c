#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BLOCK_SIZE_MB 1
#define BLOCK_COUNT 50
#define BLOCK_SIZE_BYTES (BLOCK_SIZE_MB * 1024 * 1024)

int main() {
    printf("Début allocation mémoire\n");

    void *blocks[BLOCK_COUNT];

    for (int i = 0; i < BLOCK_COUNT; ++i) {
        blocks[i] = malloc(BLOCK_SIZE_BYTES);

        if (blocks[i] == NULL) {
            fprintf(stderr, "Erreur : allocation échouée au bloc %d (soit %d MiB)\n", i, i * BLOCK_SIZE_MB);
            break;
        }

        memset(blocks[i], 0, BLOCK_SIZE_BYTES); // Remplissage avec des zéros
        printf("Bloc %d alloué (%d MiB)\n", i + 1, (i + 1) * BLOCK_SIZE_MB);
        sleep(1);
    }

    printf("Fin de l’allocation.\n");

    sleep(10);
    return 0;
}
