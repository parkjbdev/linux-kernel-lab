CC := clang-17
KDIR := $$HOME/linux-build/linux-vm

kbuild: 
	$(MAKE) CC=$(CC) -C $(KDIR) M=$$PWD modules

copy: 
	cp *.ko ~/share/kernel/modules/

.PHONY: gen_compile_commands clean

gen_compile_commands:
	bear --append -- make kbuild

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

