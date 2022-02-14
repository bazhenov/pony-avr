# Pony AVR

This is simple cooperative scheduler for AVR microcontrollers. At the moment is tested under ATMega328p (Arduino) only.
But should be abe to work with minimal hacking under any AVR microcontroller.

## How to get started

Examples can be found in `examples/` directory. But basically you can create several task which will be run
cooperatively:

```c
#include "pony.h"

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
```