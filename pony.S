#include <avr/io.h>

  .section .text
  .global test

test:
  ldi r16, 0x03
  ret
