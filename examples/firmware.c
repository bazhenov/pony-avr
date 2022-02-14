#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <util/delay.h>

// #define MAX_TASKS 2
#include "pony.h"

void __attribute__((noinline)) toggle(uint8_t value) { PORTB = value; }

void task1(void) {
  for (;;) {
    PORTB ^= 1 << PIN5;
    delay_ticks(20);
  }
}

void task2(void) {
  for (;;) {
    PORTB ^= 1 << PIN4;
    delay_ticks(50);
  }
}

int main(void) {
  DDRB = 0xFF;

  init_timers();

  task_create(&task1, 100);
  task_create(&task2, 100);

  task_yield();
}