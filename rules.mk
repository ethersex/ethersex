
OBJECTS += $(patsubst %.c,%.o,${SRC})

%.a: $(OBJECTS)
	$(RM) $@
	$(AR) qcv $@ $^
#	$(STRIP) --strip-unneeded $@

clean-common:
	$(RM) $(TARGET) *.[oda]

clean: clean-common

include $(TOPDIR)/depend.mk
