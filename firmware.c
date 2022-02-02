#include <avr/io.h>
#include <avr/sleep.h>
#include <stdbool.h>
#include <util/delay.h>

void __attribute__((noinline)) toggle(bool on) {
  if (on) {
    PORTB = 0xFF;
  } else {
    PORTB = 0;
  }
}

int main(void) {
  DDRB = 0xFF;
  for (;;) {
    toggle(true);
    toggle(false);
  }
}
