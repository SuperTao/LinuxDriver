ifneq ($(KERNELRELEASE),)

obj-m := test.o

else

KERNELDIR :=/lib/modules/`uname -r`/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
clean:
	rm -f *.ko *.o *.mod.o *.mod.c *.order *.symvers

endif
