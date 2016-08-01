CC       := gcc
LIBS     += -lm
CFLAGS   += -g -Wall -Werror -pedantic-errors -std=c99 -O3 $(LIBS)
SOURCES  := gen_nec.c rpi_gpio.c strtohex.c
OBJECTS  := $(patsubst %.c,%.o,$(SOURCES))
BINARIES := gen_nec rpi_gpio

all: $(BINARIES)

gen_nec: gen_nec.c

rpi_gpio: rpi_gpio.o strtohex.o strtohex.h

clean:
	@rm -fv $(BINARIES) $(OBJECTS)

.PHONY: clean

