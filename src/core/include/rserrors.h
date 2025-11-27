#ifndef RSERRORS_H
#define RSERRORS_H

#include <stdexcept>
#include <string>
#include <system_error>

namespace rs {

// Simple Enum for programmatic checking
enum class RsErrorCode {
    None = 0,
    NotInitialized,
    FileNotFound,
    ConfigParseError,
    HardwareError,
    PermissionError,
    UnsupportedFunction,
    InvalidParameter,
    UnknownError
};

// Custom Exception Class
// No 'RSERRORS_EXPORT' needed because it's all inline!
class RsException : public std::runtime_error {
   public:
    // Constructor taking a code and a message
    RsException(RsErrorCode code, const std::string& message)
        : std::runtime_error(message), m_code(code)
    {
    }

    // Constructor for C-style strings
    RsException(RsErrorCode code, const char* message)
        : std::runtime_error(message), m_code(code)
    {
    }

    // Accessor for the machine-readable code
    RsErrorCode code() const noexcept { return m_code; }

   private:
    RsErrorCode m_code;
};

[[noreturn]] inline void rethrowAsRsException()
{
    try {
        // Re-throw the current exception to inspect it
        throw;
    }
    catch (const RsException&) {
        throw;
    }
    catch (const std::system_error& e) {
        if (e.code() == std::errc::function_not_supported ||
            e.code() == std::errc::operation_not_supported) {
            throw RsException(RsErrorCode::UnsupportedFunction, e.what());
        }

        if (e.code() == std::errc::permission_denied ||
            e.code() == std::errc::operation_not_permitted) {
            throw RsException(
                RsErrorCode::PermissionError,
                "Access denied: Ensure you have root privileges or the IO "
                "proxy is loaded."
            );
        }

        if (e.code() == std::errc::invalid_argument) {
            throw RsException(RsErrorCode::InvalidParameter, e.what());
        }

        throw RsException(RsErrorCode::HardwareError, e.what());
    }
    catch (const std::exception& e) {
        // Catch standard C++ errors (logic_error, bad_alloc, etc.)
        throw RsException(RsErrorCode::UnknownError, e.what());
    }
    catch (...) {
        // Catch everything else (ints, string literals, foreign exceptions)
        throw RsException(
            RsErrorCode::UnknownError, "Unknown non-standard exception occurred"
        );
    }
}

}  // namespace rs

#endif  // RSERRORS_H