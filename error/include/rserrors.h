#ifndef RSERRORS_H
#define RSERRORS_H

#include <system_error>
#include <rserrors_export.h>

#include <iostream>

enum class RsErrorCode
{
    NotInitialized = 1,
    XmlParseError,
    UnknownError,
};

enum class RsErrorCondition
{
    HardwareError = 1,
    UnsupportedFunction,
    PermissionErrror,
    InitializationError
};

namespace std
{
    template <> struct is_error_code_enum<RsErrorCode> : true_type {};
    template <> struct is_error_condition_enum<RsErrorCondition> : true_type {};
}

namespace detail
{

class RsErrorCodeCategory : public std::error_category
{
public:
    const char *name() const noexcept override final
    {
        return "RsErrorCode";
    }

    std::string message(int c) const override final
    {
        switch (static_cast<RsErrorCode>(c))
        {
        case RsErrorCode::NotInitialized:
            return "Object not initialized";
        case RsErrorCode::XmlParseError:
            return "XML parsing error";
        default:
            return "Unknown Error";
        }
    }
};

class RsErrorConditionCategory : public std::error_category
{
public:
    const char *name() const noexcept override final
    {
        return "RsErrorCondition";
    }

    std::string message(int c) const override final
    {
        switch (static_cast<RsErrorCondition>(c))
        {
        case RsErrorCondition::HardwareError:
            return "Hardware error";
        case RsErrorCondition::UnsupportedFunction:
            return "Unsupported Function";
        case RsErrorCondition::PermissionErrror:
            return "Permission error";
        case RsErrorCondition::InitializationError:
            return "Initialization error";
        default:
            return "Unknown Error";
        }
    }

    bool equivalent(const std::error_code& ec, int cond) const noexcept override final
    {
        switch (static_cast<RsErrorCondition>(cond))
        {
            case RsErrorCondition::HardwareError:
                return (
                    ec == std::errc::device_or_resource_busy ||
                    ec == std::errc::no_such_device || 
                    ec == std::errc::protocol_error
                );
            case RsErrorCondition::UnsupportedFunction:
                return (
                    ec == std::errc::no_such_device ||
                    ec == std::errc::function_not_supported ||
                    ec == std::errc::invalid_argument
                );
            case RsErrorCondition::PermissionErrror:
                return (
                    ec == std::errc::operation_not_permitted ||
                    ec == std::errc::permission_denied
                );
            case RsErrorCondition::InitializationError:
                return (
                    ec == RsErrorCode::NotInitialized ||
                    ec == RsErrorCode::XmlParseError
                );
        }

        return false;
    }
};

}

extern "C" RSERRORS_EXPORT const detail::RsErrorCodeCategory &errorCodeCategory();
extern "C" RSERRORS_EXPORT const detail::RsErrorConditionCategory &errorConditionCategory();

inline std::error_code make_error_code(RsErrorCode e)
{
    return {static_cast<int>(e), errorCodeCategory()};
}

inline std::error_condition make_error_condition(RsErrorCondition e)
{
    return {static_cast<int>(e), errorConditionCategory()};
}

#endif //RSERRORS_H