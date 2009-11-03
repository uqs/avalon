CFLAGS += -I../errors -I../kinematics -I../drivers/vanilla/epos -fPIC
LDFLAGS += -L../kinematics -L../drivers/vanilla/epos

PREFIX = /usr/local

libera-controller.so: motors.c timer.c thread.c sensors.c controller.c era.c
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -o libera-controller.so \
	motors.c timer.c thread.c sensors.c controller.c era.c \
	-lera-kinematics -lepos -lpthread

install: libera-controller.so
	install -c -m 644 libera-controller.so $(PREFIX)/lib

clean:
	rm -f *.o *.a *.so
