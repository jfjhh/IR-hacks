/* Feature test macro for usleep(3) */
#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif /* _BSD_SOURCE */

/**
 * Resource:
 * <http://elinux.org/index.php?title=RPi_Low-level_peripherals&oldid=373040#GPIO_Code_examples>
 */

#define BCM2708_PERI_BASE	0x20000000
#define GPIO_BASE	(BCM2708_PERI_BASE + 0x200000) /* GPIO controller */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>

#include "strtohex.h"

#define PAGE_SIZE	4096
#define BLOCK_SIZE	4096

static int mem_fd;
static void *gpio_map;

/* I/O access */
static volatile unsigned *gpio;

/* GPIO setup macros.
 * Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y) */
#define INP_GPIO(g) \
	*(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))

#define OUT_GPIO(g) \
	*(gpio+((g)/10)) |=  (1<<(((g)%10)*3))

#define SET_GPIO_ALT(g,a) \
	*(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

/* sets bits which are 1 ignores bits which are 0 */
#define GPIO_SET	*(gpio+7)

/* clears bits which are 1 ignores bits which are 0 */
#define GPIO_CLR	*(gpio+10)

/* 0 if LOW, (1<<g) if HIGH */
#define GET_GPIO(g)	(*(gpio+13)&(1<<g))

/* Pull up/pull down */
#define GPIO_PULL *(gpio+37)

/* Pull up/pull down clock */
#define GPIO_PULLCLK0 *(gpio+38)

/**
 * Set up a memory regions to access GPIO
 */
void setup_io(void)
{
	/* Open "/dev/mem" */
	if ((mem_fd = open("/dev/mem", O_RDWR | O_SYNC)) < 0) {
		puts("can't open /dev/mem");
		exit(EXIT_FAILURE);
	}

	/* mmap GPIO */
	gpio_map = mmap(
			NULL,                 /* Any address in our space will do.  */
			BLOCK_SIZE,           /* Map length.                        */
			PROT_READ|PROT_WRITE, /* Enable RW perms to mapped memory.  */
			MAP_SHARED,           /* Shared with other processes.       */
			mem_fd,               /* File to map.                       */
			GPIO_BASE);           /* Offset to GPIO peripheral.         */

	close(mem_fd); /* No need to keep mem_fd open after mmap. */

	if (gpio_map == MAP_FAILED) {
		printf("mmap error %p\n", MAP_FAILED); /* errno also set!. */
		exit(EXIT_FAILURE);
	}

	/* Always use volatile pointer! */
	gpio = (volatile unsigned *) gpio_map;
}

int main(int argc, char **argv)
{
	struct timeval tv_a = { 0 }, tv_b = { 0 };
	uint8_t *hexbuf, *codebuf, *tmp;
	FILE *code_file;
	long size;
	int c;

	if (argc != 2 || !argv[1] || !(code_file = fopen(argv[1], "r"))) {
		if (argv[1])
			fprintf(stderr, "Could not open IR code file '%s'!\n", argv[1]);
		else
			fputs("Specify an IR code file!\n", stderr);

		return EXIT_FAILURE;
	}

	if (fseek(code_file, 0L, SEEK_END) == -1) {
		perror("fseek to end of code file");
		fclose(code_file);
		return EXIT_FAILURE;
	}

	if ((size = ftell(code_file)) == -1) {
		perror("ftell code file's size");
		fclose(code_file);
		return EXIT_FAILURE;
	}
	
	if (!(hexbuf = malloc(sizeof(uint8_t) * size))) {
		fputs("Could not allocate hex buffer memory!", stderr);
		return EXIT_FAILURE;
	}

	tmp = hexbuf;
	while ((c = fgetc(code_file)) != EOF)
		*(tmp++) = (uint8_t) c;

	if (!(codebuf = strtohex(hexbuf, size))) {
		fputs("Could not convert hex codes to raw codes!", stderr);
		free(hexbuf);
		return EXIT_FAILURE;
	}

	fclose(code_file);
	/* codebuf is now the list of addr/cmd words (4 bytes long, each). */

	/* Set up gpi pointer for direct register access. */
	setup_io();

	/**
	 * You are about to change the GPIO settings of your computer.
	 * Mess this up and it will stop working!
	 * It might be a good idea to `sync` before running this program,
	 * so at least you still have your code changes written to the SD-card!
	 */

	/* Toggle pin 4 as fast as possible. */
	INP_GPIO(16);
	OUT_GPIO(16);

	if (gettimeofday(&tv_a, NULL) == -1)
		goto exit;
	GPIO_SET = 1<<16;
	GPIO_CLR = 1<<16;
	if (gettimeofday(&tv_b, NULL) == -1)
		goto exit;

	fprintf(stderr, "Time for one pulse (SET then CLR): %uus\n",
			(unsigned int) (tv_b.tv_usec - tv_a.tv_usec));

exit: /* Fallthrough. */
	free(codebuf);
	return EXIT_SUCCESS;
}

