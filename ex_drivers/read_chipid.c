#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>

#define CHIPID_BASE 0x01C14200
#define PAGE_SIZE   4096
#define PAGE_MASK   (~(PAGE_SIZE - 1))

int main() {
    int fd;
    void *mapped_base;
    volatile uint32_t *chipid;
    off_t target = CHIPID_BASE;

    // Ouvre /dev/mem en lecture seule
    fd = open("/dev/mem", O_RDONLY | O_SYNC);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    // Aligne l'adresse sur la page
    off_t page_base = target & PAGE_MASK;
    off_t page_offset = target - page_base;

    // Mappe 1 page mémoire à partir de l'adresse physique
    mapped_base = mmap(NULL, PAGE_SIZE, PROT_READ, MAP_SHARED, fd, page_base);
    if (mapped_base == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    // Accès au registre via pointeur décalé
    chipid = (volatile uint32_t *)((char *)mapped_base + page_offset);

    printf("Chip ID registers:\n");
    printf("  [%p] = %08X\n", chipid + 0, chipid[0]);
    printf("  [%p] = %08X\n", chipid + 1, chipid[1]);
    printf("  [%p] = %08X\n", chipid + 2, chipid[2]);
    printf("  [%p] = %08X\n", chipid + 3, chipid[3]);

    // Nettoyage
    munmap(mapped_base, PAGE_SIZE);
    close(fd);

    return 0;
}
