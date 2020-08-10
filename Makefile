CC:=gcc

OUTPUT:=disconnect
OBJECTS:=main.o
DEPS:=errors.h disconnect.h

CXXFLAGS = $(CFLAGS) -Os -Wall --std=gnu99 -g3 -Wmissing-declarations

LDFLAGS = -lnl -lnl-genl
DFLAGS:=$(CFLAGS) -DDEBUG -g -O0

COMPILE:=$(CC) $(CXXFLAGS)

all: $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	$(COMPILE) -o $@ $^ $(LIBRARIES)

%.o: %.c $(DEPS)
	$(COMPILE) -c $< -o $@

clean:
	rm -rf *.o disconnect