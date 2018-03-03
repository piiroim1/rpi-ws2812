
#define BCM2708_PERI_BASE        0x3F000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>


#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))
#define GPIO_SET *(gpio+7)
#define GPIO_CLR *(gpio+10)
#define GET_GPIO(g) (*(gpio+13)&(1<<g))
#define GPIO_PULL *(gpio+37)
#define GPIO_PULLCLK0 *(gpio+38)

#define T0H_NS 350
#define T1H_NS 900
#define T0L_NS 900
#define T0H_NS 350
#define RES_NS 50000
#define T1_NS (T0H_NS)
#define T2_NS (T1H_NS)
#define T3_NS ((T0H_NS) + (T0L_NS))
#define D1_NS (T1_NS)
#define D2_NS ((T2_NS) - (T1_NS))
#define D3_NS ((T3_NS) - (T2_NS))

int  mem_fd;
void *gpio_map;
volatile unsigned *gpio;

static void setup_io() {
    if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
       printf("can't open /dev/mem \n");
       exit(-1);
    }
    gpio_map = mmap(
       NULL,
       BLOCK_SIZE,
       PROT_READ|PROT_WRITE,
       MAP_SHARED,
       mem_fd,
       GPIO_BASE
    );
    close(mem_fd);
    if (gpio_map == MAP_FAILED) {
       printf("mmap error %d\n", (int)gpio_map);
       exit(-1);
    }
    gpio = (volatile unsigned *)gpio_map;
}

#pragma GCC push_options
#pragma GCC optimize ("O0")
static inline void delay_cycles(int n) {
	for(int i = 0; i < n; ++i) {
        asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
        asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
	}
}
#pragma GCC pop_options

static inline void delay_ns(int t_ns, float one_cycle_ps) {
    int n = t_ns * 1000 / one_cycle_ps - 3;
    delay_cycles(n);
}
static inline void send_bits(int bits, int active, int one_cycle_ps) {
    GPIO_SET = active;
    delay_ns(D1_NS, one_cycle_ps);
    GPIO_CLR = active & ~bits;
    delay_ns(D2_NS, one_cycle_ps);
    GPIO_CLR = active & bits;
    delay_ns(D3_NS, one_cycle_ps);
}
static inline void send_res(int bits, int one_cycle_ps) {
    GPIO_CLR = bits;
    delay_ns(RES_NS, one_cycle_ps);
}

static inline int get_one_cycle_ps() {
    int n = 100000;
    //printf("timing busy cycles...\n");
    struct timespec t0;
    struct timespec t1;
    float one_cycle_ps = 0.0f;
    while(one_cycle_ps <= 0.0f) {
        clock_gettime(CLOCK_MONOTONIC, &t0);
        delay_cycles(n);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        int dt_ns = t1.tv_nsec - t0.tv_nsec;
        one_cycle_ps = dt_ns * 1000 / n;
    }
    return one_cycle_ps;
}
 
int main(int argc, char **argv) {
    int g,rep;
    setup_io();
    for (g=0; g<=14; g++) {
        INP_GPIO(g);
        OUT_GPIO(g);
    }
    struct timespec slp = { .tv_sec = 0, .tv_nsec = 50000000 };
    struct timespec rem;
    int p = 0;
    while(1) {
        p++;
        int r = abs(p % 512 - 256);
        int g = abs(p / 2 % 512 - 256);
        int b = abs(p / 3 % 512 - 256);
        long int one_cycle_ps = get_one_cycle_ps();
	    nanosleep(&slp, &rem);
        send_res(1<<2, one_cycle_ps);
        for(int i = 0; i < 300; ++i) {
            send_bits((g & 128) >> 7 << 2, 1<<2, one_cycle_ps);
            send_bits((g & 64 ) >> 6 << 2, 1<<2, one_cycle_ps);
            send_bits((g & 32 ) >> 5 << 2, 1<<2, one_cycle_ps);
            send_bits((g & 16 ) >> 4 << 2, 1<<2, one_cycle_ps);
            send_bits((g & 8  ) >> 3 << 2, 1<<2, one_cycle_ps);
            send_bits((g & 4  ) >> 2 << 2, 1<<2, one_cycle_ps);
            send_bits((g & 2  ) >> 1 << 2, 1<<2, one_cycle_ps);
            send_bits((g & 1  ) >> 0 << 2, 1<<2, one_cycle_ps);
            send_bits((b & 128) >> 7 << 2, 1<<2, one_cycle_ps);
            send_bits((b & 64 ) >> 6 << 2, 1<<2, one_cycle_ps);
            send_bits((b & 32 ) >> 5 << 2, 1<<2, one_cycle_ps);
            send_bits((b & 16 ) >> 4 << 2, 1<<2, one_cycle_ps);
            send_bits((b & 8  ) >> 3 << 2, 1<<2, one_cycle_ps);
            send_bits((b & 4  ) >> 2 << 2, 1<<2, one_cycle_ps);
            send_bits((b & 2  ) >> 1 << 2, 1<<2, one_cycle_ps);
            send_bits((b & 1  ) >> 0 << 2, 1<<2, one_cycle_ps);
            send_bits((r & 128) >> 7 << 2, 1<<2, one_cycle_ps);
            send_bits((r & 64 ) >> 6 << 2, 1<<2, one_cycle_ps);
            send_bits((r & 32 ) >> 5 << 2, 1<<2, one_cycle_ps);
            send_bits((r & 16 ) >> 4 << 2, 1<<2, one_cycle_ps);
            send_bits((r & 8  ) >> 3 << 2, 1<<2, one_cycle_ps);
            send_bits((r & 4  ) >> 2 << 2, 1<<2, one_cycle_ps);
            send_bits((r & 2  ) >> 1 << 2, 1<<2, one_cycle_ps);
            send_bits((r & 1  ) >> 0 << 2, 1<<2, one_cycle_ps);
        }
    }
    return 0;
}

