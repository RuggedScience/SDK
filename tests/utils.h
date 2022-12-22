#include <iostream>
#include <system_error>

void verifyError(
    const char *name,
    std::error_code actualError,
    std::error_code expectedError = std::error_code()
)
{
    if (actualError != expectedError) {
        std::cerr << name << ": Expected " << expectedError.message()
                  << " but got " << actualError.message() << std::endl;
        exit(1);
    }
}

void verifyError(
    const char *name,
    std::error_code actualError,
    std::errc expectedError
)
{
    std::error_code code = std::make_error_code(expectedError);
    verifyError(name, actualError, code);
}