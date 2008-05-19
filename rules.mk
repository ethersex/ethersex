
OBJECTS += $(patsubst %.c,%.o,${SRC})

%.a: $(OBJECTS)
	$(RM) $@
	$(AR) qcv $@ $^
#	$(STRIP) --strip-unneeded $@

%.s: %.c
	$(CC) -o $@ -O0 $(CPPFLAGS) -S $<

clean-common:
	$(RM) $(TARGET) *.[oda] *.d.new *~

clean: clean-common

all:
	make -C $(TOPDIR) all

include $(TOPDIR)/depend.mk
