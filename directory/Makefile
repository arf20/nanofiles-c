BINARY := directory

SRC := $(wildcard *.c)
HDR := $(wildcard *.h)
OBJ := $(patsubst %.c,%.o,$(SRC))

all: $(BINARY)

$(BINARY): $(OBJ)
	$(CC) -o $(BINARY) $(OBJ) $(LDFLAGS)

%.o: %.c $(HDR)
	$(CC) -c $(CFLAGS) $<

.PHONY: clean
clean:
	rm -f $(BINARY) *.o

