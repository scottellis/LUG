ifneq ($(KERNELRELEASE),)
    obj-m := lug.o
else
    KERNELDIR ?= /lib/modules/$(shell uname -r)/build
    PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules


clean:
	rm -rf *.o *.ko *.mod.* Module.* .lug* .tmp* modules.order *~

endif

