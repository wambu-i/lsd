CC:=gcc

OUTPUT:=disconnect
OBJECTS:=main.o
DEPS:=errors.h disconnect.h

CFLAGS:=-I/usr/include/libnl3/
CXXFLAGS = $(CFLAGS) -Os -Wall --std=gnu99 -g3 -Wmissing-declarations

LSDFLAGS = $(LDFLAGS) -lnl-3 -lnl-genl-3
DFLAGS:=$(CXXFLAGS) -DDEBUG -g -O0

COMPILE:=$(CC) $(CXXFLAGS)

all: $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	$(COMPILE) -o $@ $^ $(LSDFLAGS)

%.o: %.c $(DEPS)
	$(COMPILE) -c $< -o $@

clean:
	rm -rf *.o disconnect