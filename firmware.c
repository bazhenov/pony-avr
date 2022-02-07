#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <util/delay.h>

#include "pony.h"

void test(void);

void __attribute__((noinline)) toggle(uint8_t value) {
  PORTB = value;
}

void task_yield(void);

void task_create(void (*callable)(void), uint8_t stack_size) {
  task_info *task = NULL;
  for (uint8_t i = 0; i < MAX_TASKS; i++) {
    if (!(tasks[i].flags & SLOT_OCCUPIED)) {
      task = &tasks[i];
      break;
    }
  }

  if (!task) {
    return;
  }

  // Getting address of a stack
  void *task_SP = stack_allocator;
  stack_allocator -= stack_size;

  task->sp = task_SP;
  task->flags |= SLOT_OCCUPIED;
  task->f = callable;
}

uint8_t find_next_task() {
  for (uint8_t i = 0; i < MAX_TASKS; i++) {
    uint8_t task_id = (uint8_t)(current_task_idx + 1 + i) % MAX_TASKS;
    if (tasks[task_id].flags & SLOT_OCCUPIED) {
      return task_id;
    }
  }
  return NO_TASK;
}

void scheduller_halt(void) {
  for(;;);
}

void __attribute__((always_inline)) save_cpu_state(void) {
  asm("push r16"::);
  asm("push r17"::);
}

void __attribute__((always_inline)) restore_cpu_state(void) {
  asm("pop r17"::);
  asm("pop r16"::);
}

void __attribute__((noinline)) task_yield(void) {
  start:
  // context switch to next task
  uint8_t next_task_id = find_next_task();
  if (next_task_id == NO_TASK) {
    scheduller_halt();

  } else if (next_task_id != current_task_idx) {
    if (current_task_idx != NO_TASK) {
      // saving current task state
      save_cpu_state();
      tasks[current_task_idx].sp = (void *)((SPH << 8) | SPL); // NOLINT
    }

    // restoring next task state
    task_info *task = &tasks[next_task_id];
    current_task_idx = next_task_id;

    uintptr_t stack_pointer = (uintptr_t)task->sp; // NOLINT
    SPH = stack_pointer >> 8;
    SPL = stack_pointer & 0xFF;

    if (task->flags & TASK_INITIALIZED) {
      restore_cpu_state();
    } else {
      task->flags |= TASK_INITIALIZED;
      task->f();
      // current_task_idx should be used here. Because the task we are return from
      // may be not the same task we was delegating to
      tasks[current_task_idx].flags = 0;
      goto start;
    }
  }
}

void task1(void) {
  for (uint8_t i = 3; i > 0; i--) {
    toggle(1);
    task_yield();
  }
}
void task2(void) {
  for (;;) {
    toggle(2);
    task_yield();
  }
}

void task3(void) {
  for (uint8_t i = 2; i > 0; i--) {
    toggle(3);
    task_yield();
  }
}

void task4(void) {
  toggle(4);
}

int main(void) {
  DDRB = 0xFF;
  test();
  
  task_create(&task1, 100);
  task_create(&task2, 100);
  task_create(&task3, 100);
  task_create(&task4, 100);

  task_yield();
}
