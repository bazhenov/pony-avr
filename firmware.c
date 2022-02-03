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

#define MAX_TASKS 2
uint8_t tasks_left = MAX_TASKS;
void *stacks[MAX_TASKS];
void *stack_allocator = (void *)RAMEND;
uint8_t current_task_idx = 0;

void __attribute__((noinline))
task_create(void (*callable)(void), uint8_t stack_size) {
  if (tasks_left == 0) {
    return;
  }
  // Getting address of a stack
  void *task_stack_pointer = stack_allocator;
  stack_allocator -= stack_size;
  uint8_t task_idx = --tasks_left;

  task_stack_pointer -= sizeof(uintptr_t) - 1;
  uintptr_t ret_address = (uintptr_t)callable; // NOLINT
  ret_address = (ret_address >> 8) | (ret_address << 8);
  *(uintptr_t *)task_stack_pointer = ret_address;
  task_stack_pointer--;
  stacks[task_idx] = task_stack_pointer;
}

void __attribute__((noinline)) task_yield(void) {
  // context switch to next task
  if (current_task_idx >= MAX_TASKS) {
    current_task_idx = 0;
  }
  uint8_t task_idx = current_task_idx++;
  uintptr_t stack_pointer = (uintptr_t)stacks[task_idx]; // NOLINT
  SPH = stack_pointer >> 8;
  SPL = stack_pointer & 0xFF;
}

void task_sleep(uint16_t delay) {
  // context switch to next task
}

void __attribute__((noinline)) run_scheduler(void) {
  // delegating control to the first task
  task_yield();
}

void task1(void) {
  for (;;) {
    toggle(1);
    task_yield();
  }
}

void task2(void) {
  for (;;) {
    toggle(0);
    task_yield();
  }
}

int main(void) {
  task_create(&task1, 100);
  task_create(&task2, 100);
  run_scheduler();
}
