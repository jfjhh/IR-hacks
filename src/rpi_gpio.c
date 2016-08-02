/* Feature test macro for usleep(3) */
#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif /* _BSD_SOURCE */

/**
 * Resources:
 * <http://elinux.org/index.php?title=RPi_Low-level_peripherals&oldid=373040#GPIO_Code_examples>
 * WiringPi source (wiringPi.c).
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <errno.h>

/**
 * BCM Magic!
 */
#define BCM_MAGIC	0x5A000000

/**
 * GPIO Port function select bits.
 */
#define	FSEL_INPT	0b000
#define	FSEL_OUTP	0b001
#define	FSEL_ALT0	0b100
#define	FSEL_ALT1	0b101
#define	FSEL_ALT2	0b110
#define	FSEL_ALT3	0b111
#define	FSEL_ALT4	0b011
#define	FSEL_ALT5	0b010

/**
 * Memory parameters.
 */
#define PAGE_SIZE	4096
#define BLOCK_SIZE	4096

/**
 * Word offsets into the PWM control region.
 */
#define PWM_CONTROL	0x0
#define PWM_STATUS 	0x1
#define PWM0_RANGE 	0x4
#define PWM0_DATA  	0x5
#define PWM1_RANGE 	0x8
#define PWM1_DATA  	0x9

/**
 * Clock register offsets.
 */
#define PWMCLK_CNTL	40
#define PWMCLK_DIV	41

/**
 * PWM CTL register.
 */
#define PWM0_MS_MODE	0x0080 /**< Run in MS mode. */
#define PWM0_USEFIFO	0x0020 /**< Data from FIFO. */
#define PWM0_REVPOLAR	0x0010 /**< Reverse polarity. */
#define PWM0_OFFSTATE	0x0008 /**< Ouput Off state. */
#define PWM0_REPEATFF	0x0004 /**< Repeat last value if FIFO empty. */
#define PWM0_SERIAL		0x0002 /**< Run in serial mode. */
#define PWM0_ENABLE		0x0001 /**< Channel Enable. */
#define PWM1_MS_MODE	0x8000 /**< Run in MS mode. */
#define PWM1_USEFIFO	0x2000 /**< Data from FIFO. */
#define PWM1_REVPOLAR	0x1000 /**< Reverse polarity. */
#define PWM1_OFFSTATE	0x0800 /**< Ouput Off state. */
#define PWM1_REPEATFF	0x0400 /**< Repeat last value if FIFO empty. */
#define PWM1_SERIAL		0x0200 /**< Run in serial mode. */
#define PWM1_ENABLE		0x0100 /**< Channel Enable. */

/* I/O access */
static int mem_fd;
static uint32_t *gpio_map, *pwm_map, *clk_map;
static volatile uint32_t *gpio, *pwm, *clk;

/* Peripheral base address. */
static volatile uint32_t BCM2708_PERI_BASE;

/* GPIO control offsets. */
static volatile uint32_t GPIO_PADS;
static volatile uint32_t GPIO_CLOCK_BASE;
static volatile uint32_t GPIO_BASE;
static volatile uint32_t GPIO_TIMER;
static volatile uint32_t GPIO_PWM;

/* GPIO setup macros.
 * Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y) */
#define INP_GPIO(g) \
	(*(gpio+((g)/10)) &= ~(7<<(((g)%10)*3)))

#define OUT_GPIO(g) \
	(*(gpio+((g)/10)) |=  (1<<(((g)%10)*3)))

#define SET_GPIO_ALT(g,a) \
	(*(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3)))

/* sets bits which are 1 ignores bits which are 0 */
#define GPIO_SET	(*(gpio+7))

/* clears bits which are 1 ignores bits which are 0 */
#define GPIO_CLR	(*(gpio+10))

/* 0 if LOW, (1<<g) if HIGH */
#define GET_GPIO(g)	(*(gpio+13)&(1<<g))

/* Pull up/pull down */
#define GPIO_PULL	(*(gpio+37))

/* Pull up/pull down clock */
#define GPIO_PULLCLK0	(*(gpio+38))

