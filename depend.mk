# How to build automatic dependencies

# Don't include dependencies if $(no_deps) is set; the master makefile
# does this for clean and other such targets that don't need
# dependencies.  That then avoids rebuilding dependencies.

ifneq ($(no_deps),t)
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),mrproper)
ifneq ($(MAKECMDGOALS),menuconfig)  

# For each .o file we need a .d file.
include $(foreach file,$(subst .o,.d,$(filter %.o,$(OBJECTS))),$(TOPDIR)/${file}) /dev/null

endif
endif
endif
endif

# Here is how to build those dependency files

define make-deps
set -e; $(CC) $(CFLAGS) $(CPPFLAGS) -M -MM $<  | \
sed > $@.new -e 's;$(*F)\.o:;$@ $*.o $*.E $*.s:;' \
	     -e 's% [^ ]*/gcc-lib/[^ ]*\.h%%g'
if test -s $@.new; then mv -f $@.new $@; else rm -f $@.new; fi
endef

# Here is how to make .d files from .c files
%.d: %.c $(TOPDIR)/control6/control6.h $(TOPDIR)/pinning.c; $(make-deps)

%.d: %.S ; $(make-deps)
