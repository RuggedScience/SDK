#include "../include/rserrors.h"

#include <iostream>

namespace detail {

const char *RsErrorCodeCategory::name() const noexcept { return "RsErrorCode"; }

std::string RsErrorCodeCategory::message(int c) const
{
    switch (static_cast<RsErrorCode>(c)) {
        case RsErrorCode::NotInitialized:
            return "Object not initialized";
        case RsErrorCode::XmlParseError:
            return "XML parsing error";
        default:
            return "Unknown Error";
    }
}

const char *RsErrorConditionCategory::name() const noexcept
{
    return "RsErrorCondition";
}

std::string RsErrorConditionCategory::message(int c) const
{
    switch (static_cast<RsErrorCondition>(c)) {
        case RsErrorCondition::HardwareError:
            return "Hardware error";
        case RsErrorCondition::UnsupportedFunction:
            return "Unsupported Function";
        case RsErrorCondition::PermissionError:
            return "Permission error";
        default:
            return "Unknown Error";
    }
}

bool RsErrorConditionCategory::equivalent(const std::error_code &ec, int cond)
    const noexcept
{
    switch (static_cast<RsErrorCondition>(cond)) {
        case RsErrorCondition::HardwareError:
            return (
                ec == std::errc::device_or_resource_busy ||
                ec == std::errc::no_such_device ||
                ec == std::errc::protocol_error
            );
        case RsErrorCondition::UnsupportedFunction:
            return (
                ec == std::errc::function_not_supported ||
                ec == std::errc::operation_not_supported
            );
        case RsErrorCondition::PermissionError:
            return (
                ec == std::errc::operation_not_permitted ||
                ec == std::errc::permission_denied
            );
    }

    return false;
}

}  // namespace detail

const detail::RsErrorCodeCategory &errorCodeCategory()
{
    static detail::RsErrorCodeCategory c;
    return c;
}

const detail::RsErrorConditionCategory &errorConditionCategory()
{
    static detail::RsErrorConditionCategory c;
    return c;
}