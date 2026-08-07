#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>

static uint8_t fw[8192];

int main(void) {
    int fd; size_t sz; struct stat st;
    const char *path = "/usr/share/sg2002-8051-demo/8051-mbox-echo.bin";

    fd = open(path, O_RDONLY);
    if (fd < 0) { perror("open fw"); return 1; }
    fstat(fd, &st); sz = st.st_size;
    read(fd, fw, sz); close(fd);
    printf("loaded %zu bytes\n", sz);

    fd = open("/dev/mem", O_RDWR|O_SYNC);
    if (fd < 0) { perror("open mem"); return 1; }

    volatile uint32_t *regs = mmap(NULL, 0x27000, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x05000000UL);
    volatile uint32_t *sram = mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x05200000UL);
    volatile uint32_t *top  = mmap(NULL, 0x1000, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x03000000UL);
    if (regs == MAP_FAILED || sram == MAP_FAILED || top == MAP_FAILED)
        { perror("mmap"); return 1; }

    regs[(0x05025018UL - 0x05000000UL)/4] = 0x8107fffdU;
    for (size_t i = 0; i < 8192; i += 4) {
        uint32_t w = 0;
        for (size_t b = 0; b < 4 && i+b < sz; b++) w |= (uint32_t)fw[i+b] << (8U*b);
        sram[i/4] = w;
    }
    top[(0x03000248UL - 0x03000000UL)/4] = 1;
    regs[(0x05025020UL - 0x05000000UL)/4] = (0x05200000UL & 0xfffff000UL) | 0x84U;
    regs[(0x05025018UL - 0x05000000UL)/4] = 0x8107ffffU;
    munmap((void*)sram, 8192); munmap((void*)top, 0x1000); munmap((void*)regs, 0x27000);

    sleep(2);

    volatile uint32_t *rtc = mmap(NULL, 0x2000, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x05025000UL);
    printf("RTC_INFO0 = 0x%08x\n", rtc[0x101c/4]);
    munmap((void*)rtc, 0x2000);

    /* Mailbox test via polling */
    volatile uint8_t *mbox = mmap(NULL, 0x1000, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x01900000UL);
    if (mbox == MAP_FAILED) { perror("mmap mbox"); close(fd); return 1; }
    mbox[4] = 0xFF;

    const char *tx = "Hello805";
    volatile uint32_t *ctx = (volatile uint32_t *)(mbox + 0x408);
    ctx[0] = ((uint32_t)(uint8_t)tx[0]) | ((uint32_t)(uint8_t)tx[1]<<8)
           | ((uint32_t)(uint8_t)tx[2]<<16) | ((uint32_t)(uint8_t)tx[3]<<24);
    ctx[1] = ((uint32_t)(uint8_t)tx[4]) | ((uint32_t)(uint8_t)tx[5]<<8)
           | ((uint32_t)(uint8_t)tx[6]<<16) | ((uint32_t)(uint8_t)tx[7]<<24);
    mbox[0x40] = 2; mbox[12] |= 2; mbox[0x60] = 2;

    uint32_t prev = ctx[0];
    printf("Sent '%s', polling...\n", tx);
    for (int i = 0; i < 3000; i++) {
        if (ctx[0] != prev) {
            char rx[9] = {0};
            uint32_t r0 = ctx[0], r1 = ctx[1];
            rx[0]=r0&0xff; rx[1]=(r0>>8)&0xff; rx[2]=(r0>>16)&0xff; rx[3]=(r0>>24)&0xff;
            rx[4]=r1&0xff; rx[5]=(r1>>8)&0xff; rx[6]=(r1>>16)&0xff; rx[7]=(r1>>24)&0xff;
            printf("Echo '%s' — %s\n", rx,
                   (rx[0]=='E' && rx[1]=='C' && rx[2]=='H') ? "PASS" : "PARTIAL");
            munmap((void*)mbox, 0x1000); close(fd);
            return (rx[0]=='E' && rx[1]=='C' && rx[2]=='H') ? 0 : 1;
        }
        usleep(1000);
    }
    printf("TIMEOUT\n");
    munmap((void*)mbox, 0x1000); close(fd);
    return 1;
}
