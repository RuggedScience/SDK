#ifndef PORTIO_H
#define PORTIO_H

#include <stdint.h>

// Initialize the driver/proxy (Optional: lazy initialization is used by default)
void portio_init();

// Close handles/file descriptors (Optional: OS cleans up on exit)
void portio_close();

// Hardware IO functions
uint8_t portio_inb(uint16_t port);
void portio_outb(uint8_t value, uint16_t port);

// Permission function
// - On Windows/Proxy: Returns 0 (Always Success/No-op)
// - On Legacy Linux: Calls the real ioperm() (Requires Root)
int portio_ioperm(unsigned long from, unsigned long num, int turn_on);

#endif // PORTIO_H