#ifndef PORTIO_HPP
#define PORTIO_HPP

#include <stdint.h>
#include <windows.h>
#include <stdexcept>

#define ioperm(x,y,z) 0

typedef void(*InitialDriver_t)();
static InitialDriver_t pfn_InitialDriver;

typedef uint8_t(*Inp_t)(uint16_t);
static Inp_t pfn_Inp;

typedef void(*Outp_t)(uint16_t, uint8_t);
static Outp_t pfn_Outp;

static void init()
{
    HINSTANCE hlib = LoadLibrary(L"drv.dll");
    if (hlib == NULL)
        throw std::runtime_error("Error loading drv.dll");

    pfn_InitialDriver = (InitialDriver_t)GetProcAddress(hlib, "InitialDriver");
    pfn_Inp = (Inp_t)GetProcAddress(hlib, "Inp");
    pfn_Outp = (Outp_t)GetProcAddress(hlib, "Outp");

    if (!pfn_InitialDriver || !pfn_Inp || pfn_Outp)
    {
        pfn_InitialDriver = NULL;
        pfn_Inp = NULL;
        pfn_Outp = NULL;
        throw std::runtime_error("Failed to load functions from drv.dll");
    }

    pfn_InitialDriver();
}

uint8_t inb(uint16_t port)
{
    if (!pfn_Inp)
        init();

    return pfn_Inp(port);
}

void outb(uint8_t value, uint16_t port)
{
    if (pfn_Outp == NULL)
        init();

    return pfn_Outp(port, value);
}

#endif //PORTIO_HPP