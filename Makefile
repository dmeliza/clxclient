#  Copyright (C) 2003-2008 Fons Adriaensen <fons@kokkinizita.net>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU Lesser General Public License as published
#  by the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


# Modify as required.
#
PREFIX = /usr/local
SUFFIX := $(shell uname -m | sed -e 's/^unknown/$//' -e 's/^i.86/$//' -e 's/^x86_64/$/64/')
LIBDIR = lib$(SUFFIX)


MAJVERS = 3
MINVERS = 6.1
VERSION = $(MAJVERS).$(MINVERS)
DISTDIR = clxclient-$(VERSION)


CPPFLAGS += -Wall -I. -I/usr/X11R6/include `freetype-config --cflags` -fpic -DVERSION=\"$(VERSION)\" -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS -O2 
LDFLAGS += -L$(PREFIX)/$(LIBDIR) -L/usr/X11R6/$(LIBDIR) `freetype-config --libs`
LDLIBS +=


CLXCLIENT_SO = libclxclient.so
CLXCLIENT_MAJ = $(CLXCLIENT_SO).$(MAJVERS)
CLXCLIENT_MIN = $(CLXCLIENT_MAJ).$(MINVERS)
CLXCLIENT_DEP = -lpthread -lXft -lX11
CLXCLIENT_O = xdisplay.o xresman.o xhandler.o xwindow.o xdraw.o \
	button.o textip.o enumip.o menuwin.o scale.o slider.o scroll.o mclist.o meter.o
CLXCLIENT_H = clxclient.h


$(CLXCLIENT_MIN): $(CLXCLIENT_O)
	g++ -shared $(LDFLAGS) -Wl,-soname,$(CLXCLIENT_MAJ) -o $(CLXCLIENT_MIN) $(CLXCLIENT_O) $(CLXCLIENT_DEP)


install:	$(CLXCLIENT_MIN)
	/usr/bin/install -d $(PREFIX)/$(LIBDIR)
	/usr/bin/install -m 644 $(CLXCLIENT_H) $(PREFIX)/include
	/usr/bin/install -m 755 $(CLXCLIENT_MIN) $(PREFIX)/$(LIBDIR)
	/sbin/ldconfig -n $(PREFIX)/$(LIBDIR)
	ln -sf $(CLXCLIENT_MIN) $(PREFIX)/$(LIBDIR)/$(CLXCLIENT_SO)


clean:
	/bin/rm -f *~ *.o *.a *.d *.so.*


tarball:
	cd ..; \
	/bin/rm -f -r $(DISTDIR)*; \
	svn export clxclient $(DISTDIR); \
	tar cvf $(DISTDIR).tar $(DISTDIR); \
	bzip2 $(DISTDIR).tar; \
	/bin/rm -f -r $(DISTDIR);

