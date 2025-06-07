#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/epoll.h>
#include <errno.h>
#include <sys/stat.h>
#include "oled/ssd1306.h"

#define GPIO_EXPORT   "/sys/class/gpio/export"
#define GPIO_UNEXPORT "/sys/class/gpio/unexport"

#define GPIO_LED_PWR  "362"
#define GPIO_LED_PWR_PATH "/sys/class/gpio/gpio362"

#define GPIO_K1 "0"  // S1 → +1 Hz
#define GPIO_K2 "2"  // S2 → -1 Hz
#define GPIO_K3 "3"  // S3 → mode toggle

#define SYSFS_MODE "/sys/class/fan_control/fan/mode"
#define SYSFS_FREQ "/sys/class/fan_control/fan/frequency"

#define MAX_EVENTS 4

#define FIFO_PATH "/tmp/fan_ipc"

int write_file(const char *path, const char *val) {
    int f = open(path, O_WRONLY);
    if (f < 0) return -1;
    write(f, val, strlen(val));
    close(f);
    return 0;
}

void export_gpio(const char *n) {
    write_file(GPIO_UNEXPORT, n); usleep(100000);
    write_file(GPIO_EXPORT, n); usleep(100000);
}

int open_button_gpio(const char *gpio) {
    char path[128];
    export_gpio(gpio);

    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%s/direction", gpio);
    write_file(path, "in");
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%s/edge", gpio);
    write_file(path, "rising");

    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%s/value", gpio);
    int fd = open(path, O_RDONLY | O_NONBLOCK);
    char dummy;
    read(fd, &dummy, 1); // clear old value
    return fd;
}

void flash_led() {
    char path[128];
    export_gpio(GPIO_LED_PWR);

    snprintf(path, sizeof(path), GPIO_LED_PWR_PATH "/direction");
    write_file(path, "out");

    snprintf(path, sizeof(path), GPIO_LED_PWR_PATH "/value");
    write_file(path, "1");
    usleep(150000);  // 150 ms
    write_file(path, "0");
}

int read_int_from_file(const char *path) {
    char buf[16];
    int f = open(path, O_RDONLY);
    if (f < 0) return -1;
    int n = read(f, buf, sizeof(buf)-1);
    close(f);
    buf[n] = 0;
    return atoi(buf);
}

void set_freq(int freq) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", freq);
    write_file(SYSFS_FREQ, buf);
}

void toggle_mode() {
    char buf[16];
    int f = open(SYSFS_MODE, O_RDONLY);
    int n = read(f, buf, sizeof(buf)-1);
    close(f);
    buf[n] = 0;

    if (strncmp(buf, "manual", 6) == 0)
        write_file(SYSFS_MODE, "auto");
    else
        write_file(SYSFS_MODE, "manual");
}

void update_oled() {
    char mode_buf[16] = {0};
    int f = open(SYSFS_MODE, O_RDONLY);
    int mlen = read(f, mode_buf, sizeof(mode_buf)-1);
    close(f);
    mode_buf[mlen] = '\0';
    mode_buf[strcspn(mode_buf, "\n")] = '\0'; // remove newline

    int freq = 0;

    int temp_mC = read_int_from_file("/sys/class/thermal/thermal_zone0/temp");
    int temp = temp_mC > 0 ? temp_mC / 1000 : 0;

    // Recalcul dynamique si en auto
    if (strncmp(mode_buf, "auto", 4) == 0) {
        if (temp < 35)
            freq = 2;
        else if (temp < 40)
            freq = 5;
        else if (temp < 45)
            freq = 10;
        else
            freq = 20;
    } else {
        freq = read_int_from_file(SYSFS_FREQ);  // En mode manuel
    }

    char line[32];

    ssd1306_clear_display();

    ssd1306_set_position(0, 0);
    snprintf(line, sizeof(line), "Mode: %s", mode_buf);
    ssd1306_puts(line);

    ssd1306_set_position(0, 2);
    snprintf(line, sizeof(line), "Temp: %dC", temp);
    ssd1306_puts(line);

    ssd1306_set_position(0, 4);
    snprintf(line, sizeof(line), "Freq: %dHz", freq);
    ssd1306_puts(line);
}

int main() {

    ssd1306_init();
    update_oled();

    int k1_fd = open_button_gpio(GPIO_K1);
    int k2_fd = open_button_gpio(GPIO_K2);
    int k3_fd = open_button_gpio(GPIO_K3);

    // IPC: create and open FIFO
    mkfifo(FIFO_PATH, 0666);
    int fifo_fd = open(FIFO_PATH, O_RDONLY | O_NONBLOCK);

    int epfd = epoll_create1(0);
    struct epoll_event ev = {0};
    ev.events = EPOLLPRI;

    ev.data.fd = k1_fd; epoll_ctl(epfd, EPOLL_CTL_ADD, k1_fd, &ev);
    ev.data.fd = k2_fd; epoll_ctl(epfd, EPOLL_CTL_ADD, k2_fd, &ev);
    ev.data.fd = k3_fd; epoll_ctl(epfd, EPOLL_CTL_ADD, k3_fd, &ev);

    // IPC: register FIFO in epoll
    ev.events = EPOLLIN;
    ev.data.fd = fifo_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, fifo_fd, &ev);

    struct epoll_event events[MAX_EVENTS];

    char line[64];

    while (1) {
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;

            if (fd == fifo_fd) {
                int len = read(fd, line, sizeof(line) - 1);
                if (len > 0) {
                    line[len] = '\0';

                    if (strncmp(line, "mode manual", 11) == 0)
                        write_file(SYSFS_MODE, "manual");

                    else if (strncmp(line, "mode auto", 9) == 0)
                        write_file(SYSFS_MODE, "auto");

                    else if (strncmp(line, "freq ", 5) == 0) {
                        int val = atoi(line + 5);
                        if (val >= 1 && val <= 20)
                            set_freq(val);
                    }
                    update_oled();
                }
                continue;
            }

            // Gestions boutons physiques
            char val;
            lseek(fd, 0, SEEK_SET);
            read(fd, &val, 1);
            if (val != '1') continue;

            if (fd == k1_fd || fd == k2_fd) {
                char mode_buf[16] = {0};
                int f = open(SYSFS_MODE, O_RDONLY);
                int mlen = read(f, mode_buf, sizeof(mode_buf)-1);
                close(f);
                mode_buf[mlen] = '\0';

                if (strncmp(mode_buf, "manual", 6) == 0) {
                    int freq = read_int_from_file(SYSFS_FREQ);

                    if (fd == k1_fd && freq < 20)
                        set_freq(freq + 1);
                    if (fd == k2_fd && freq > 1)
                        set_freq(freq - 1);

                    flash_led();  // que pour S1/S2 en mode manuel
                    update_oled();
                }
            }

            if (fd == k3_fd) {
                toggle_mode();  // ne signale pas par LED
                update_oled();
            }
        }
    }

    return 0;
}