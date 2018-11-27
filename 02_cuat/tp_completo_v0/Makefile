COMPILER = gcc
CROSCOMPILER = arm-linux-gnueabihf-gcc
GCCFLAGS = -Wall
driverpath = ./src/driver
export KERNEL_BEAGLE = $(realpath ../kernel_beagle)

# SRC es la lista de todos los .c que tengo o sea:
# 	src/file1.c  src/file2.c etc
# OBJ es la misma lista, pero remplazando src y .c por bin y .o
# 	bin/file1.o  bin/file2.o etc
#OBJ=$(patsubst src%,bin%,$(SRC:.c=))
SRCBBB=$(wildcard src/bbb/*.c)
SRCPC=$(wildcard src/pc/*.c)
OBJBBB=$(patsubst src/bbb%,bin%,$(SRCBBB:.c=))
OBJPC=$(patsubst src/pc%,bin%,$(SRCPC:.c=))
OBJ=$(OBJPC) $(OBJBBB)
INC=$(wildcard inc/*.h)

# regla general para armar todos los .o
# de cada src/*.c arma un bin/*.o,
# el $(INC) es para que se de cuenta si cambio algun .h
bin/%: src/bbb/%.c $(INC)
	$(CROSCOMPILER) $< -o $@ $(GCCFLAGS)

bin/%: src/pc/%.c $(INC)
	$(COMPILER) $< -o $@ $(GCCFLAGS)


all: $(OBJ) driver

driver: $(driverpath)/mi_driver.o

$(driverpath)/mi_driver.o:
	make -C $(driverpath)/

copy:
	scp $(driverpath)/mi_driver.ko $(OBJBBB) debian@192.168.6.2:/home/debian/td3

clean:
	rm -f $(OBJ)
	make -C $(driverpath) clean
