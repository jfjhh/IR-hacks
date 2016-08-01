#ifndef STRTOHEX_H
#define STRTOHEX_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/**
 * Converts an ascii-encoded hex string into a raw buffer of the hex bytes
 * 
 * E.g. "deadbeef" => <0xdeadbeef>.
 * 0000000: 6465 6164 6265 6566                      deadbeef
 * becomes:
 * 0000000: dead beef                                ....
 *
 * @param ascii  The ascii-encoded hex data.
 * @param outlen The length of the input ascii (2x as much as the hex bytes).
 *
 * @returns a malloc'd buffer to the hex bytes, or @c NULL on error.
 */
uint8_t *strtohex(const uint8_t *ascii, size_t inlen);

#endif /* STRTOHEX_H */

