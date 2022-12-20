#ifndef RSERRORS_H
#define RSERRORS_H

#include <system_error>

#ifdef NO_EXPORT
#define RSERRORS_EXPORT
#else
#include <rserrors_export.h>
#endif

#include <iostream>

enum class RsErrorCode {
    NotInitialized = 1,
    XmlParseError,
    UnknownError,
};

enum class RsErrorCondition {
    HardwareError = 1,
    UnsupportedFunction,
    PermissionError
};

namespace std {
template <>
struct is_error_code_enum<RsErrorCode> : true_type {};
template <>
struct is_error_condition_enum<RsErrorCondition> : true_type {};
}  // namespace std

namespace detail {

class RsErrorCodeCategory : public std::error_category {
   public:
    const char *name() const noexcept override final;
    std::string message(int c) const override final;
};

class RsErrorConditionCategory : public std::error_category {
   public:
    const char *name() const noexcept override final;
    std::string message(int c) const override final;
    bool equivalent(const std::error_code &ec, int cond)
        const noexcept override final;
};

}  // namespace detail

extern "C" RSERRORS_EXPORT const detail::RsErrorCodeCategory &errorCodeCategory(
);
extern "C" RSERRORS_EXPORT const detail::RsErrorConditionCategory &
errorConditionCategory();

inline std::error_code make_error_code(RsErrorCode e)
{
    return {static_cast<int>(e), errorCodeCategory()};
}

inline std::error_condition make_error_condition(RsErrorCondition e)
{
    return {static_cast<int>(e), errorConditionCategory()};
}

#endif  // RSERRORS_H