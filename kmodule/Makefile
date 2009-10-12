ifneq ($(KERNELRELEASE),)
obj-m:= sht1x.o
    
else
KERNEL_PATH:= /lib/modules/$(shell uname -r)/build
PWD:= $(shell pwd)
RM:=rm -f

default:
	$(MAKE) -C $(KERNEL_PATH) SUBDIRS=$(PWD) modules
modules_install:
	$(MAKE) -C $(KERNEL_PATH) SUBDIRS=$(PWD) modules_install

clean:
	$(RM) sht1x.o sht1x.ko sht1x.mod.c sht1x.mod.o Module.* modules.order
endif
