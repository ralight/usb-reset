.PHONY: all memtest clean install

VERSION=0.2

CC=gcc
NAME=usb-reset
CFLAGS=-Wall -O2 -ggdb -I/usr/include/libusb-1.0 -DVERSION="\"${VERSION}\""
LDFLAGS=-lusb-1.0
INSTALL=install
prefix=/usr/local

all : $(NAME)

$(NAME).o : $(NAME).c
	$(CC) -c $(CFLAGS) -o $@ $<

$(NAME) : $(NAME).o
	$(CC) -o $@ $^ ${LDFLAGS}

memtest : $(NAME)
	valgrind -v --show-reachable=yes --leak-check=full ./$^

clean :
	-rm -f $(NAME) *.o

install : all
	$(INSTALL) -d ${DESTDIR}$(prefix)/bin
	$(INSTALL) $(NAME) $(DESTDIR)$(prefix)/bin
