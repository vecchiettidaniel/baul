#Comandos a utilizar
RM=rm -f
CC=gcc

#Lista de objetos, en este caso uno solo
OBJECTS=main.o

#Archivo de salida
OUTPUT=fork

#Flags para el compilador/linker
CFLAGS= -Wall
LDFLAGS=

#Target default
all: $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	$(CC) -o $@ $< $(LDFLAGS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

#PHONY target, es decir, un target que no es especificamente un archivo, en este caso "clean"
.PHONY: clean
clean:
	$(RM) $(OBJECTS) $(OUTPUT)