static void set_PWM_divisor(int divisor)
{
	/* The base clock frequency is 19.2 MHz.
	 * E.g. To get a 38 kHz carrier wave for IR, the divisor must be 500. */
	uint32_t pwm_control;
	divisor &= 4095;

	pwm_control = *(pwm + PWM_CONTROL); /* Save PWM_CONTROL. */

	/* We need to stop PWM prior to stopping PWM in MS mode otherwise BUSY stays
	 * high. */
	*(pwm + PWM_CONTROL) = 0; /* Stop PWM. */

	/**
	 * Stop PWM before changing divisor. The delay after this does need to this
	 * big (95uS occasionally fails, 100uS OK), it's almost as though the BUSY
	 * flag is not working properly in balanced mode. Without the delay when DIV
	 * is adjusted the clock sometimes switches to very slow, once slow further
	 * DIV adjustments do nothing and it's difficult to get out of this mode.
	 */
	*(clk + PWMCLK_CNTL) = BCM_MAGIC | 0x01; /* Stop PWM. */
	/* delayMicroseconds(110); /1* prevents clock going sloooow. *1/ */
	usleep(110);

	while ((*(clk + PWMCLK_CNTL) & 0x80) != 0) /* Wait for clock to be !BUSY */
		/* delayMicroseconds(1); */
		usleep(1);

	*(clk + PWMCLK_DIV)  = BCM_MAGIC | (divisor << 12);
	*(clk + PWMCLK_CNTL) = BCM_MAGIC | 0x11; /* Start PWM.           */
	*(pwm + PWM_CONTROL) = pwm_control;      /* Restore PWM_CONTROL. */
}

/**
 * Setup memory regions to access GPIO.
 */
void setup_io(void)
{
	/* Open "/dev/mem". I'm using PWM, so I won't bother with trying to use
	 * "/dev/gpiomem", as it doesn't work for PWM (currently). */
	if ((mem_fd = open("/dev/mem", O_RDWR | O_SYNC)) < 0) {
		puts("can't open /dev/mem");
		exit(EXIT_FAILURE);
	}

	/* mmap GPIO. */
	gpio_map = mmap(
			NULL,                   /* Any address in our space will do.  */
			BLOCK_SIZE,             /* Map length.                        */
			PROT_READ | PROT_WRITE, /* Enable RW perms to mapped memory.  */
			MAP_SHARED,             /* Shared with other processes.       */
			mem_fd,                 /* File to map.                       */
			GPIO_BASE);             /* Offset to GPIO peripheral.         */

	pwm_map = mmap(
			NULL,
			BLOCK_SIZE,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			mem_fd,
			GPIO_PWM);

	clk_map = mmap(
			NULL,
			BLOCK_SIZE,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			mem_fd,
			GPIO_CLOCK_BASE);

	close(mem_fd); /* No need to keep mem_fd open after mmap. */

	if (gpio_map == MAP_FAILED
			|| pwm_map == MAP_FAILED
			|| clk_map == MAP_FAILED) {
		printf("mmap error %p\n", MAP_FAILED);
		exit(EXIT_FAILURE);
	}

	/* Always use volatile pointer! */
	gpio = (volatile uint32_t *) gpio_map;
	pwm  = (volatile uint32_t *) pwm_map;
	clk  = (volatile uint32_t *) clk_map;

	/* Need a 341 / 1024 M-S ratio.
	 * (8.77us / 26.3us pulses)
	 * (33.346% duty cycle) for PWM. */
	*(pwm + PWM_CONTROL) = PWM0_ENABLE | PWM0_MS_MODE;
	*(pwm + PWM0_DATA)   = 341;
	usleep(10);
	*(pwm + PWM0_RANGE)  = 1023;
	usleep(10);
	set_PWM_divisor(500); /* For a 38 kHz carrier signal. */
}

