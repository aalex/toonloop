man_MANS = toonloop.1 
EXTRA_DIST = $(man_MANS) man-toonloop.txt
MAINTAINERCLEANFILES = $(man_MANS)

# The man pages depend on the --help strings and the version number.
common_mandeps = $(top_srcdir)/configure.ac

# Depend on the source, not the binary; we don't need to regenerate the
# binary when any source file changes, only the main one.
# Use -o so that the `missing' program can infer the output file.
toonloop.1: $(top_srcdir)/src/application.cpp $(common_mandeps) $(top_srcdir)/man/man-toonloop.txt
	$(HELP2MAN) --include=man-toonloop.txt --name="The Toonloop Live Animation Performance Tool" --no-info $(top_builddir)/src/toonloop -o $@
