CC := clang-17
KDIR := $$HOME/linux-build/linux-vm

all: 
	bear --append -- make kbuild copy

kbuild: 
	$(MAKE) CC=$(CC) -C $(KDIR) M=$$PWD modules

copy: 
	cp *.ko ~/share/kernel/modules/

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

