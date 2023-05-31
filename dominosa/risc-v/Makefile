.PHONY: all clean

all:
	riscv64-unknown-elf-gcc -T linker_script.lds -nostdlib dominosa.s

clean:
	rm -f a.out
