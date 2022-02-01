#include <avr/io.h>
#include <avr/sleep.h>
#include <util/delay.h>

int main(void) {
  DDRB = 0xFF;
  for (;;) {
    PORTB = 0xF0;
    PORTB = 0x0F;
  }
}
