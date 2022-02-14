.PHONY = avr-gdb run flash
CC = gcc
CFLAGS = -Wall -pedantic -lsimavr

F_CPU = 16000000
MCU = atmega328p

AVR_CC = avr-gcc
AVR_CFLAGS = -DF_CPU=$(F_CPU) -mmcu=$(MCU) -O3 -Wall -Wpedantic -I./include

examples/%.o: examples/%.c
	$(AVR_CC) $(AVR_CFLAGS) -g -c $^ -o $@

src/%.o: src/%.c
	$(AVR_CC) $(AVR_CFLAGS) -g -c $^ -o $@

examples/%.elf: examples/%.o src/pony.o
	$(AVR_CC) $(AVR_CFLAGS) $^ -o $@

%.hex: %.elf
	avr-objcopy -j .text -j .data -O ihex $< $@

# Host tasks

run-example-%: examples/%.elf
	./test/main $^

run-example-%-debug: examples/%.elf
	./test/main $^ -g

test/main: test/main.c
	$(CC) $(CFLAGS) -o $@ $<

compile_flags.txt:
	echo "-I/usr/avr/include" >> $@
	echo "-I./include" >> $@
	echo "-D__AVR_ATtiny85__" >> $@
	echo "$(AVR_CFLAGS)" >> $@

gdb-%: examples/%.elf
	avr-gdb $< \
		-ex 'target remote :1234'\
		-ex 'break task_yield'\
		-ex 'break main'\
		-ex 'rbreak ^task[0-9]'\

flash-%: examples/%.hex
	avrdude -P /dev/ttyACM0 -c arduino -p m328p -U flash:w:$<:i
