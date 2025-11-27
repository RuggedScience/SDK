#ifndef RSDIO_TYPES_H
#define RSDIO_TYPES_H

namespace rs {
    enum class OutputMode
    { 
        Source = -1, 
        Sink = -2 
    };

    enum class PinDirection
    { 
        Input, 
        Output 
    };
}

#endif // RSDIO_TYPES_H