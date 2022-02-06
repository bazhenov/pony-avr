#include <avr/io.h>
#include <avr/sleep.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <util/delay.h>

void test(void);

void __attribute__((noinline)) toggle(bool on) {
  if (on) {
    PORTB = 0xFF;
  } else {
    PORTB = 0;
  }
}

typedef struct {
  // stack pointer of a task
  void *sp;
  uint8_t flags;
} task_info;

#define TASK_ACTIVE 1
#define MAX_TASKS 2
#define NO_TASK 0xFF

task_info tasks[MAX_TASKS];
void *stack_allocator = (void *)RAMEND - 100;
uint8_t current_task_idx = NO_TASK;

// Swaps the byte order in 2 byte integer
#define SWAP_ORDER(x) ((x >> 8) | (x << 8))

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
  *(uintptr_t *)task_SP = SWAP_ORDER((uintptr_t)task_finished);
  task_SP -= 2;
  *(uintptr_t *)task_SP = SWAP_ORDER((uintptr_t)callable);
  task_SP -= 1;

  task->sp = task_SP;
  task->flags |= TASK_ACTIVE;
}

uint8_t find_next_task() {
  for (uint8_t i = 0; i < MAX_TASKS; i++) {
    uint8_t task_id = (current_task_idx + 1 + i) % MAX_TASKS;
    if (tasks[task_id].flags & TASK_ACTIVE) {
      return task_id;
    }
  }
  return NO_TASK;
}

void __attribute__((noinline)) task_yield(void) {
  // context switch to next task
  bool save_sp = true;
  if (current_task_idx == NO_TASK) {
    // First run of task_yield - no need to save context executuion
    current_task_idx = MAX_TASKS;
    save_sp = false;
  }
  if (save_sp) {
    // saving task state
    uintptr_t stack_pointer = (uintptr_t)((SPH << 8) | SPL);
    tasks[current_task_idx].sp = (void *)stack_pointer;
  }
  uint8_t next_task_id = find_next_task();
  if (next_task_id == NO_TASK) {
    // HALT
    for (;;)
      ;
  }
  current_task_idx = next_task_id;
  uintptr_t stack_pointer = (uintptr_t)tasks[current_task_idx].sp; // NOLINT
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
