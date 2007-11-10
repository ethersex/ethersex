# How to build automatic dependencies

# Don't include dependencies if $(no_deps) is set; the master makefile
# does this for clean and other such targets that don't need
# dependencies.  That then avoids rebuilding dependencies.

ifneq ($(no_deps),t)
ifneq ($(@F),clean)

# For each .o file we need a .d file.
-include $(subst .o,.d,$(filter %.o,$(OBJECTS))) /dev/null

endif
endif

# Here is how to build those dependency files

define make-deps
set -e; $(CC) $(CFLAGS) $(CPPFLAGS) -M -MG $<  | \
sed > $@.new -e 's;$(*F)\.o:;$@ $*.o:;' \
	     -e 's% [^ ]*/gcc-lib/[^ ]*\.h%%g'
mv -f $@.new $@
endef

# Here is how to make .d files from .c files
%.d: %.c; $(make-deps)
#	$(CC) $(CFLAGS) $(CPPFLAGS) -M -MG $< > $@
