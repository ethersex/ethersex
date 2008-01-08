
OBJECTS += $(patsubst %.c,%.o,${SRC})

%.a: $(OBJECTS)
	$(RM) $@
	$(AR) qcv $@ $^
#	$(STRIP) --strip-unneeded $@

clean-common:
	$(RM) $(TARGET) *.[oda]

clean: clean-common

all:
	make -C $(TOPDIR) all

include $(TOPDIR)/depend.mk
