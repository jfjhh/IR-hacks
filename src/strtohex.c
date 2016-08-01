#include "strtohex.h"

static int tohex(uint8_t c)
{
	if (c == '\0')
		return 0;
	else if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'a' && c <= 'f')
		return (c - 'a') + 0xA;
	else if (c >= 'A' && c <= 'F')
		return (c - 'A') + 0xA;
	else
		return EOF;
}

uint8_t *strtohex(const uint8_t *ascii, size_t inlen)
{
	uint8_t *out, c, d;

	if (!(out = malloc(sizeof(uint8_t) * (inlen >> 1))))
		return NULL;

	for (size_t i = 0; i < inlen; i += 2) {
		c = tohex(ascii[i]);
		d = tohex(ascii[i + 1]);

		if ((c == EOF) || (d == EOF)) {
			free(out);
			return NULL;
		}

		/* bits: CCCCDDDD */
		out[i/2] = ((uint8_t) c << 4) | (uint8_t) d;
	}

	return out;
}

