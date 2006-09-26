# Disable asserts.
#OPTIONS = -DNDEBUG

CC = gcc
CFLAGS = -Wall -Wstrict-prototypes -std=gnu99 -ggdb3 -fno-inline -O2 -Werror
CFLAGS += $(OPTIONS)
CFLAGS += -DDEBUG -Werror
CFLAGS += $(CEXTRA)
#LDFLAGS += -lm

# Name of Makefile for make depend
MAKEFILE = Makefile

SRC = $(shell echo *.c)
OBJECTS += $(patsubst %.c,%.o,${SRC})
TARGETS=ethcmd

all: $(TARGETS)

$(TARGETS): $(OBJECTS)

.PHONY: clean clean-target distclean depend undepend

clean: clean-target

clean-target:
	rm -f $(TARGETS)
	rm -f $(OBJECTS)

distclean: clean
	rm -f $(MAKEFILE).dep

depend:
	$(CC) -M $(CDEFS) $(CINCS) $(SRC) $(ASRC) >> $(MAKEFILE).dep

-include $(MAKEFILE).dep
