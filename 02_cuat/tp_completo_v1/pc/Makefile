COMPILER = gcc
GCCFLAGS = -Wall
# SRC es la lista de todos los .c que tengo o sea:
# 	src/file1.c  src/file2.c etc
# OBJ es la misma lista, pero remplazando src y .c por bin y .o
# 	bin/file1.o  bin/file2.o etc
SRC=$(wildcard src/*.c)
OBJ=$(patsubst src/%,bin/%,$(SRC:.c=))
INC=$(wildcard inc/*.h)

# regla general para armar todos los .o
# de cada src/*.c arma un bin/*.o,
# el $(INC) es para que se de cuenta si cambio algun .h
bin/%: src/%.c $(INC)
	$(COMPILER) $< -o $@ $(GCCFLAGS)


all: $(OBJ)

clean:
	rm -f $(OBJ)
