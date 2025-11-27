#ifndef IO_DEFS_H
#define IO_DEFS_H

#include <linux/ioctl.h>

// Data structure to pass port and value
struct io_req {
    unsigned int port;
    unsigned char value;
};

// Magic number 'k' (arbitrary, but standard practice)
#define IO_PROXY_MAGIC 'k'

// Defines the commands. 
// _IOW means we are Writing data to the kernel.
// _IOWR means we are Writing a request and Reading a response.
#define IO_WRITE _IOW(IO_PROXY_MAGIC, 1, struct io_req)
#define IO_READ  _IOWR(IO_PROXY_MAGIC, 2, struct io_req)

#endif