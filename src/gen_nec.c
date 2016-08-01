#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define CODE_BITS	16
#define CODE_LIMIT	(1 << CODE_BITS)

int main(int argc, const char **argv)
{
	uint32_t tmp = ~0;
	uint32_t code_offset, code_space = ~0;

	if (argc < 2
			|| !argv[1]
			|| argv[1][1] == 'h') {
		fputs("Please specify NEC code space arguments!\n"
				"\tUsage: ./gen_nec <code space> [code offset]\n",
				stderr);
		return EXIT_FAILURE;
	} else if (sscanf(argv[1], "%u", &tmp) != 1
			|| tmp == 0
			|| tmp > CODE_LIMIT) {
		fprintf(stderr,
				"Code space %u is invalid! Give a count from 1 to 65536.\n",
				tmp);
		return EXIT_FAILURE;
	} else {
		code_space = tmp;
	}

	if (!argv[2]) {
		code_offset = 0;
	} else if (sscanf(argv[2], "%u", &tmp) != 1
			|| tmp + code_space >= CODE_LIMIT) {
		fprintf(stderr,
				"Code offset %u makes space %u go to limit %u (off by %u)!\n"
				"\tSpecify a smaller limit.\n",
				code_space, tmp, CODE_LIMIT, tmp + code_space - CODE_LIMIT);
		return EXIT_FAILURE;
	} else {
		code_offset = tmp;
	}

	fprintf(stderr, "Code Space\t%5u\n"
			"Code Offset\t%5u\n"
			"Code Limit\t%5u\n"
			"Code Range\t[%u, %u] (%u codes total, 2^%0.5f)\n\n",
			code_space, code_offset, CODE_LIMIT,
			code_offset, code_space + code_offset - 1u,
			code_space, log2f(code_space));

	for (uint32_t i = code_offset; i < code_offset + code_space; i++) {
		uint8_t addr, cmd, naddr, ncmd;
		addr  = i & ((1 << (CODE_BITS >> 1)) - 1);
		cmd   = i >> (CODE_BITS >> 1);
		naddr = ~addr;
		ncmd  = ~cmd;
		printf("%02x%02x%02x%02x", addr, naddr, cmd, ncmd);
	}

	return EXIT_SUCCESS;
}

