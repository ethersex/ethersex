
OBJECTS += $(patsubst %.c,%.o,${SRC})

%.a: $(OBJECTS)
	$(RM) $@
	$(AR) qcv $@ $^
#	$(STRIP) --strip-unneeded $@

clean-common:
	$(RM) $(TARGET) *.[odasE] *.d.new *~

clean: clean-common

all:
	make -C $(TOPDIR) all

include $(TOPDIR)/depend.mk
