obj-m = mi_driver.o
#SRC=$(wildcard *.c)
SRC=mi_driver.c

#KERNEL_DIR = ../../kernel_beagle
$(obj-m): $(SRC)
	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -C $(KERNEL_BEAGLE) M=$(CURDIR) modules
clean:
	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -C $(KERNEL_BEAGLE) M=$(CURDIR) clean
