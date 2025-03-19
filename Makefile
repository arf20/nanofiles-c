CLIENT_TARGET := nanofiles
DIRECTORY_TARGET := directory

CC := gcc
CFLAGS := -Wall -pedantic -g -O0
LDFLAGS := 

.PHONY: $(CLIENT_TARGET)/$(CLIENT_TARGET) $(DIRECTORY_TARGET)/$(DIRECTORY_TARGET) clean doc


$(CLIENT_TARGET)/$(CLIENT_TARGET):
	make -C $(CLIENT_TARGET)/

$(DIRECTORY_TARGET)/$(DIRECTORY_TARGET):
	make -C $(DIRECTORY_TARGET)/

clean:
	make -C $(CLIENT_TARGET)/ clean
	make -C $(DIRECTORY_TARGET)/ clean

doc: doc/PROTOCOL.html doc/PROTOCOL.pdf

doc/PROTOCOL.html: doc/PROTOCOL.md
	pandoc $? -o $@
       
doc/PROTOCOL.pdf: doc/PROTOCOL.md
	pandoc $? -o $@

