#include <simavr/sim_avr.h>
#include <simavr/sim_elf.h>
#include <simavr/sim_gdb.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  elf_firmware_t firmware;

  char *file_name = argv[1];

  elf_read_firmware(file_name, &firmware);

  fprintf(stdout, "Firmware: %s, mcu=%s, freq=%d\n", file_name, firmware.mmcu,
          firmware.frequency);

  avr_t *avr = avr_make_mcu_by_name("atmega328p");
  if (avr == NULL) {
    fprintf(stderr, "[ERROR] Unable to create AVR core\n");
    exit(1);
  }
  avr_init(avr);
  avr_load_firmware(avr, &firmware);

  avr->gdb_port = 1234;
  avr->state = cpu_Stopped;
  avr_gdb_init(avr);

  int status = cpu_Running;
  while ((status != cpu_Crashed) && (status != cpu_Done)) {
    status = avr_run(avr);
  }
  return 0;
}
