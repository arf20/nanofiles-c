NF_TARGET := nanofiles
DIR_TARGET := directory

NF_SRC := $(wildcard $(NF_TARGET)/*.c)
NF_OBJ := $(patsubst $(NF_TARGET)/%.c,build/$(NF_TARGET)_obj/%.o,$(NF_SRC))

DIR_SRC := $(wildcard $(DIR_TARGET)/*.c)
DIR_OBJ := $(patsubst $(DIR_TARGET)/%.c,build/$(DIR_TARGET)_obj/%.o,$(DIR_SRC))

COM_SRC := $(wildcard common/*.c)
COM_OBJ := $(patsubst common/%.c,build/common_obj/%.o,$(COM_SRC))

CC := gcc
LD := gcc

CFLAGS := -g -O0 -Wall -pedantic
LDFLAGS := -lmd

all: build/ build/$(NF_TARGET) build/$(DIR_TARGET)

build/:
	mkdir build build/common_obj build/$(NF_TARGET)_obj build/$(DIR_TARGET)_obj

build/$(NF_TARGET): $(NF_OBJ) $(COM_OBJ)
	$(LD) -o build/$(NF_TARGET) $(NF_OBJ) $(COM_OBJ) $(LDFLAGS)

build/$(DIR_TARGET): $(DIR_OBJ) $(COM_OBJ)
	$(LD) -o build/$(DIR_TARGET) $(DIR_OBJ) $(COM_OBJ) $(LDFLAGS)

build/common_obj/%.o: common/%.c
	$(CC) -c $(CFLAGS) -o $@ $<

build/$(NF_TARGET)_obj/%.o: $(NF_TARGET)/%.c
	$(CC) -c $(CFLAGS) -o $@ $<

build/$(DIR_TARGET)_obj/%.o: $(DIR_TARGET)/%.c
	$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -f build/$(NF_TARGET) build/$(DIR_TARGET) build/*.o

doc: doc/PROTOCOL.html doc/PROTOCOL.pdf

doc/PROTOCOL.html: doc/PROTOCOL.md
	pandoc $? -o $@
       
doc/PROTOCOL.pdf: doc/PROTOCOL.md
	pandoc $? -o $@

