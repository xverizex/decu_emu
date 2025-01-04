LIBS=`pkg-config ncurses --libs --cflags`
all:
	gcc main.c hex_editor.c machine.c -pthread -fsanitize=address -g $(LIBS) -o decu_emu
install:
	mkdir -p /usr/local/share/man/man6
	gzip -c decu_emu.6x > decu_emu.6x.gz
	install decu_emu /usr/local/bin
	install decu_emu.6x.gz /usr/local/share/man/man6/
clean:
	rm -f decu_emu
	rm -f decu_emu.6x.gz
