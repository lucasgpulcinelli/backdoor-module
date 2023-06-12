KDIR ?= /usr/src/kernels/$(shell uname -r)

.PHONY: all clean module insert remove

all: module

insert:
	sudo insmod src/backdoor.ko

remove:
	sudo rmmod src/backdoor.ko

clean:
	$(MAKE) -C $(KDIR) M=$(PWD)/src clean

module:
	$(MAKE) CC=$(CC) -C $(KDIR) M=$(PWD)/src modules

