#ifndef IO_SHIM_HPP
#define IO_SHIM_HPP

#include <linux/ioctl.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <stdlib.h>

#define outb(val, port) proxy_outb(val, port)
#define inb(port)       proxy_inb(port)

// ioperm is no longer needed, so we macro it to always succeed (return 0)
#define ioperm(from, num, turn_on) (0)


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

static int io_fd = -1;

inline int io_init(void) {
    if (io_fd >= 0) return 0; // Already open

    io_fd = open("/dev/io_proxy", O_RDWR);
    if (io_fd < 0) {
        perror("FATAL: Could not open /dev/io_proxy. Is the kernel module loaded?");
        return -1;
    }
    return 0;
}

inline void io_close(void) {
    if (io_fd >= 0) {
        close(io_fd);
        io_fd = -1;
    }
}

inline void proxy_outb(unsigned char value, unsigned int port) {
    struct io_req req;
    req.port = port;
    req.value = value;

    io_init();

    if (io_fd < 0) {
        fprintf(stderr, "Error: IO Proxy not initialized. Call io_init() first.\n");
        return;
    }

    if (ioctl(io_fd, IO_WRITE, &req) < 0) {
        perror("ioctl IO_WRITE failed");
    }
}

inline unsigned char proxy_inb(unsigned int port) {
    struct io_req req;
    req.port = port;
    req.value = 0;

    io_init();

    if (io_fd < 0) {
        fprintf(stderr, "Error: IO Proxy not initialized. Call io_init() first.\n");
        return 0xFF;
    }

    if (ioctl(io_fd, IO_READ, &req) < 0) {
        perror("ioctl IO_READ failed");
        return 0xFF;
    }
    return req.value;
}

#endif