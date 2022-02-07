#include <simavr/avr_ioport.h>
#include <simavr/sim_avr.h>
#include <simavr/sim_elf.h>
#include <simavr/sim_gdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

void pin_changed_hook(struct avr_irq_t *irq, uint32_t value, void *param) {
  static bool header_printed = false;
  if (!header_printed) {
    printf("    T1 T2 T3 T4\n");
    header_printed = true;
  }
  switch (value) {
    case 1:
      printf("    --\n");
      break;
    case 2:
      printf("       --\n");
      break;
    case 3:
      printf("          --\n");
      break;
    case 4:
      printf("             --\n");
      break;

    default:
      //printf("PB%d [%d]\n", irq->irq, value);
      break;
  }
}

int main(int argc, char **argv) {
  elf_firmware_t firmware;

  argv++;
  char *file_name = *argv++;

  bool gdb_enabled = false;
  if (argc > 2 && strcmp(*argv, "-g") == 0) {
    gdb_enabled = true;
  }

  elf_read_firmware(file_name, &firmware);

  fprintf(stdout, "Firmware: %s, mcu=%s, freq=%d\n", file_name, firmware.mmcu,
          firmware.frequency);

  for (;;) {
    avr_t *avr = avr_make_mcu_by_name("atmega328p");
    if (avr == NULL) {
      fprintf(stderr, "[ERROR] Unable to create AVR core\n");
      exit(1);
    }
    avr_init(avr);
    avr_load_firmware(avr, &firmware);

    avr_irq_register_notify(
        avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), IOPORT_IRQ_PIN_ALL),
        pin_changed_hook, NULL);

    if (gdb_enabled) {
      avr->gdb_port = 1234;
      avr->state = cpu_Stopped;
      avr_gdb_init(avr);
    }

    printf("Running AVR core...\n");
    int status = cpu_Running;
    while ((status != cpu_Crashed) && (status != cpu_Done)) {
      status = avr_run(avr);
    }

    if (gdb_enabled) {
      avr_deinit_gdb(avr);
    }
  }
  return 0;
}
