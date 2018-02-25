#define BCM2708_PERI_BASE        0x20000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)
int  mem_fd;
void *gpio_map;
volatile unsigned *gpio;
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))
#define GPIO_SET *(gpio+7)
#define GPIO_CLR *(gpio+10)
#define GET_GPIO(g) (*(gpio+13)&(1<<g))
#define GPIO_PULL *(gpio+37)
#define GPIO_PULLCLK0 *(gpio+38)
 
void setup_io();
inline long int get_one_cycle_ns();
inline void update(int* gpio_set, int* gpio_clr, int n) { for(int i; i < n; i++) { GPIO_SET = gpio_set[i]; GPIO_CLR = gpio_clr[i]; } }
inline void delay_ns(int t_ns, int one_cycle_ns) { int n = (time_ns / one_cycle_ns - 5); delay_cycles(n); }
inline void delay_cycles(int n) { for(int i = 0; i < n; ++i) {} }

int main(int argc, char **argv) {
  int g,rep;
  setup_io();
  for (g=7; g<=11; g++) {
    INP_GPIO(g);
    OUT_GPIO(g);
  }
  long int t_cycle = get_one_cycle_ns();
  return 0;
}


inline get_one_cycle_ns() {
    int gpio_set[10000] = { 0 };
    int gpio_clr[10000] = { 0 };
    int n = 100000;
    printf("timing %d gpio updates...\n", n);
    struct timespec t0;
    struct timespec t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    delay_cycles(n);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    long int dt = t1.tv_nsec - t0.tv_nsec;
    long int t_cycle = dt / n;
    printf("delay_cycles(%d): %ld ns\n", n, dt);
    printf("one_cycle: %ld ns\n", t_cycle);
    return dt/n;
}
 
 
void setup_io() {
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
