#include <linux/init.h>
#include <linux/module.h>
#include <linux/ktime.h>
#include <linux/gpio.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#define GPIO2 2
#define THREAD_NAME "ws2812"

#define T0H_PS 350
#define T1H_PS 900
#define T0L_PS 900
#define T0H_PS 350
#define RES_PS 55000
#define T1_PS (T0H_PS)
#define T2_PS (T1H_PS)
#define T3_PS ((T0H_PS) + (T0L_PS))
#define D1_PS (T1_PS)
#define D2_PS ((T2_PS) - (T1_PS))
#define D3_PS ((T3_PS) - (T2_PS))

static inline void delay_cycles(u32 n) {
    int i;
	for(i = 0; i < n; ++i) {
        asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
        asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
	}
}

static inline u32 get_one_cycle_ps(void) {
    ktime_t t0;
    ktime_t t1;
    u32 n = 100000;
    u32 one_cycle_ps = 0;
    while(one_cycle_ps <= 0) {
	    u64 dt_ps;
        t0 = ktime_get();
        delay_cycles(n);
        t1 = ktime_get();
        dt_ps = ktime_to_ns(ktime_sub(t1, t0)) * 1000;
        do_div(dt_ps, n);
        one_cycle_ps = dt_ps;
    }
    return (one_cycle_ps);
}

static inline void delay_ps(u64 t_ps, u32 one_cycle_ps) {
    u32 n;
    do_div(t_ps, one_cycle_ps);
    n = t_ps - 3;
    delay_cycles(n);
}

static inline void send_res(u32 gpio, u32 one_cycle_ps) {
    gpio_set_value(gpio, 0);
    delay_ps(RES_PS, one_cycle_ps);
}

static inline void send_bit(u32 gpio, u32 bit, u32 one_cycle_ps) {
    gpio_set_value(gpio, 1);
    delay_ps(D1_PS, one_cycle_ps);
    gpio_set_value(gpio, !bit);
    delay_ps(D2_PS, one_cycle_ps);
    gpio_set_value(gpio, bit);
    delay_ps(D3_PS, one_cycle_ps);
}

int ws2812_thread(void *data) {
    while(!kthread_should_stop()) {
        u16 i;
        u32 one_cycle_ps = get_one_cycle_ps();
        for(i = 0; i < 100; ++i) {
            u16 j;
            send_res(1<<2, one_cycle_ps);
            for(j = 0; j < 600; ++j) {
               send_bit(GPIO2, 0, one_cycle_ps);
               send_bit(GPIO2, 0, one_cycle_ps);
               send_bit(GPIO2, 0, one_cycle_ps);
               send_bit(GPIO2, 0, one_cycle_ps);
               send_bit(GPIO2, 0, one_cycle_ps);
               send_bit(GPIO2, 0, one_cycle_ps);
               send_bit(GPIO2, 0, one_cycle_ps);
               send_bit(GPIO2, 0, one_cycle_ps);
               send_bit(GPIO2, 0, one_cycle_ps);
               send_bit(GPIO2, 0, one_cycle_ps);
               send_bit(GPIO2, 0, one_cycle_ps);
               send_bit(GPIO2, 0, one_cycle_ps);
               send_bit(GPIO2, 0, one_cycle_ps);
               send_bit(GPIO2, 0, one_cycle_ps);
               send_bit(GPIO2, 0, one_cycle_ps);
               send_bit(GPIO2, 0, one_cycle_ps);
               send_bit(GPIO2, 1, one_cycle_ps);
               send_bit(GPIO2, 1, one_cycle_ps);
               send_bit(GPIO2, 1, one_cycle_ps);
               send_bit(GPIO2, 1, one_cycle_ps);
               send_bit(GPIO2, 1, one_cycle_ps);
               send_bit(GPIO2, 1, one_cycle_ps);
               send_bit(GPIO2, 1, one_cycle_ps);
               send_bit(GPIO2, 1, one_cycle_ps);
            }
        }
    }
    return 0;
}

struct task_struct *task;

void ws2812_thread_init(void) {
    printk("ws2812: starting thread...");
    task = kthread_run(ws2812_thread, NULL, THREAD_NAME);
    printk("ws2812: thread started.");
}

void ws2812_thread_exit(void) {
    printk("ws2812: stopping thread...");
    kthread_stop(task);
    printk("ws2812: thread stopped.");
}

static int __init ws2812_init(void) {
    printk(KERN_INFO "ws2812: starting...");
    gpio_request(GPIO2, "GPIO2");
    gpio_direction_output(GPIO2, 0);
    printk(KERN_INFO "ws2812: started.");
    return 0;
}

static void __exit ws2812_exit(void) {
    printk(KERN_INFO "ws2812: stopping...");
    printk(KERN_INFO "ws2812: stopped.");
}

module_init(ws2812_init);
module_exit(ws2812_exit);
