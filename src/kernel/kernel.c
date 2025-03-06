#include "./terminal/terminal.c"

void kernel_main(void) {
  terminal_initialize();

  terminal_writestring("-=-=-=-=-= Hello from Kernel in C =-=-=-=-=-");
}