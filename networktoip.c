#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // to get pagesize
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>
#include <stdbool.h>

#include "udpclient.h"

#define IP   "10.255.255.255"
#define PORT 7891

#define AUDIO_ADDRESS     0x43C60000

void *ptr;
unsigned registerOffset;

void audio_read() {
    udp_client_setup(IP, PORT);
    while (true) {
        if (udp_client_recv(ptr + registerOffset, 4) == -1)
            exit(0);
    }
}

int main(int argc, char *argv[]){
    unsigned address = AUDIO_ADDRESS;

    // open /dev/mem file
    int fd = open("/dev/mem", O_RDWR);
    if (fd < 1) { perror(argv[0]); return -1; } // Can't read /dev/mem

    // get architecture specific page size
    unsigned pageSize = sysconf(_SC_PAGESIZE);

    // find the device in the system memory
    unsigned offset = address & (~(pageSize-1));
    registerOffset = address - offset;

    // a pointer declaration to access virtual address space where physical memory will be mapped
    ptr = mmap(NULL, pageSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);

    audio_read();

    //delete mapping
    munmap(ptr, pageSize); // TODO

    return 0;
}
