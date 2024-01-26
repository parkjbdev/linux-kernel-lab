CC := clang
LINUX_PATH := $$HOME/linux
KDIR := $(LINUX_PATH)/build/linux-vm
# KDIR := /lib/modules/$(shell uname -r)/build

kbuild: 
	$(MAKE) CC=$(CC) -C $(KDIR) M=$$PWD modules

copy: 
	cp *.ko ~/share/modules/

.PHONY: gen_compile_commands clean

gen_compile_commands:
	bear --append -- make kbuild

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

