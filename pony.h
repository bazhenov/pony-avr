#pragma once

#include <avr/io.h>

typedef struct {
  // stack pointer of a task
  void *sp;
  uint8_t flags;
  uint16_t ticks_to_sleep;
  void (*f)(void);
} task_info;

// Task has been already initialialized by a scheduler
#define TASK_ACTIVE 1

#define TASK_SLEEP 2

#define MAX_TASKS 5
#define NO_TASK 0xFF

void *stack_allocator = (void *)RAMEND;
volatile task_info tasks[MAX_TASKS];
volatile uint8_t current_task_idx = NO_TASK;

// Swaps the byte order in 2 byte integer
#define SWAP_ORDER(x) ((x >> 8) | (x << 8))
