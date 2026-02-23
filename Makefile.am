AM_CPPFLAGS = -I $(srcdir)/include

AM_CFLAGS = -O3 -g0 -Wall

ARFLAGS = cr

lib_LIBRARIES = lib/libstrings.a
lib_libstrings_a_SOURCES = src/strings.c include/libstrings.h

bin_PROGRAMS = bin/test-strings
bin_test_strings_SOURCES = src/test-strings.c
bin_test_strings_LDADD = lib/libstrings.a $(AVL_LIBS)

include_HEADERS = include/libstrings.h

EXTRA_DIST = windows acdoxygen.m4 amdoxygen.am doxygen.libstrings.cfg libstrings.pc .gitignore

super-clean: clean distclean
	@find . -name 'Makefile.in' -delete
	@find . -name 'config.h.in' -delete
	@find . -name '*.tar' -delete
	@find . -name '*.tar.*' -delete
	@find . -name '*~' -delete
	@find . -name '*.obj' -delete
	@find . -name '*.lib' -delete
	@find . -name '*.exe' -delete
	@rm -rf autom4te.cache
	@rm -f aclocal.m4
	@rm -f configure
	@rm -rf src/.deps
	@rm -f texput.log

include amdoxygen.am

MOSTLYCLEANFILES = $(DX_CLEANFILES)

EXTRA_DIST += $(DX_CONFIG)

install-data-hook:
	@if test -d $(DX_HTML_DIR); \
	 then \
	   echo installing $(DX_HTML_DIR) to $(prefix)/share/doc/libstrings; \
	   mkdir -p $(prefix)/share/doc/libstrings; \
	   cp -r $(DX_HTML_DIR) $(prefix)/share/doc/libstrings; \
	 fi
	@echo installing libstrings.pc
	@mkdir -p $(prefix)/share/pkgconfig
	@cp libstrings.pc $(prefix)/share/pkgconfig/libstrings.pc

uninstall-hook:
	@rm -rf $(prefix)/share/doc/libstrings
	@rm -rf $(prefix)/share/pkgconfig/libstrings.pc

code-stats:
	@echo -n " total lines of source code: "; wc -l src/*.[ch] include/*.[ch] | tail -1 | sed -e 's/ total//' -e 's/^  *//'
	@echo -n "actual lines of source code: "; (gcc -fpreprocessed -dD -E src/*.[ch] include/*.[ch] | sed -e '/^$$/d' -e '/^# [0-9]/d' | wc -l)

