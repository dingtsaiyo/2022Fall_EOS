obj-m += driver.o

KDIR = ../../linux-5-15-74

#CC=/usr/bin/gcc-5
#CXX=/usr/bin/g++-5

all:
	make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C $(KDIR) M=$(shell pwd) modules
clean:
	make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C $(KDIR) M=$(shell pwd) clean
