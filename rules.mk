#
# rules.mk
#
# Make rules common to all sub-directories of the build system.
# This file must NOT be sourced from the Makefile in ${TOPDIR}.
# Rules that should be shared by the TOPDIR-Makefile even also
# should go into ${TOPDIR}/defaults.mk
#

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

$(TOPDIR)/pinning.c:
	make -C $(TOPDIR) pinning.c

include $(TOPDIR)/depend.mk
