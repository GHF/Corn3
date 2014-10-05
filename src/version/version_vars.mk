
ifeq ($(VERSIONDIR),)
  VERSIONDIR = src/version
endif

# Grab git commit name
GIT_VERSION = $(shell git describe --dirty --always)

# The default version will be overwritten
VERSIONSRC = $(VERSIONDIR)/version.c

# This ensures every linker invocation gets a fresh timestamp.
VERSIONLIBS = $(VERSIONDIR)/build_time.c
