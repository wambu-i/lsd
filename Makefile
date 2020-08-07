CC:=gcc

OUTPUT:=disconnect
OBJECTS:=main.o
DEPS:=errors.h disconnect.h

CFLAGS:=-Wall -Wextra
INCLUDE:=-I/usr/include/libnl3/

LIBRARIES:=-lnl-3 -lnl-genl-3
DFLAGS:=$(CFLAGS) -DDEBUG -g -O0

COMPILE:=$(CC) $(CFLAGS) $(INCLUDE)

all: $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	$(COMPILE) -o $@ $^ $(LIBRARIES)

%.o: %.c $(DEPS)
	$(COMPILE) -c $< -o $@ $(LIBRARIES)

clean:
	rm -rf *.o disconnect