#ifndef RSERRORS_H
#define RSERRORS_H

#include <system_error>

enum class RsErrorCode
{
    NotInitialized = 1,
    XmlParseError,
    UnknownError,
};

enum class RsErrorCondition
{
    HardwareError = 1,
    UserError
};

namespace std
{
    template <> struct is_error_code_enum<RsErrorCode> : true_type {};
    template <> struct is_error_condition_enum<RsErrorCondition> : true_type {};
}

std::error_code make_error_code(RsErrorCode);
std::error_condition make_error_condition(RsErrorCondition);

#endif //RSERRORS_H