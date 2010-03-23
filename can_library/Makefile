CFLAGS += -I../can -I../epos
LDFLAGS += -L../can -L../epos

PREFIX = /usr/local

libcan-serial.so: can_serial.c
	$(CC) $(CFLAGS) $(LDFLAGS) -Wall -shared -o libcan-serial.so \
	can_serial.c
	ln -sf "$(PWD)"/libcan-serial.so ../can/libcan.so

install: libcan-serial.so
	install -c -m 755 libcan-serial.so $(PREFIX)/lib/libcan.so
	install -c -m 644 can_serial.h $(PREFIX)/include/can

clean:
	rm -f *.o
	rm -f *.a
	rm -f *.so
