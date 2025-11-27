#include "../include/portio.h"
#include "i801_smbus.h"
#include <stdexcept>
#include <cstdio>

// =============================================================
// WINDOWS IMPLEMENTATION
// =============================================================
#if defined(_WIN32) || defined(_WIN64)

#include <windows.h>

static HINSTANCE g_hlib = NULL;

typedef void(*InitFunc)();
typedef uint8_t(*InpFunc)(uint64_t);
typedef void(*OutpFunc)(uint16_t, uint8_t);

static InpFunc g_inp = NULL;
static OutpFunc g_outp = NULL;

static void ensureLoaded() {
    if (g_hlib != NULL) return;

    g_hlib = LoadLibrary("drv.dll");
    if (g_hlib == NULL) {
        throw std::runtime_error("Error loading drv.dll");
    }

    InitFunc init_drv = (InitFunc)GetProcAddress(g_hlib, "InitialDriver");
    if (init_drv) init_drv();

    g_inp = (InpFunc)GetProcAddress(g_hlib, "Inp");
    g_outp = (OutpFunc)GetProcAddress(g_hlib, "Outp");
}

void portio_init() { ensureLoaded(); }
void portio_close() { if (g_hlib) FreeLibrary(g_hlib); g_hlib = NULL; }

uint8_t portio_inb(uint16_t port) {
    ensureLoaded();
    if (g_inp) return g_inp(port);
    return 0;
}

void portio_outb(uint8_t value, uint16_t port) {
    ensureLoaded();
    if (g_outp) g_outp(port, value);
}

int portio_ioperm(unsigned long from, unsigned long num, int turn_on) {
    // Windows driver handles permissions
    return 0; 
}

// =============================================================
// LINUX IMPLEMENTATION (Hybrid: Proxy + Direct Fallback)
// =============================================================
#elif defined(__linux__)

#include <fcntl.h>
#include <unistd.h>
#include <sys/io.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <system_error>  
#include <fstream>
#include <string>
#include "io_defs.h"


static int g_io_fd = -1;
// Fallback flag to determin if io_proxy or standard IO calls should be used
static bool g_direct_io = false;
static bool g_initialized = false;

// Function to check if kernel lockdown is active
static bool isLockdownActive() {
    std::ifstream file("/sys/kernel/security/lockdown");
    if (!file.is_open()) return false; // Kernel too old or securityfs not mounted

    std::string content;
    std::getline(file, content);
    
    // The file format uses brackets to show active mode: "none [integrity] confidentiality"
    // If "none" is NOT in brackets, then some form of lockdown is active.
    if (content.find("[none]") != std::string::npos) {
        return false; 
    }
    return true; // "[integrity]" or "[confidentiality]" was found
}

static void ensureLoaded() {
    if (g_initialized) return;

    // Try to open the Proxy Device
    g_io_fd = open("/dev/io_proxy", O_RDWR);
    
    if (g_io_fd >= 0) {
        // Success: Use Proxy device
        g_direct_io = false;
    } else {
        // If permission was denied, throw an error and don't fall back to direct IO.
        if (errno == EACCES) {
            throw std::system_error(
                std::make_error_code(std::errc::permission_denied)
            );
        } else if (isLockdownActive()) {
            fprintf(stderr, 
                "\nCRITICAL WARNING: \n"
                "  Kernel Lockdown is ACTIVE (Secure Boot enabled).\n"
                "  The 'io_proxy' kernel module is NOT loaded.\n"
                "  Direct hardware access (ioperm) WILL FAIL.\n"
                "  Please install the driver or sign the kernel module.\n"
                "  See https://github.com/RuggedScience/io_proxy\n\n"
            );
        }
        // Fail: Use standard IO calls
        g_direct_io = true;
    }

    g_initialized = true;
}

void portio_init() { ensureLoaded(); }

void portio_close() {
    if (g_io_fd >= 0) {
        close(g_io_fd);
        g_io_fd = -1;
    }
    g_initialized = false;
}

int portio_ioperm(unsigned long from, unsigned long num, int turn_on) {
    ensureLoaded();
    
    if (g_direct_io) {
        // In Direct mode, we MUST call the real kernel syscall
        return ioperm(from, num, turn_on);
    } else {
        // In Proxy mode, the kernel module handles access
        return 0;
    }
}

uint8_t portio_inb(uint16_t port) {
    ensureLoaded();

    if (g_direct_io) {
        // Legacy: Use inline assembly for raw IO
        uint8_t value;
        __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
        return value;
    } else {
        // Proxy: Use ioctl
        struct io_req req = { port, 0 };
        if (ioctl(g_io_fd, IO_READ, &req) < 0) {
            throw std::system_error(
                std::make_error_code(std::errc(errno))
            );
        }
        return req.value;
    }
}

void portio_outb(uint8_t value, uint16_t port) {
    ensureLoaded();

    if (g_direct_io) {
        // Legacy: Use inline assembly for raw IO
        __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
    } else {
        // Proxy: Use ioctl
        struct io_req req = { port, value };
        if (ioctl(g_io_fd, IO_WRITE, &req) < 0) {
            throw std::system_error(
                std::make_error_code(std::errc(errno))
            );
        }
    }
}

#else
#error "Unsupported Platform"
#endif

