#include <avr/io.h>

  .section .text
  .global test

test:
  ldi r16, 0x0F
  out _SFR_IO_ADDR(PORTB), r16

  lds r17, current_task_idx
  inc r17
  ret