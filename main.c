
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
#define RES_NS 55000
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

static inline void delay_ns(int t_ns, float one_cycle_ns) { int n = (int)(t_ns / (float)one_cycle_ns) - 3; delay_cycles(n); }
static inline void send_bits(int bits, int active, float one_cycle_ns) {
    GPIO_SET = active;
    delay_ns(D1_NS, one_cycle_ns);
    GPIO_CLR = active & ~bits;
    delay_ns(D2_NS, one_cycle_ns);
    GPIO_CLR = active & bits;
    delay_ns(D3_NS, one_cycle_ns);
}
static inline void send_res(int bits, float one_cycle_ns) {
    GPIO_CLR = bits;
    delay_ns(RES_NS, one_cycle_ns);
}

static inline float get_one_cycle_ns() {
    int n = 100000;
    //printf("timing busy cycles...\n");
    struct timespec t0;
    struct timespec t1;
    float one_cycle_ns = 0.0f;
    while(one_cycle_ns <= 0.0f) {
        clock_gettime(CLOCK_MONOTONIC, &t0);
        delay_cycles(n);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        long int dt_ns = t1.tv_nsec - t0.tv_nsec;
        one_cycle_ns = (float)dt_ns / n;
        //printf("delay_cycles(%d): %ld ns\n", n, dt_ns);
        //printf("one_cycle: %f ns\n", one_cycle_ns);
    }
    return (one_cycle_ns);
}
 
int main(int argc, char **argv) {
    int g,rep;
    setup_io();
    for (g=0; g<=14; g++) {
        INP_GPIO(g);
        OUT_GPIO(g);
    }
    while(1) {
        long int one_cycle_ns = get_one_cycle_ns();
        for(int i = 0; i < 100; ++i) {
            send_res(1<<2, one_cycle_ns);
            for(int i = 0; i < 600; ++i) {
                    send_bits(0<<2, 1<<2, one_cycle_ns);
                    send_bits(0<<2, 1<<2, one_cycle_ns);
                    send_bits(0<<2, 1<<2, one_cycle_ns);
                    send_bits(0<<2, 1<<2, one_cycle_ns);
                    send_bits(0<<2, 1<<2, one_cycle_ns);
                    send_bits(0<<2, 1<<2, one_cycle_ns);
                    send_bits(0<<2, 1<<2, one_cycle_ns);
                    send_bits(0<<2, 1<<2, one_cycle_ns);
                    send_bits(0<<2, 1<<2, one_cycle_ns);
                    send_bits(0<<2, 1<<2, one_cycle_ns);
                    send_bits(0<<2, 1<<2, one_cycle_ns);
                    send_bits(0<<2, 1<<2, one_cycle_ns);
                    send_bits(0<<2, 1<<2, one_cycle_ns);
                    send_bits(0<<2, 1<<2, one_cycle_ns);
                    send_bits(0<<2, 1<<2, one_cycle_ns);
                    send_bits(0<<2, 1<<2, one_cycle_ns);
                    send_bits(1<<2, 1<<2, one_cycle_ns);
                    send_bits(1<<2, 1<<2, one_cycle_ns);
                    send_bits(1<<2, 1<<2, one_cycle_ns);
                    send_bits(1<<2, 1<<2, one_cycle_ns);
                    send_bits(1<<2, 1<<2, one_cycle_ns);
                    send_bits(1<<2, 1<<2, one_cycle_ns);
                    send_bits(1<<2, 1<<2, one_cycle_ns);
                    send_bits(1<<2, 1<<2, one_cycle_ns);
            }
        }
    }
    return 0;
}

