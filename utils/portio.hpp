#ifndef PORTIO_HPP
#define PORTIO_HPP

#include <stdint.h>
#include <windows.h>
#include <stdexcept>

#define ioperm(x,y,z) 0

static void init(HINSTANCE hlib)
{
    typedef void(*pfn_t)();
    static pfn_t pfn = NULL;

	if (hlib == NULL)
        throw std::runtime_error("Error loading drv.dll");

    if (pfn == NULL)
    {
        pfn = (pfn_t)GetProcAddress(hlib, "InitialDriver");
        pfn();
    }
}

static HINSTANCE GetIOMemLibrary()
{
	static HINSTANCE hlib = NULL;
	if (hlib == NULL)
    {
		hlib = LoadLibrary("drv.dll");
        init(hlib);
    }

	return hlib;
}

inline uint8_t inb(uint16_t port)
{
	typedef uint8_t(*pfn_t)(uint64_t);
	static pfn_t pfn = NULL;

	if (pfn == NULL)
	{
		HINSTANCE hlib = GetIOMemLibrary();
		if (hlib != NULL)
			pfn = (pfn_t)GetProcAddress(hlib, "Inp");
	}

	uint8_t value = 0;
	if (pfn) value = (pfn)(port);
	return value;
}

inline void outb(uint8_t value, uint16_t port)
{
    typedef void(*pfn_t)(uint16_t, uint8_t);
    static pfn_t pfn = NULL;

    if (pfn == NULL)
    {
        HINSTANCE hlib = GetIOMemLibrary();
        if (hlib != NULL)
            pfn = (pfn_t)GetProcAddress(hlib, "Outp");
    }

    if (pfn) (pfn)(port, value);
}

#endif //PORTIO_HPP