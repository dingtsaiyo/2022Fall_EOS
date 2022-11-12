obj-m := mydev.o

x86_KDIR = /lib/modules/$(shell uname -r)/build

RPI_KDIR = ../../linux-5-15-74

all: RPI_build
# or RPI_build ,you can specify only one build at a time

clean: RPI_clean

x86_build: mydev.c
	sudo make -C $(x86_KDIR) M=$(shell pwd) modules

x86_clean:
	sudo make -C $(x86_KDIR) M=$(shell pwd) clean

RPI_build: mydev.c
	sudo make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C $(RPI_KDIR) M=$(shell pwd) modules

RPI_clean:
	sudo make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C $(RPI_KDIR) M=$(shell pwd) clean

.PHONY: all clean x86_build x86_clean RPI_build RPI_clean
