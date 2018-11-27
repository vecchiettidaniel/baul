DRIVERPATH=$(wildcard ./driver/*.ko)
BBB=$(wildcard ./bbb/bin/*)

export KERNEL_BEAGLE = $(realpath ../kernel_beagle)

# Cada carpeta tendra sus makefiles y desde aca los llamo y ya ta
# TODOSlosMAKES tiene todos los makefiles que hay en subdirectorios
# COMPILEPATHS son solamemnte lso directorios
# BORRARPATHS esto es igual que el otro pero no encontre otra forma
# de llamatr a los make cleans desde aca arriba... ver el clean
TODOSlosMAKES=$(wildcard */Makefile)
COMPILEPATHS=$(patsubst %Makefile,%,$(TODOSlosMAKES))
BORRARPATHS=$(addsuffix pasillo4,$(COMPILEPATHS))
.PHONY: $(COMPILEPATHS)
.PHONY: $(BORRARPATHS)

all: $(COMPILEPATHS)

$(COMPILEPATHS):
	$(MAKE) -C $@

copy:
	scp $(DRIVERPATH) $(BBB) debian@192.168.6.2:/home/debian/td3

run:
	./pc/bin/cliente 192.168.6.2 4545 10 3

clean: $(BORRARPATHS)

$(BORRARPATHS):
	$(MAKE) -C $(patsubst %pasillo4,%,$@) clean
