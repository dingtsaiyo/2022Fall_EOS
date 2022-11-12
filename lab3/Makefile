obj-m += led_driver.o

#KDIR = /lib/modules/$(shell uname -r)/build
KDIR = ../../linux-5-15-74

all:
	make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C $(KDIR) M=$(shell pwd) modules
clean:
	make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C $(KDIR) M=$(shell pwd) clean
