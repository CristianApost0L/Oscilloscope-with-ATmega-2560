#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif
  
  //! returns the descriptor of a serial port
  int serial_open(const char* name);

  //! sets the attributes
  int serial_set_interface_attribs(int fd, int speed, int parity);
  
  //! puts the port in blocking/nonblocking mode
  void serial_set_blocking(int fd, int should_block);

#ifdef __cplusplus
}
#endif

void printProgressBar(int currentIteration, int totalIterations);
void handle_sigint(int sig);

// Struct per la ricezione dei dati
typedef struct {
  uint16_t sample_frequency;
  uint8_t channels;
  uint8_t mode;
} __attribute__((packed)) __serial_data__;