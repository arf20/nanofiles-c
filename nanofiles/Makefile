BINARY := nanofiles

SRC := $(wildcard *.c)
HDR := $(wildcard *.h)
OBJ := $(patsubst %.c,%.o,$(SRC))

CFLAGS := -g -O0 -Wall -pedantic
LDFLAGS := -lmd

all: $(BINARY)

$(BINARY): $(OBJ)
	$(CC) -o $(BINARY) $(OBJ) $(LDFLAGS)

%.o: %.c $(HDR)
	$(CC) -c $(CFLAGS) $<

.PHONY: clean
clean:
	rm -f $(BINARY) *.o

