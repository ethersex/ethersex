#
# scripts/rules.mk
#
# Make rules common to all sub-directories of the build system.
# This file must NOT be sourced from the Makefile in ${TOPDIR}.
# Rules that should be shared by the TOPDIR-Makefile even also
# should go into ${TOPDIR}/defaults.mk
#

ifneq ($(rootbuild),t)
${ECMD_PARSER_SUPPORT}_SRC += ${y_ECMD_SRC}
SRC += ${y_SRC}
ASRC += ${y_ASRC}

OBJECTS += $(patsubst %.c,%.o,${SRC})
OBJECTS += $(patsubst %.S,%.o,${ASRC})

# This is NOT a root build, i.e. make has been called directly
# within a subdirectory.  Override the SUBDIR variable and declare a
# suitable target.
compile-subdir:
	[ "x${OBJECTS}" = "x " ] || $(MAKE) no_deps=t -C $(TOPDIR) $(OBJECTS)

clean-common:
	cd $(TOPDIR) && $(RM) $(OBJECTS) \
		$(patsubst %.o,%.dep,${OBJECTS}) \
		$(patsubst %.o,%.E,${OBJECTS}) \
		$(patsubst %.o,%.s,${OBJECTS}) \
		$(CLEAN_FILES)

clean: clean-common

$(TOPDIR)/pinning.c:
	$(MAKE) -C $(TOPDIR) pinning.c

#$(TOPDIR)/control6/control6.h:
#	$(MAKE) -C $(TOPDIR)/control6 control6.h

include $(TOPDIR)/scripts/depend.mk
include $(TOPDIR)/scripts/defaults.mk
endif
