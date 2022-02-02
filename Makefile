.PHONY = avr-gdb run
CC = gcc
CFLAGS = -Wall -pedantic -lsimavr

F_CPU = 16000000
MCU = attiny85

AVR_CC = avr-gcc
AVR_CFLAGS = -DF_CPU=$(F_CPU) -mmcu=$(MCU) -O2

main: main.c
	$(CC) $(CFLAGS) -o $@ $<

compile_flags.txt:
	echo "-I/usr/avr/include" >> $@
	echo "-D__AVR_ATtiny85__" >> $@
	echo "$(AVR_CFLAGS)" >> $@

firmware.elf: firmware.c
	$(AVR_CC) $(AVR_CFLAGS) -g -o $@ $<

run: main firmware.elf
	./main firmware.elf

avr-gdb: firmware.elf
	avr-gdb $< \
		-ex 'target remote :1234'\
		-ex 'break main'\
		-ex 'cont'
