obj-m = mi_driver.o
#KERNEL_DIR = ../../kernel_beagle
$(obj-m):
	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -C $(KERNEL_BEAGLE) M=$(CURDIR) modules
clean:
	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -C $(KERNEL_BEAGLE) M=$(CURDIR) clean
