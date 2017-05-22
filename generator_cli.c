#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // to get pagesize
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

#define SAMPLE_RATE 48000
#define MAX_AMPLITUDE 0x7FFFFF

#define FREQUENCY_REG(PTR)  *((unsigned *)(PTR + 0 * 4))
#define AMPLITUDE_REG(PTR)  *((unsigned *)(PTR + 1 * 4))
#define PERIOD_REG(PTR)     *((unsigned *)(PTR + 2 * 4))
#define DUTY_CYCLE_REG(PTR) *((unsigned *)(PTR + 3 * 4))

#define GENERATOR_ADDRESS 0x43C00000

int main(int argc, char *argv[]) {
    if (*argv[1] == '?') {
        printf("USAGE: %s frequency amplitude period duty_cycle\n", argv[0]);
    } else {
        unsigned frequency  = atoi(argv[1]);
        unsigned amplitude  = atoi(argv[2]);
        unsigned period     = atoi(argv[3]);
        unsigned duty_cycle = atoi(argv[4]);

        // Redirect stdout/printf into /dev/kmsg file (so it will be printed using printk)
        freopen("/dev/kmsg", "w", stdout);

        int fd = open("/dev/mem", O_RDWR);
        if (fd < 1) { perror(argv[0]); return -1; } // Can't read /dev/mem

        // get architecture specific page size
        unsigned pageSize = sysconf(_SC_PAGESIZE);

        // find the device in the system memory
        unsigned generator_offset;
        generator_offset = GENERATOR_ADDRESS & (~(pageSize-1));

        void *generator_ptr = mmap(NULL, pageSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, generator_offset);

        FREQUENCY_REG(generator_ptr)  = SAMPLE_RATE / frequency / 2;
        AMPLITUDE_REG(generator_ptr)  = amplitude * MAX_AMPLITUDE / 100;
        PERIOD_REG(generator_ptr)     = period     * SAMPLE_RATE / 1000;
        DUTY_CYCLE_REG(generator_ptr) = duty_cycle * SAMPLE_RATE / 1000;

        printf("Hello generator! :)\n");

        // delete mapping
        munmap(generator_ptr, pageSize);
    }
    return 0;
}
