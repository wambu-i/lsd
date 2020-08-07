CC:=gcc

OUTPUT:=disconnect
OBJECTS:=main.o
DEPS:=errors.h disconnect.h

CFLAGS:=-Wall -Wextra
INCLUDE:=-I/usr/include/libnl3/
DFLAGS:=$(CFLAGS) -DDEBUG -g -O0

COMPILE:=$(CC) $(CFLAGS) $(INCLUDE)

all: $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	$(COMPILE) -o $@ $^

%.o: %.c $(DEPS)
	$(COMPILE) -c $< -o $@

clean:
	rm -rf *.o disconnect