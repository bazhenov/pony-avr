#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <util/delay.h>

#include "pony.h"

void __attribute__((noinline)) toggle(uint8_t value) { PORTB = value; }

void task_yield(void);

void task_create(void (*callable)(void), uint8_t stack_size) {
  volatile task_info *task = NULL;
  for (uint8_t i = 0; i < MAX_TASKS; i++) {
    if (tasks[i].f == NULL) {
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
  task->f = callable;
}

uint8_t find_next_task() {
  for (;;) {
    for (uint8_t i = 0; i < MAX_TASKS; i++) {
      uint8_t task_id = (uint8_t)(current_task_idx + 1 + i) % MAX_TASKS;
      volatile task_info *task = &tasks[task_id];
      if (task->f != NULL && task->flags != TASK_SLEEP) {
        return task_id;
      }
    }
  }
}

void scheduller_halt(void) {
  for (;;)
    ;
}

void __attribute__((always_inline)) save_cpu_state(void) {
  asm("push r2" ::);
  asm("push r3" ::);
  asm("push r4" ::);
  asm("push r5" ::);
  asm("push r6" ::);
  asm("push r7" ::);
  asm("push r8" ::);
  asm("push r9" ::);
  asm("push r10" ::);
  asm("push r11" ::);
  asm("push r12" ::);
  asm("push r13" ::);
  asm("push r14" ::);
  asm("push r15" ::);
  asm("push r16" ::);
  asm("push r17" ::);
  asm("push r28" ::);
  asm("push r29" ::);
}

void __attribute__((always_inline)) restore_cpu_state(void) {
  asm("pop r29" ::);
  asm("pop r28" ::);
  asm("pop r17" ::);
  asm("pop r16" ::);
  asm("pop r15" ::);
  asm("pop r14" ::);
  asm("pop r13" ::);
  asm("pop r12" ::);
  asm("pop r11" ::);
  asm("pop r10" ::);
  asm("pop r9" ::);
  asm("pop r8" ::);
  asm("pop r7" ::);
  asm("pop r6" ::);
  asm("pop r5" ::);
  asm("pop r4" ::);
  asm("pop r3" ::);
  asm("pop r2" ::);
}

void __attribute__((noinline)) task_yield(void) {
  // context switch to next task
  uint8_t next_task_id;
start:
  next_task_id = find_next_task();

  if (next_task_id != current_task_idx) {
    if (current_task_idx != NO_TASK) {
      // saving current task state
      save_cpu_state();
      tasks[current_task_idx].sp = (void *)((SPH << 8) | SPL); // NOLINT
    }

    // restoring next task state
    volatile task_info *task = &tasks[next_task_id];
    current_task_idx = next_task_id;

    uintptr_t stack_pointer = (uintptr_t)task->sp; // NOLINT
    SPH = stack_pointer >> 8;
    SPL = stack_pointer & 0xFF;

    if (task->flags == 0) {
      // Initial run ad the task

      task->flags = TASK_ACTIVE;
      task->f();
      // current_task_idx should be used here. Because the task we are return
      // from may be not the same task we was delegating to
      tasks[current_task_idx].flags = 0;
      tasks[current_task_idx].f = NULL;
      goto start;
    } else {
      // Continuation of the task
      restore_cpu_state();
    }
  }
}

void delay_ticks(uint16_t ticks) {
  volatile task_info *task = &tasks[current_task_idx];
  task->ticks_to_sleep = ticks;
  task->flags = TASK_SLEEP;
  task_yield();
}

void task1(void) {
  for (;;) {
    PORTB ^= 1 << PIN5;
    delay_ticks(20);
  }
}

void task2(void) {
  for (;;) {
    PORTB ^= 1 << PIN4;
    delay_ticks(50);
  }
}

void setup_timer() {
  // set up timer with prescaler = 1024
  TCCR0B |= (1 << CS02) | (1 << CS00);
  // initialize counter
  TCNT0 = 0;
  // enable overflow interrupt
  TIMSK0 |= (1 << TOIE0);
  // enable global interrupts
  sei();
}

uint32_t ticks = 0;
ISR(TIMER0_OVF_vect) {
  ticks++;
  for (uint8_t i = 0; i < MAX_TASKS; i++) {
    volatile task_info *task = &tasks[i];
    if (task->flags == TASK_SLEEP && --task->ticks_to_sleep == 0) {
      task->flags = TASK_ACTIVE;
    }
  }
}

int main(void) {
  DDRB = 0xFF;

  setup_timer();
  task_create(&task1, 100);
  task_create(&task2, 100);

  task_yield();
}
