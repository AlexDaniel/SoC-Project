#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //to get pagesize
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

#include "ZedboardOLED.h"

#define FILTER_REG(PTR, REG) *((unsigned *)((PTR) + (REG) * 4))
#define VOLUME_REG_R(PTR)    *((unsigned *)(PTR   +  0    * 4))
#define VOLUME_REG_L(PTR)    *((unsigned *)(PTR   +  1    * 4))

#define VOLUME_0_ADDRESS 0x43C20000
#define FILTER_0_ADDRESS 0x43C40000
#define VOLUME_1_ADDRESS 0x43C30000
#define FILTER_1_ADDRESS 0x43C50000
#define OLED_ADDRESS     0x43C10000

void gpio_setup(int gpio) {
    char buf[255];
    int fd = open("/sys/class/gpio/export", O_WRONLY);
    sprintf(buf, "%d", gpio);
    write(fd, buf, strlen(buf));
    close(fd);

    sprintf(buf, "/sys/class/gpio/gpio%d/direction", gpio);
    fd = open(buf, O_WRONLY);
    write(fd, "out", 3);
    close(fd);
}

void gpio_drive(int gpio, int value) {
    char buf[255];
    sprintf(buf, "/sys/class/gpio/gpio%d/value", gpio);
    int fd = open(buf, O_WRONLY);
    sprintf(buf, "%d", value);
    write(fd, buf, 1);
    close(fd);
}

int main(int argc, char *argv[]) {
    if (*argv[1] == '?') {
        printf("USAGE: %s line/network Vol_L Vol_R High Mid Low\n", argv[0]);
    } else {
        // read address from user input
        unsigned address = strtoul(argv[1], NULL, 0); //if input is in hex, it convert it to dec.

        char *sound_source  = argv[1];

        unsigned left  = atoi(argv[2]);
        unsigned right = atoi(argv[3]);

        unsigned high  = atoi(argv[4]);
        unsigned band  = atoi(argv[5]);
        unsigned low   = atoi(argv[6]);

        // Redirect stdout/printf into /dev/kmsg file (so it will be printed using printk)
        freopen("/dev/kmsg", "w", stdout);

        int fd = open("/dev/mem", O_RDWR);
        if (fd < 1) { perror(argv[0]); return -1; } // Can't read /dev/mem

        // get architecture specific page size
        unsigned pageSize = sysconf(_SC_PAGESIZE);

        // find the device in the system memory
        unsigned volume_offset;
        unsigned filter_offset;
        unsigned oled_offset;
        int gpio_base;
        int bank_shift;
        if (sound_source[0] == 'n') {
            volume_offset = VOLUME_0_ADDRESS & (~(pageSize-1));
            filter_offset = FILTER_0_ADDRESS & (~(pageSize-1));
            gpio_base = 224 + 5;
            bank_shift = 0;
        } else {
            volume_offset = VOLUME_1_ADDRESS & (~(pageSize-1));
            filter_offset = FILTER_1_ADDRESS & (~(pageSize-1));
            gpio_base = 224 + 0;
            bank_shift = 2;
        }
        oled_offset = OLED_ADDRESS & (~(pageSize-1));

        void *volume_ptr = mmap(NULL, pageSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, volume_offset);
        void *filter_ptr = mmap(NULL, pageSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, filter_offset);
        void *oled_ptr   = mmap(NULL, pageSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, oled_offset);

        // 2KHz - 3KHz
        FILTER_REG(filter_ptr, 0) = 0x00002CB6;
        FILTER_REG(filter_ptr, 1) = 0x0000596C;
        FILTER_REG(filter_ptr, 2) = 0x00002CB6;
        FILTER_REG(filter_ptr, 3) = 0x8097A63A;
        FILTER_REG(filter_ptr, 4) = 0x3F690C9D;
        // 2KHz - 3KHz
        FILTER_REG(filter_ptr, 5) = 0x074D9236;
        FILTER_REG(filter_ptr, 6) = 0x00000000;
        FILTER_REG(filter_ptr, 7) = 0xF8B26DCA;
        FILTER_REG(filter_ptr, 8) = 0x9464B81B;
        FILTER_REG(filter_ptr, 9) = 0x3164DB93;
        // 12KHz
        FILTER_REG(filter_ptr, 10) = 0x12BEC333;
        FILTER_REG(filter_ptr, 11) = 0xDA82799A;
        FILTER_REG(filter_ptr, 12) = 0x12BEC333;
        FILTER_REG(filter_ptr, 13) = 0x00000000;
        FILTER_REG(filter_ptr, 14) = 0x0AFB0CCC;
        //
        FILTER_REG(filter_ptr, 16) = 1;
        FILTER_REG(filter_ptr, 16) = 0;
        //
        FILTER_REG(filter_ptr, 15) = 0;
        //
        FILTER_REG(filter_ptr, 17) = high;
        FILTER_REG(filter_ptr, 18) = band;
        FILTER_REG(filter_ptr, 19) = low;


        // VOLUME 0
        VOLUME_REG_R(volume_ptr) = right;
        VOLUME_REG_L(volume_ptr) = left;

        gpio_setup(gpio_base + 0);
        gpio_setup(gpio_base + 1);
        gpio_setup(gpio_base + 2);
        gpio_drive(gpio_base + 0, low);
        gpio_drive(gpio_base + 1, band);
        gpio_drive(gpio_base + 2, high);

        // oled_clear(oled_ptr);
        char str1[16];
        char str2[16];
        sprintf(str1, "%3d%% (%.6s)", 100 * left  / 256, sound_source);
        sprintf(str2, "%3d%% (%.6s)", 100 * right / 256, sound_source);
        oled_print_message(str1, bank_shift + 0, oled_ptr);
        oled_print_message(str2, bank_shift + 1, oled_ptr);

        printf("Hello everything! :)\n");

        //delete mapping
        munmap(volume_ptr, pageSize);
        munmap(filter_ptr, pageSize);
        munmap(oled_ptr,   pageSize);
    }
    return 0;
}
