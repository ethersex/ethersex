
OBJECTS += $(patsubst %.c,%.o,${SRC})
OBJECTS += $(patsubst %.S,%.o,${ASRC})

%.a: $(OBJECTS)
	$(RM) $@
	$(AR) qcv $@ $^
#	$(STRIP) --strip-unneeded $@

%.o: %.S
	$(CC) -o $@ $(CPPFLAGS) $(ASFLAGS) -c $<

clean-common:
	$(RM) $(TARGET) *.[odasE] *.d.new *~

clean: clean-common

all:
	make -C $(TOPDIR) all

include $(TOPDIR)/depend.mk
