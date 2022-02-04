.PHONY = avr-gdb run
CC = gcc
CFLAGS = -Wall -pedantic -lsimavr

F_CPU = 16000000
MCU = atmega328p

AVR_CC = avr-gcc
AVR_CFLAGS = -DF_CPU=$(F_CPU) -mmcu=$(MCU) -O3 -Wall -Wpedantic

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
		-ex 'rbreak toggle.*'\
		-ex 'break main'\
		-ex 'break task_yield'\
		-ex 'cont'\
		-ex 'next'\
		-ex 'next'\
		-ex 'x /4xb tasks[0].sp + 1'\
		-ex 'x /4xb tasks[1].sp + 1'\
		-ex 'x /4xb $$SP + 1'
