CFLAGS += -I. -fPIC
CFLAGS += -I. -fPIC
LDFLAGS += -L../../../maxonmotors/epos-serial/can

PREFIX = /usr/local

libepos.so: epos.c
	$(CC) $(CFLAGS) $(LDFLAGS) -Wall -shared -o libepos.so epos.c -lcan

install: libepos.so
	install -c -m 755 libepos.so $(PREFIX)/lib
	install -c -m 644 epos.h $(PREFIX)/include/can

clean:
	rm -f *.o
	rm -f *.a
	rm -f *.so
