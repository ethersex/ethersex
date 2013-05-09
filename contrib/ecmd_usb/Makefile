# $Id: Makefile 5 2006-01-12 21:41:36Z matled $

CC=gcc
LD=gcc
STRIP=strip
RM=rm -f --

-include Makefile.local

OBJECTS+= ecmd_usb.o
HEADERS+=$(shell echo *.h)
TARGET=ecmd_usb

LDFLAGS+=`libusb-config --libs`
CFLAGS+=-std=c99 -pedantic-errors -Wall -W `libusb-config --cflags`

ifneq ($(DEBUG),) # {{{
	CFLAGS+=-ggdb3
	CFLAGS+=-Wchar-subscripts -Wmissing-prototypes
	CFLAGS+=-Wmissing-declarations -Wredundant-decls
	CFLAGS+=-Wstrict-prototypes -Wshadow -Wbad-function-cast
	CFLAGS+=-Winline -Wpointer-arith -Wsign-compare
	CFLAGS+=-Wunreachable-code -Wdisabled-optimization
	CFLAGS+=-Wcast-align -Wwrite-strings -Wnested-externs -Wundef
	CFLAGS+=-DDEBUG
	LDFLAGS+=
else
	CFLAGS+=-O2
	LDFLAGS+=
endif
# }}}
ifneq ($(PROFILING),) # {{{
	CFLAGS+=-pg -DPROFILING
	LDFLAGS+=-pg
endif
# }}}

all: $(TARGET)

clean:
	$(RM) $(TARGET) $(OBJECTS)

.PHONY: clean all

%.o: $(HEADERS)

$(TARGET): $(OBJECTS) $(TARGET).o
	$(LD) -o $@ $^ $(LDFLAGS)

