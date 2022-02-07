#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <util/delay.h>

#include "pony.h"

void test(void);

void __attribute__((noinline)) toggle(bool on) {
  if (on) {
    PORTB = 0xFF;
  } else {
    PORTB = 0;
  }
}

void task_yield(void);

void __attribute__((noinline)) task_finished(void) {
  tasks[current_task_idx].flags &= ~TASK_ACTIVE;
  task_yield();
}

void __attribute__((noinline))
task_create(void (*callable)(void), uint8_t stack_size) {
  task_info *task = NULL;
  for (uint8_t i = 0; i < MAX_TASKS; i++) {
    if (!(tasks[i].flags & TASK_ACTIVE)) {
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

  // Putting 2 return addresses:
  // task_finished callback
  // task callable address
  task_SP -= sizeof(uintptr_t) - 1;
  *(uintptr_t *)task_SP = SWAP_ORDER((uintptr_t)task_finished); // NOLINT
  task_SP -= 2;
  *(uintptr_t *)task_SP = SWAP_ORDER((uintptr_t)callable); // NOLINT
  task_SP -= 1;

  task->sp = task_SP;
  task->flags |= TASK_ACTIVE;
  task->f = callable;
}

uint8_t __attribute__((always_inline)) find_next_task() {
  for (uint8_t i = 0; i < MAX_TASKS; i++) {
    uint8_t task_id = (current_task_idx + 1 + i) % MAX_TASKS;
    if (tasks[task_id].flags & TASK_ACTIVE) {
      return task_id;
    }
  }
  return NO_TASK;
}

void __attribute__((always_inline)) scheduller_halt(void) {
  for(;;);
}

void __attribute__((always_inline)) save_cpu_state(void) {
  // asm("push r16"::);
  // asm("push r17"::);
}

void __attribute__((always_inline)) restore_cpu_state(void) {
  // asm("pop r17"::);
  // asm("pop r16"::);
}

void __attribute__((noinline)) task_yield(void) {
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
    current_task_idx = next_task_id;    
    task_info *task = &tasks[current_task_idx];
    uintptr_t stack_pointer = (uintptr_t)task->sp; // NOLINT
    SPH = stack_pointer >> 8;
    SPL = stack_pointer & 0xFF;

    if (task->flags & TASK_INITIALIZED) {
      restore_cpu_state();
    } else {
      task->flags |= TASK_INITIALIZED;
      //task->f();
    }
  }
}

void run_scheduler(void) {
  // delegating control to the first task
  task_yield();
}

void task1(void) {
  for (int i = 3; i > 0; i--) {
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
  DDRB = 0xFF;
  test();
  task_create(&task1, 100);
  task_create(&task2, 100);
  run_scheduler();
}
