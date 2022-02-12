.PHONY = avr-gdb run flash
CC = gcc
CFLAGS = -Wall -pedantic -lsimavr

F_CPU = 16000000
MCU = atmega328p

AVR_CC = avr-gcc
AVR_CFLAGS = -DF_CPU=$(F_CPU) -mmcu=$(MCU) -O3 -Wall -Wpedantic

run: main firmware.elf
	./main firmware.elf

main: main.c
	$(CC) $(CFLAGS) -o $@ $<

compile_flags.txt:
	echo "-I/usr/avr/include" >> $@
	echo "-D__AVR_ATtiny85__" >> $@
	echo "$(AVR_CFLAGS)" >> $@

firmware.elf: firmware.c pony.S
	$(AVR_CC) $(AVR_CFLAGS) -g -o $@ $^

%.hex: %.elf
	avr-objcopy -j .text -j .data -O ihex $< $@

run-debug: main firmware.elf
	./main firmware.elf -g

avr-gdb: firmware.elf
	avr-gdb $< \
		-ex 'target remote :1234'\
		-ex 'break task_yield'\
		-ex 'rbreak ^task[0-9]'\
		-ex 'cont'

flash: firmware.hex
	avrdude -P /dev/ttyACM0 -c arduino -p m328p -U flash:w:$<:i
