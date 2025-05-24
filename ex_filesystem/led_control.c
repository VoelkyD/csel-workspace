/**
 * Copyright 2025 University of Applied Sciences Western Switzerland / Fribourg
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Project: HEIA-FR / HES-SO MSE - MA-CSEL1 Laboratory
 *
 * Abstract: System programming -  file system
 *
 * Purpose: NanoPi status led control system
 *
 * Autĥor:  Yoann Künti
 * Date:    24.05.2025
 */
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <stdio.h>

/*
 * status led - gpioa.10 --> gpio10
 * power led  - gpiol.10 --> gpio362
 */
#define GPIO_EXPORT   "/sys/class/gpio/export"
#define GPIO_UNEXPORT "/sys/class/gpio/unexport"

#define GPIO_LED      "/sys/class/gpio/gpio10"
#define LED           "10"

#define GPIO_K1       "/sys/class/gpio/gpio0"
#define K1            "0"
#define GPIO_K2       "/sys/class/gpio/gpio2"
#define K2            "2"
#define GPIO_K3       "/sys/class/gpio/gpio3"
#define K3            "3"

#define MAX_EVENTS 4    // 3 buttons + 1 timer_fd

static int open_led()
{
    // unexport pin out of sysfs (reinitialization)
    int f = open(GPIO_UNEXPORT, O_WRONLY);
    write(f, LED, strlen(LED));
    close(f);

    // export pin to sysfs
    f = open(GPIO_EXPORT, O_WRONLY);
    write(f, LED, strlen(LED));
    close(f);

    // config pin
    f = open(GPIO_LED "/direction", O_WRONLY);
    write(f, "out", 3);
    close(f);

    // open gpio value attribute
    f = open(GPIO_LED "/value", O_RDWR);
    return f;
}

int open_buttons(const char *pin, const char *path) {
    int f = open(GPIO_UNEXPORT, O_WRONLY);
    write(f, pin, strlen(pin));
    close(f);

    f = open(GPIO_EXPORT, O_WRONLY);
    write(f, pin, strlen(pin));
    close(f);

    char dir[64], edge[64], val[64];
    snprintf(dir, sizeof(dir), "%s/direction", path);
    f = open(dir, O_WRONLY);
    write(f, "in", 2);
    close(f);

    snprintf(edge, sizeof(edge), "%s/edge", path);
    f = open(edge, O_WRONLY);
    write(f, "rising", 6);
    close(f);

    snprintf(val, sizeof(val), "%s/value", path);
    f = open(val, O_RDONLY | O_NONBLOCK);
    char buf;
    read(f, &buf, 1);
    return f;
}

int read_buttons(int fd) {
    lseek(fd, 0, SEEK_SET);
    char val;
    read(fd, &val, 1);
    return (val == '1');
}

void register_event_fd(int epfd, int fd, int is_timer) {
    struct epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = is_timer ? EPOLLIN : EPOLLPRI;

    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
}

int main() {
    int led_fd = open_led();
    pwrite(led_fd, "1", 1, 0);

    int k1_fd = open_buttons(K1, GPIO_K1);
    int k2_fd = open_buttons(K2, GPIO_K2);
    int k3_fd = open_buttons(K3, GPIO_K3);

    int epfd = epoll_create1(0);
    register_event_fd(epfd, k1_fd, 0);
    register_event_fd(epfd, k2_fd, 0);
    register_event_fd(epfd, k3_fd, 0);

    int timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);
    register_event_fd(epfd, timer_fd, 1);

    // Timer init
    int freq = 2;   // 2 Hz
    int state = 0;
    struct itimerspec its;
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = 1e9 / (freq * 2);
    its.it_interval = its.it_value;
    timerfd_settime(timer_fd, 0, &its, NULL);

    struct epoll_event events[MAX_EVENTS];

    while (1) {
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;
            if (fd == timer_fd) {
                uint64_t exp;
                read(timer_fd, &exp, sizeof(exp));
                state = !state;
                pwrite(led_fd, state ? "1" : "0", 1, 0);
            } else if (fd == k1_fd && read_buttons(k1_fd)) {
                if (freq < 10) {    // 10 Hz max
                    freq++;  
                }
            } else if (fd == k2_fd && read_buttons(k2_fd)) {
                freq = 2;
            } else if (fd == k3_fd && read_buttons(k3_fd)) {
                if (freq > 1) {     // 1 Hz min
                    freq--;
                }
            }

            its.it_value.tv_nsec = 1e9 / (freq * 2);
            its.it_interval = its.it_value;
            timerfd_settime(timer_fd, 0, &its, NULL);
        }
    }

    return 0;
}