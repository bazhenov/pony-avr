#pragma once

#include <avr/io.h>

typedef struct {
  // stack pointer of a task
  void *sp;
  uint8_t flags;
  void (*f)(void);
} task_info;

// Task is allocated and can be run by a scheduler
#define TASK_ACTIVE 1

// Task has been already initialialized by a scheduler
#define TASK_INITIALIZED 2

#define MAX_TASKS 2
#define NO_TASK 0xFF

task_info tasks[MAX_TASKS];
void *stack_allocator = (void *)RAMEND - 100;
uint8_t current_task_idx = NO_TASK;

// Swaps the byte order in 2 byte integer
#define SWAP_ORDER(x) ((x >> 8) | (x << 8))