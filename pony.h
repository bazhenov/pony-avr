#pragma once

#include <avr/io.h>
#include <stdbool.h>

typedef volatile struct {
  // stack pointer of a task
  void *sp;
  uint8_t status;
  uint16_t ticks_to_sleep;
  void (*f)(void);
} task_info;

// Task has been already initialialized by a scheduler
#define TASK_ACTIVE 1

#define TASK_SLEEP 2

#define MAX_TASKS 5
#define NO_TASK 0xFF

void *stack_allocator = (void *)RAMEND;
task_info tasks[MAX_TASKS];
volatile uint8_t current_task_idx = NO_TASK;

// Swaps the byte order in 2 byte integer
#define SWAP_ORDER(x) ((x >> 8) | (x << 8))

void __attribute__((noinline)) task_yield(void);
bool task_create(void (*callable)(void), uint8_t stack_size);
void delay_ticks(uint16_t ticks);
