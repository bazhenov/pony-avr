#include "pony.h"

#include <avr/interrupt.h>
#include <stddef.h>

// Defines how many ticks in one ms
//
// Should be consistent with the prescaler logic of TIMER0 in `init_timers()`
#define MAX(a, b) (a > b ? a : b)
#define MS_TO_TICKS_RATIO (MAX(1, F_CPU / 1000 / 64 / 256))

void *stack_allocator = (void *)RAMEND;
task_info tasks[MAX_TASKS];
volatile uint8_t current_task_idx = NO_TASK;

bool task_create(void (*callable)(void), uint8_t stack_size) {
  task_info *task = NULL;
  for (uint8_t i = 0; i < MAX_TASKS; i++) {
    if (tasks[i].f == NULL) {
      task = &tasks[i];
      break;
    }
  }

  if (!task) {
    return false;
  }

  // Getting address of a stack
  void *task_SP = stack_allocator;
  stack_allocator -= stack_size;

  task->sp = task_SP;
  task->f = callable;
  return true;
}

uint8_t find_next_task() {
  for (;;) {
    for (uint8_t i = 0; i < MAX_TASKS; i++) {
      uint8_t task_id = (uint8_t)(current_task_idx + 1 + i) % MAX_TASKS;
      task_info *task = &tasks[task_id];
      if (task->f != NULL && task->status != TASK_SLEEP) {
        return task_id;
      }
    }
  }
}

#define SAVE_CPU_STATE  \
  {                     \
    asm("push r2" ::);  \
    asm("push r3" ::);  \
    asm("push r4" ::);  \
    asm("push r5" ::);  \
    asm("push r6" ::);  \
    asm("push r7" ::);  \
    asm("push r8" ::);  \
    asm("push r9" ::);  \
    asm("push r10" ::); \
    asm("push r11" ::); \
    asm("push r12" ::); \
    asm("push r13" ::); \
    asm("push r14" ::); \
    asm("push r15" ::); \
    asm("push r16" ::); \
    asm("push r17" ::); \
    asm("push r28" ::); \
    asm("push r29" ::); \
  }

#define RESTORE_CPU_STATE \
  {                       \
    asm("pop r29" ::);    \
    asm("pop r28" ::);    \
    asm("pop r17" ::);    \
    asm("pop r16" ::);    \
    asm("pop r15" ::);    \
    asm("pop r14" ::);    \
    asm("pop r13" ::);    \
    asm("pop r12" ::);    \
    asm("pop r11" ::);    \
    asm("pop r10" ::);    \
    asm("pop r9" ::);     \
    asm("pop r8" ::);     \
    asm("pop r7" ::);     \
    asm("pop r6" ::);     \
    asm("pop r5" ::);     \
    asm("pop r4" ::);     \
    asm("pop r3" ::);     \
    asm("pop r2" ::);     \
  }

void task_yield(void) {
  // context switch to next task
  uint8_t next_task_id;
start:
  next_task_id = find_next_task();

  if (next_task_id == current_task_idx) {
    return;
  }
  if (current_task_idx != NO_TASK) {
    // saving current task state
    SAVE_CPU_STATE;
    tasks[current_task_idx].sp = (void *)((SPH << 8) | SPL);  // NOLINT
  }

  // restoring next task state
  task_info *task = &tasks[next_task_id];
  current_task_idx = next_task_id;

  uintptr_t stack_pointer = (uintptr_t)task->sp;  // NOLINT
  SPH = stack_pointer >> 8;
  SPL = stack_pointer & 0xFF;

  if (task->status == 0) {
    // Initial run ad the task
    task->status = TASK_ACTIVE;
    task->f();
    // current_task_idx should be used here. Because the task we are return
    // from may be not the same task we was delegating to
    tasks[current_task_idx].status = 0;
    tasks[current_task_idx].f = NULL;
    goto start;
  } else {
    // Continuation of the task
    RESTORE_CPU_STATE;
  }
}

void delay_ms(uint16_t ms) {
  delay_ticks(ms * MS_TO_TICKS_RATIO);
}

void delay_ticks(uint16_t ticks) {
  task_info *task = &tasks[current_task_idx];
  task->ticks_to_sleep = ticks;
  task->status = TASK_SLEEP;
  task_yield();
}

// Setting up system timer
void init_timers() {
  // set up timer with prescaler = 64
  // when chaning this logic MS_TO_TICKS_RATIO should be updated
  TCCR0B |= (1 << CS01) | (1 << CS00);
  // initialize counter
  TCNT0 = 0;
  // enable overflow interrupt
  TIMSK0 |= (1 << TOIE0);
  // enable global interrupts
  sei();
}

ISR(TIMER0_OVF_vect) {
  for (uint8_t i = 0; i < MAX_TASKS; i++) {
    task_info *task = &tasks[i];
    if (task->status == TASK_SLEEP && --task->ticks_to_sleep == 0) {
      task->status = TASK_ACTIVE;
    }
  }
}
