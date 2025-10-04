all: asm emu

asm: asm.c
	gcc -ansi -std=c99 -o asm asm.c

emu: emu.c
	gcc -ansi -std=c99 -o emu emu.c

run: asm emu
	./asm test.asm

clean:
	rm asm emu
