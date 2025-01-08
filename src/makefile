CC=gcc
CFLAGS=
LDFLAGS=-lncurses -Wl,--push-state,--as-needed -ltinfo -Wl,--pop-state
SOURCES=main.c hex_editor.c machine.c debug.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=decu_emu
MAN=decu_emu.6x
ZIPPED_MAN=decu_emu.6x.gz
MAN_PAGES_DIR=/usr/local/share/man/man6
BIN_DIR=/usr/local/bin
COUNT=4

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

.c.o:
	@echo [CC] $<
	@cc -c $< $(CFLAGS) -o $@

install:
	mkdir -p $(MAN_PAGES_DIR)
	gzip -c $(MAN) > $(ZIPPED_MAN)
	nstall $(EXECUTABLE) $(BIN_DIR)
	install $(ZIPPED_MAN) $(MAN_PAGES_DIR)
clean:
	rm -f *.o
	rm -f $(EXECUTABLE)
	rm -f $(ZIPPED_MAN)
