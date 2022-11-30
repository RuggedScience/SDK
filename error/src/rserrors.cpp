#include "../include/rserrors.h"

#include <iostream>

namespace {

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
        case RsErrorCondition::UserError:
            return "User error";
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
            case RsErrorCondition::UserError:
                return (
                    ec == RsErrorCode::NotInitialized ||
                    ec == RsErrorCode::XmlParseError ||
                    ec == std::errc::invalid_argument ||
                    ec == std::errc::function_not_supported ||
                    ec == std::errc::operation_not_permitted ||
                    ec == std::errc::no_such_file_or_directory
                );
        }

        return false;
    }
};

const RsErrorCodeCategory rsErrorCodeCategory{};
const RsErrorConditionCategory rsErrorConditionCategory{};

} // Anonymous namespace

std::error_code make_error_code(RsErrorCode e)
{
    return {static_cast<int>(e), rsErrorCodeCategory};
}

std::error_condition make_error_condition(RsErrorCondition e)
{
    return {static_cast<int>(e), rsErrorConditionCategory};
}