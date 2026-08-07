/* SPDX-License-Identifier: MIT */
/*
 * 8051-mbox-echo.c — 8051 firmware for SG2002 mailbox echo demo
 *
 * Polls mailbox channel 1 via SET_INT(3). Read messages, echoes
 * them back with "ECH" prefix into the context area. Linux side
 * polls the context for changes.
 *
 * Build: make -f Makefile.8051
 */

typedef unsigned char uint8_t;
typedef unsigned int uint16_t;
typedef unsigned long uint32_t;

/* Robot SFR registers (verified against working mars_mcu_fw) */
__sfr __at (0xE4) r51_rd0; __sfr __at (0xE5) r51_rd1;
__sfr __at (0xE6) r51_rd2; __sfr __at (0xE7) r51_rd3;
__sfr __at (0xF2) r51_fire;
__sfr __at (0xF3) r51_write_enable;
__sfr __at (0xF4) r51_wd0; __sfr __at (0xF5) r51_wd1;
__sfr __at (0xF6) r51_wd2; __sfr __at (0xF7) r51_wd3;
__sfr __at (0xF8) r51_addr0; __sfr __at (0xF9) r51_addr1;
__sfr __at (0xFA) r51_addr2; __sfr __at (0xFB) r51_addr3;
__sbit __at (0xAF) EA;

#define RTC_INFO0   0x0502601CUL
#define TOP_MISC    0x03000248UL
#define MB          0x01900000UL
#define MB_CTX(ch)  (MB + 0x400 + (ch) * 8)
#define SET_CLR(cpu) (MB + 0x10 + (cpu) * 16)
#define EN(cpu)     (MB + (cpu) * 4)
#define SET         (MB + 0x60)
#define SET_INT(cpu) (MB + 0x18 + (cpu) * 16)
#define CPU1 1
#define CPU3 3
#define CH   1

static void w(uint32_t addr, uint32_t val) {
    uint8_t ie = EA; EA = 0;
    r51_addr0 = (uint8_t)addr; r51_addr1 = (uint8_t)(addr >> 8);
    r51_addr2 = (uint8_t)(addr >> 16); r51_addr3 = (uint8_t)(addr >> 24);
    r51_wd0 = (uint8_t)val; r51_wd1 = (uint8_t)(val >> 8);
    r51_wd2 = (uint8_t)(val >> 16); r51_wd3 = (uint8_t)(val >> 24);
    r51_write_enable = 5; r51_fire = 1;
    while (r51_fire == 1) {}
    EA = ie;
}

static uint32_t r(uint32_t addr) {
    uint8_t ie = EA; uint32_t val; EA = 0;
    r51_addr0 = (uint8_t)addr; r51_addr1 = (uint8_t)(addr >> 8);
    r51_addr2 = (uint8_t)(addr >> 16); r51_addr3 = (uint8_t)(addr >> 24);
    r51_write_enable = 4; r51_fire = 1;
    while (r51_fire == 1) {}
    val = r51_rd0 | ((uint32_t)r51_rd1 << 8)
        | ((uint32_t)r51_rd2 << 16) | ((uint32_t)r51_rd3 << 24);
    EA = ie;
    return val;
}

static void delay(void) {
    uint16_t i;
    for (i = 0; i < 5000; i++) { __asm nop __endasm; }
}

void main(void)
{
    uint8_t rx[8], tx[8], i, valid = 1 << CH;

    w(RTC_INFO0, 0x00008051);
    w(TOP_MISC, 1);
    w(EN(CPU3), 0xFF);

    for (;;) {
        if (r(SET_INT(CPU3)) & valid) {
            uint32_t lo = r(MB_CTX(CH)), hi = r(MB_CTX(CH) + 4);
            rx[0] = lo; rx[1] = lo >> 8; rx[2] = lo >> 16; rx[3] = lo >> 24;
            rx[4] = hi; rx[5] = hi >> 8; rx[6] = hi >> 16; rx[7] = hi >> 24;
            w(SET_CLR(CPU3), valid);

            tx[0] = 'E'; tx[1] = 'C'; tx[2] = 'H';
            for (i = 0; i < 5; i++) tx[3 + i] = rx[i];

            /* Write echo to context area (visible to Linux via /dev/mem polling) */
            w(MB_CTX(CH),
              ((uint32_t)tx[0]) | ((uint32_t)tx[1] << 8)
              | ((uint32_t)tx[2] << 16) | ((uint32_t)tx[3] << 24));
            w(MB_CTX(CH) + 4,
              ((uint32_t)tx[4]) | ((uint32_t)tx[5] << 8)
              | ((uint32_t)tx[6] << 16) | ((uint32_t)tx[7] << 24));
        }
    }
}
