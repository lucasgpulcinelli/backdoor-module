KDIR ?= $(PWD)/linux-*/

.PHONY: all clean module

all: module

clean:
	$(MAKE) -C $(KDIR) M=$(PWD)/src clean

module:
	$(MAKE) CC=$(CC) -C $(KDIR) M=$(PWD)/src modules

