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

TARGETS=ethcmd
OBJ=ethcmd.o

all: $(TARGETS)

.PHONY: clean clean-target depend undepend

clean: clean-target

clean-target:
	rm -f $(TARGETS)
	rm -f $(OBJ)

depend:
	$(CC) -M $(CDEFS) $(CINCS) $(SRC) $(ASRC) >> $(MAKEFILE).dep

-include $(MAKEFILE).dep