static void op_priority(void)
{
	struct sched_param sched = { 0 };
	struct rlimit rlim = { 0 };
	int priority;

	/* Get as big a CPU timeslice as possible (Under default scheduling). */
	if (getrlimit(RLIMIT_NICE, &rlim) == -1) {
		perror("getrlimit RLIMIT_NICE");
		exit(EXIT_FAILURE);
	}

	errno = 0;
	priority = getpriority(PRIO_PROCESS, getpid());
	if (errno) {
		perror("getpriority on self process");
		exit(EXIT_FAILURE);
	}

	if (rlim.rlim_cur < rlim.rlim_max) {
		rlim.rlim_cur = rlim.rlim_max;
		if (!setrlimit(RLIMIT_NICE, &rlim)) {
			perror("setrlimit RLIMIT_NICE max");
			exit(EXIT_FAILURE);
		}
	}

	if (nice(-priority) == -1) {
		perror("nice to highest priority");
		exit(EXIT_FAILURE);
	}

	/**
	 * Set high scheduling priority. Should be better than SCHED_OTHER and a
	 * greedy nice value, but I've set the nice value above just in case.
	 *
	 * SCHED_FIFO seems faster than SCHED_RR, but if a maximum thread time
	 * quantum is desired, then just change from SCHED_FIFO to SCHED_RR.
	 */
	if ((sched.sched_priority = sched_get_priority_max(SCHED_FIFO)) == -1) {
		perror("sched_get_priority_max for SCHED_FIFO");
		exit(EXIT_FAILURE);
	}

	if (sched_setscheduler(0, SCHED_FIFO, &sched) == -1) {
		perror("sched_setscheduler max priority");
		exit(EXIT_FAILURE);
	}
}

int main(void)
{
	struct timeval tv_a = { 0 }, tv_b = { 0 };

	/* Set highest possible priority. */
	op_priority();

	/* Set up I/O pointers for direct register access. */
	BCM2708_PERI_BASE = 0x20000000;
	GPIO_PADS         = BCM2708_PERI_BASE + 0x00100000;
	GPIO_CLOCK_BASE   = BCM2708_PERI_BASE + 0x00101000;
	GPIO_BASE         = BCM2708_PERI_BASE + 0x00200000;
	GPIO_TIMER        = BCM2708_PERI_BASE + 0x0000B000;
	GPIO_PWM          = BCM2708_PERI_BASE + 0x0020C000;

	setup_io();

	/**
	 * You are about to change the GPIO settings of your computer.
	 * Mess this up and it will stop working!
	 * It might be a good idea to `sync` before running this program,
	 * so at least you still have your code changes written to the SD-card!
	 */

	INP_GPIO(18);

	/* Toggle pin 18 with carrier frequency PWM. */
	SET_GPIO_ALT(18, 5); /* ALT5 makes pin 18 do PWM0. */

	/* Toggle pin 18 as fast as possible. */
	/* OUT_GPIO(18); */

	/* Block for user input so the PWM can be measured with an oscilloscope. */
	getchar();

	/* Disable PWM. */
	/* *(pwm + PWM0_DATA) */
	/* 	= *(pwm + PWM1_DATA) */
	/* 	= *(pwm + PWM_CONTROL) = 0; */
	/* INP_GPIO(18); */

	for (;;) {
		SET_GPIO_ALT(18, 5); /* ALT5 makes pin 18 do PWM0. */
		usleep(5e5);
		INP_GPIO(18);
		usleep(5e5);
	}

	if (gettimeofday(&tv_a, NULL) == -1)
		goto exit;

	/* <codeandlife.com> RPi C GPIO benchmark says f = 22MHz. */
	for (size_t i = 0; i < 22e6; i++) {
		GPIO_SET = 1<<18;
		GPIO_CLR = 1<<18;
	}

	if (gettimeofday(&tv_b, NULL) == -1)
		goto exit;

	printf("%0.13Lf\n",
			((tv_b.tv_sec - tv_a.tv_sec)
			 + ((tv_b.tv_usec - tv_a.tv_usec) * 1e-6L)
			 - 3.2065e-6L) /* Benchmarked time for `gettimeofday(2)` calls. */
			/ 22e6L);
	/* Mean time for 1000 trials was 64.8755 nanoseconds. */

exit: /* Fallthrough. */
	/* Disable outputs, to try to return system to a sane state. */

	/* Disable the PWM (In M-S, M=0 => On 0% of the time and CONTROL off. */
	if (pwm) {
		*(pwm + PWM0_DATA)
			= *(pwm + PWM1_DATA)
			= *(pwm + PWM_CONTROL) = 0;
	}

	/* Turn off the GPIO pin. */
	if (gpio) {
		GPIO_CLR = 1<<16;
	}

	return EXIT_SUCCESS;
}

