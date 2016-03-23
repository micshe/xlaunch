DESTDIR=
PREFIX=/usr/local
CFLAGS=-Wall -I. -I/usr/include -I/usr/local/include
LDFLAGS=-lX11 -L. -L/usr/lib -L/usr/local/lib

help:
	@echo
	@echo "xlaunch Makefile help"
	@echo "*********************"
	@echo
	@echo "to build xlaunch, type"
	@echo " $$ make xlaunch"
	@echo
	@echo "to install xlaunch into the default location, type"
	@echo " $$ make install"
	@echo 
	@echo "to uninstall xlaunch from the default location, type"
	@echo " $$ make uninstall"
	@echo
	@echo "to clean the xlaunch source directory, type"
	@echo " $$ make clean"
	@echo
	@echo "to show the default configuration options, type"
	@echo " $$ make options"
	@echo
	@echo "CC is the c compiler that will be used to compile xlaunch."
	@echo "to use a different c compiler type"
	@echo " $$ make xlaunch CC=/path/to/differnt/c/compiler"
	@echo 
	@echo "CFLAGS and LDFLAGS are used to pass the location of the"
	@echo "libX11 library and the libX11 header files to the c compiler"
	@echo "override these when building if this library and its header"
	@echo "files are install in a different place on your machine"
	@echo
	@echo "by default, xlaunch installs to and uninstalls from"
	@echo "/usr/local/bin. to install to or uninstall from a different"
	@echo "location, override the PREFIX option, like so:"
	@echo 
	@echo "to install xlaunch into your home directory, type"
	@echo " $$ make install PREFIX=$$HOME"
	@echo
	@echo "to uninstall xlaunch from your /tmp directory, type"
	@echo " $$ make uninstall PREFIX=/tmp"
	@echo

all: options xlaunch

options:
	@echo "CC="$(CC)
	@echo "LD="$(LD)
	@echo "CFLAGS="$(CFLAGS)
	@echo "LDFLAGS="$(LDFLAGS)
	@echo "DESTDIR="$(DESTDIR)
	@echo "PREFIX="$(PREFIX)

clean:
	@rm -f xlaunch.o xlaunch || true

uninstall:
	@echo uninstalling xlaunch from $(DESTDIR)$(PREFIX)
	@rm -f $(DESTDIR)$(PREFIX)/bin/xlaunch || true

install: xlaunch 
	@echo installing xlaunch to $(DESTDIR)$(PREFIX)	
	@mkdir -p $(DESTDIR)$(PREFIX)/bin
	@cp xlaunch $(DESTDIR)$(PREFIX)/bin/xlaunch
	@chmod 755 $(DESTDIR)$(PREFIX)/bin/xlaunch

xlaunch.o: xlaunch.c
	$(CC) -c xlaunch.c $(CFLAGS)
	
xlaunch: xlaunch.o
	$(CC) -o xlaunch xlaunch.o $(LDFLAGS)

.PHONY: install uninstall clean options all help

