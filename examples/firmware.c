#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <util/delay.h>

#include "pony.h"

void __attribute__((noinline)) toggle(uint8_t value) { PORTB = value; }

void task1(void) {
  for (;;) {
    PORTB ^= 1 << PIN5;
    delay_ms(33);
  }
}

void task2(void) {
  for (;;) {
    PORTB ^= 1 << PIN4;
    delay_ms(500);
  }
}

int main(void) {
  DDRB = 0xFF;

  init_timers();

  task_create(&task1, 100);
  task_create(&task2, 100);

  task_yield();
}
