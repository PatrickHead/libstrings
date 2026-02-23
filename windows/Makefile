SRCDIR = ../src
INCLDIR = ../include

WINPKGSDIR = $(HOME)/develop/win-pkgs
WINPKGINCLDIR = $(WINPKGSDIR)/include
WINPKGLIBDIR = $(WINPKGSDIR)/lib

CC = x86_64-w64-mingw32-gcc
COPTS = -O3 -g0 -Wall -I $(INCLDIR) -I $(WINPKGINCLDIR) -L . -L $(WINPKGLIBDIR)
AR = x86_64-w64-mingw32-ar
RANLIB = x86_64-w64-mingw32-ranlib

all: strings.lib test-strings.exe

strings.obj: $(SRCDIR)/strings.c $(INCLDIR)/libstrings.h
	$(CC) $(COPTS) -o strings.obj -c $(SRCDIR)/strings.c

test-strings.exe: test-strings.obj strings.obj
	$(CC) $(COPTS) -o test-strings.exe test-strings.obj strings.obj -lavl

test-strings.obj: $(SRCDIR)/test-strings.c $(INCLDIR)/libstrings.h
	$(CC) $(COPTS) -o test-strings.obj -c $(SRCDIR)/test-strings.c

libstrings.a: strings.obj
	$(AR) rcs libstrings.a strings.obj

strings.lib: libstrings.a
	@cp libstrings.a strings.lib
	$(RANLIB) strings.lib

clean:
	@rm -f *.a *.obj *.lib *.exe

install: strings.lib
	@cp strings.lib $(WINPKGLIBDIR)
	@cp ../include/libstrings.h $(WINPKGINCLDIR)

uninstall:
	@rm -f $(WINPKGLIBDIR)/strings.lib
	@rm -f $(WINPKGINCLDIR)/libstrings.h

