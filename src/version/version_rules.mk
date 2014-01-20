
# Create a non-existent target that gets built every time
.PHONY: ALWAYS_MAKE

# Generate C source for git version
$(BUILDDIR)/version.c.newest: ALWAYS_MAKE
	@echo 'const char * const g_build_version = "$(GIT_VERSION)";' > $@

# If the generated C has changed from the current, copy it over
$(VERSIONSRC): $(BUILDDIR)/version.c.newest
	@cmp -s $< $@ || cp $< $@
