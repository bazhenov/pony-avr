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

#define MAX_TASKS 3
uint8_t tasks_left = MAX_TASKS;
void *stacks[MAX_TASKS];

void __attribute__((noinline))
task_create(void (*callable)(void), uint16_t stack_size) {
  // Getting address of a stack
  uint16_t *stack = (uint16_t *)((SPH << 8) | SPL); // NOLINT
  // Moving to a previous stack position
  stack++;
  // Replacing return address on a stack
  *stack = (uint16_t *)callable; // NOLINT
}

void task1(void) {}

int main(void) {
  task_create(&task1, 100);
  DDRB = 0xFF;
  for (;;) {
    toggle(true);
    toggle(false);
  }
}
