#include <system_error>

void verifyError(
    const char *name,
    std::error_code actualError,
    std::error_code expectedError = std::error_code()
);


void verifyError(
    const char *name,
    std::error_code actualError,
    std::errc expectedError
);